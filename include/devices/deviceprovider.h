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

typedef uint64_t archetype_id;
typedef uint64_t device_id;
typedef uint64_t light_id;
typedef uint64_t deviceprovider_id;

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

enum class EDeviceState : uint8_t
{
	Disconnected = 0,
	Connecting,
	PendingLink,
	Connected
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
class DeviceProvider: public QObject
{
	Q_OBJECT

public:
	DeviceProvider(class DeviceProviderDiscovery* parent = nullptr);
	virtual ~DeviceProvider();
	DeviceProviderDiscovery* deviceDiscoveryParent;

	virtual QString getName() = 0;
	virtual archetype_id getArchetype() = 0;
	virtual int getMaxLowLatencyDevices() = 0;
	virtual void setUsedDevices(std::vector<device_id> devices) = 0;
	virtual void setLowLatencyDevices(std::vector<device_id> devices) = 0;

	QAtomicInteger<uint8_t> state;

public slots:
	void link();
	void start() { emit startRequested(); };
	void stop() { emit stopRequested(); };

signals:
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
	std::shared_ptr<Device> createAndAddDeviceFromArchetype(archetype_id archetype, QString name = QString(""));
};

struct LightArchetype
{
	//0.0 to 1.0, in device space
	double minX;
	double minY;
	double minZ;

	double maxX;
	double maxY;
	double maxZ;

	LightArchetype(double inMinX, double inMinY, double inMinZ, double inMaxX, double inMaxY, double inMaxZ) :
		minX(inMinX), minY(inMinY), minZ(inMinZ), maxX(inMaxX), maxY(inMaxY), maxZ(inMaxZ)
	{

	}
};

struct DeviceArchetype
{
	QString name;
	std::vector<LightArchetype> Lights;
	DeviceArchetype(QString inName, std::vector<LightArchetype>& inLights) : name(inName), Lights(inLights) {}
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