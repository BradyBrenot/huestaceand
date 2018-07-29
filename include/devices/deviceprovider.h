#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QHash>
#include <QHostAddress>
#include <QReadWriteLock>
#include <QAtomicInteger>
#include <QString>

#include "types.h"
#include "archetypes.h"

struct Light
{
	double h;
	double s;
	double l;

	double desired_h;
	double desired_s;
	double desired_l;
};

struct Device
{
	archetype_id archetype;
	std::vector<Light> lights;
	device_id id;
	QString name;

	Device(archetype_id inArchetype, std::vector<Light>& inLights, device_id inId, QString inName)
		: archetype(inArchetype),
		lights(inLights),
		id(inId),
		name(inName)
	{
	}

	Device() : archetype(), lights(), id(), name() {}
};

//////////////////////////////////////////////////////////////////////////

struct BaseDeviceProviderLock
{
	bool contains(device_id id);
	std::vector<device_id> keys();
	const Device& operator[](device_id id);

	BaseDeviceProviderLock(std::shared_ptr<class DeviceProvider> parent);
	BaseDeviceProviderLock(BaseDeviceProviderLock&& from);

	virtual ~BaseDeviceProviderLock();

protected:
	BaseDeviceProviderLock() : m_parent() {};
	std::shared_ptr<class DeviceProvider> m_parent;
};

struct DeviceProviderReadLock : public BaseDeviceProviderLock
{
	DeviceProviderReadLock(std::shared_ptr<class DeviceProvider> parent);
	DeviceProviderReadLock(DeviceProviderReadLock&& from);
};

struct DeviceProviderWriteLock : public BaseDeviceProviderLock
{
	DeviceProviderWriteLock(std::shared_ptr<class DeviceProvider> parent);
	DeviceProviderWriteLock(DeviceProviderWriteLock&& from);

	virtual ~DeviceProviderWriteLock();

	//#todo: is this dumb to do in bulk?
	void SetDesiredColor(double h, double s, double l, device_id device, light_id light);

private:
	bool didDesiredLightColorChange;
};

/*
 * DeviceProvider contains, manages a set of devices. 
 * Devices can be read and written in a thread-safe manner.
 *
 * All public functions are safe to call from any thread. Some
 * functionality is hidden behind the two lock functions for safety;
 * any other public functions can be assumed to be thread-safe.
 *
 * Subclasses are expected to implement their own update loop using
 * whatever timing or other events are appropriate. A subclass must
 * call updateLightColors(), then lock the device for reads and
 * perform the upload.
 *
 * The update loop should start as a response to the
 * startRequested() and stopRequested() signals.
 *
 * Aside from simply being timing-based, you might listen to
 * the desiredLightColorChanged() signal.
 */
class DeviceProvider: public QObject, public std::enable_shared_from_this<DeviceProvider>
{
	Q_OBJECT

public:
	enum class EDeviceState
	{
		Disconnected = 0,
		Connecting,
		PendingLink,
		Connected
	};
	Q_ENUM(EDeviceState)

	DeviceProvider(class DeviceProviderDiscovery* parent = nullptr);
	virtual ~DeviceProvider();
	DeviceProviderDiscovery* deviceDiscoveryParent;

	virtual QString getName() = 0;
	virtual archetype_id getArchetype() = 0;
	virtual int getMaxLowLatencyDevices() = 0;
	virtual void setUsedDevices(std::vector<device_id> devices) = 0;
	virtual void setLowLatencyDevices(std::vector<device_id> devices) = 0;

	QAtomicInteger<uint8_t> state;

	deviceprovider_id id;

	//To access or write to the array of devices or the current state, you must use the appropriate lock
	DeviceProviderReadLock lockDeviceRead();
	DeviceProviderWriteLock lockDeviceWrite();

public slots:
	void link();
	void start() { emit startRequested(); };
	void stop() { emit stopRequested(); };

signals:
	void desiredLightColorChanged();
	void devicesChanged();
	void stateChanged(EDeviceState newState);
	void startRequested();
	void stopRequested();

protected:
	//Call this to update the actual colors of lights, prior to uploading color changes to hardware
	//(handles over-time color interpolation, setting actual light color part-way to desired color)
	void updateLightColors();

	void setState(EDeviceState state);

	// Optionally implement if this device requires prompting the user to 'link' in order to use
	virtual void doLink() {}

	std::unordered_map<device_id, std::shared_ptr<Device> > devices;

	//Create a device, add it to the devices array, and return a reference to it.
	std::shared_ptr<Device> createAndAddDeviceFromArchetype(archetype_id archetype, device_id id, QString name = QString(""));

	friend BaseDeviceProviderLock;
	friend DeviceProviderReadLock;
	friend DeviceProviderWriteLock;

	QReadWriteLock rwlock;
};

class DeviceProviderDiscovery : public QObject
{
	Q_OBJECT

public:
	DeviceProviderDiscovery(class Huestaceand* parent = nullptr);
	class Huestaceand* daemonParent;

	virtual QString getName() = 0;
	virtual bool canSearchForDevicesAtIPAddress() = 0;

public slots:
	virtual void searchForDeviceProviders() = 0;
	virtual void searchForDeviceProviderByIp(QHostAddress address) = 0;

signals:
	void foundDeviceProvider(std::shared_ptr<DeviceProvider> Provider);

protected:
	friend class DeviceProvider;
};