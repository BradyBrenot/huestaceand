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

typedef uint64_t archetype_id;
typedef uint64_t device_id;
typedef uint64_t light_id;
typedef QString deviceprovider_id;

namespace std
{
#if 0
	template<> struct hash<device_id>
	{
		std::size_t operator()(device_id const& id) const noexcept
		{
			return std::hash<uint64_t>()(static_cast<uint64_t>(id));
		}
	};
#endif

	template<> struct hash<deviceprovider_id>
	{
		std::size_t operator()(deviceprovider_id const& id) const noexcept
		{
			return qHash(static_cast<QString>(id));
		}
	};
}

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

struct BaseDeviceProviderLock
{
	bool contains(device_id id);
	std::vector<device_id> keys();
	EDeviceState state();
	const Device& operator[](device_id id);

	BaseDeviceProviderLock(std::shared_ptr<class DeviceProvider> provider);
	BaseDeviceProviderLock(BaseDeviceProviderLock&& from);

	virtual ~BaseDeviceProviderLock();

protected:
	BaseDeviceProviderLock() : m_provider() {};
	std::shared_ptr<class DeviceProvider> m_provider;
};

struct DeviceProviderReadLock : public BaseDeviceProviderLock
{
	DeviceProviderReadLock(std::shared_ptr<class DeviceProvider> provider);
	DeviceProviderReadLock(DeviceProviderReadLock&& from);
};

struct DeviceProviderWriteLock : public BaseDeviceProviderLock
{
	DeviceProviderWriteLock(std::shared_ptr<class DeviceProvider> provider);
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
class DeviceProvider: public QObject, std::enable_shared_from_this<DeviceProvider>
{
	Q_OBJECT

public:
	DeviceProvider(class DeviceProviderDiscovery* parent = nullptr);
	virtual ~DeviceProvider();

	//To access or write to the array of devices or the current state, you must use the appropriate lock
	DeviceProviderReadLock lockDeviceRead();
	DeviceProviderWriteLock lockDeviceWrite();

	virtual QString getName() = 0;
	virtual int getMaxLowLatencyDevices() = 0;
	virtual void setUsedDevices(std::vector<device_id> devices) = 0;
	virtual void setLowLatencyDevices(std::vector<device_id> devices) = 0;

public slots:
	void link();
	void start() { emit startRequested(); };
	void stop() { emit stopRequested(); };

signals:
	void devicesChanged();
	void stateChanged(EDeviceState newState);
	void desiredLightColorChanged();
	void startRequested();
	void stopRequested();

protected:
	//Call this to update the actual colors of lights, prior to uploading color changes to hardware
	void updateLightColors();

	EDeviceState getState() const;
	void setState(EDeviceState state);

	// Optionally implement if this device requires prompting the user to 'link' in order to use
	virtual void doLink() {}

	//Create a device, add it to the devices array, and return a reference to it.
	std::unique_ptr<Device>& createAndAddDeviceFromArchetype(device_id id, archetype_id archetype, QString name = QString(""));

	std::unordered_map<device_id, std::unique_ptr<Device> > devices;

private:
	QReadWriteLock rwlock;
	EDeviceState m_state;

	friend BaseDeviceProviderLock;
	friend DeviceProviderReadLock;
	friend DeviceProviderWriteLock;
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
};

struct DeviceArchetype
{
	std::vector<LightArchetype> Lights;
};

typedef std::unordered_map<archetype_id, std::shared_ptr<DeviceArchetype>> archetypeMap;
typedef std::unordered_map<deviceprovider_id, std::shared_ptr<DeviceProvider>> deviceProviderMap;

class DeviceProviderDiscovery : public QObject
{
	Q_OBJECT

public:
	DeviceProviderDiscovery(QObject* parent = nullptr);

	virtual QString getName() = 0;
	virtual bool canSearchForDevicesAtIPAddress() = 0;

	const std::shared_ptr<DeviceArchetype> GetDeviceArchetype(archetype_id id) {
		QMutexLocker lock(&archetypesLock);
		auto archetype = m_deviceArchetypes.find(id);
		if (archetype == m_deviceArchetypes.end()) {
			return std::shared_ptr<DeviceArchetype>(nullptr);
		}
		return archetype->second;
	}
	const std::shared_ptr<DeviceProvider> GetDeviceProvider(deviceprovider_id id) {
		QMutexLocker lock(&deviceProvidersLock);
		auto provider = m_deviceProviders.find(id);
		if (provider == m_deviceProviders.end()) {
			return std::shared_ptr<DeviceProvider>(nullptr);
		}
		return provider->second;
	}

	const archetypeMap GetDeviceArchetypes() {
		QMutexLocker lock(&archetypesLock);
		return m_deviceArchetypes;
	}
	const deviceProviderMap GetDeviceProviders() {
		QMutexLocker lock(&deviceProvidersLock);
		return m_deviceProviders;
	}

public slots:
	virtual void searchForDeviceProviders() = 0;
	virtual void searchForDeviceProviderByIp(QHostAddress address) = 0;

signals:
	void foundDeviceProvider(std::shared_ptr<DeviceProvider> Provider);

protected:
	archetypeMap m_deviceArchetypes;
	deviceProviderMap m_deviceProviders;

	QMutex archetypesLock;
	QMutex deviceProvidersLock;

	friend class DeviceProvider;
};