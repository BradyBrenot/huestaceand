#pragma once

#include <QObject>
#include <memory>
#include <QReadWriteLock>
#include "devices/deviceprovider.h"

typedef std::unordered_map<deviceprovider_id, std::shared_ptr<DeviceProvider>> deviceProviderMap;

//////////////////////////////////////////////////////////////////////////

struct BaseDeviceProviderLock
{
	bool contains(device_id id);
	std::vector<device_id> keys();
	const Device& operator[](device_id id);

	BaseDeviceProviderLock(std::shared_ptr<class Huestaceand> parent);
	BaseDeviceProviderLock(BaseDeviceProviderLock&& from);

	virtual ~BaseDeviceProviderLock();

protected:
	BaseDeviceProviderLock() : m_parent() {};
	std::shared_ptr<class Huestaceand> m_parent;
};

struct DeviceProviderReadLock : public BaseDeviceProviderLock
{
	DeviceProviderReadLock(std::shared_ptr<class Huestaceand> parent);
	DeviceProviderReadLock(DeviceProviderReadLock&& from);
};

struct DeviceProviderWriteLock : public BaseDeviceProviderLock
{
	DeviceProviderWriteLock(std::shared_ptr<class Huestaceand> parent);
	DeviceProviderWriteLock(DeviceProviderWriteLock&& from);

	virtual ~DeviceProviderWriteLock();

	deviceprovider_id addDevice(std::shared_ptr<struct Device> device);
	void removeDevice(device_id id);

	//#todo: is this dumb to do in bulk?
	void SetDesiredColor(double h, double s, double l, device_id device, light_id light);

private:
	bool didDesiredLightColorChange;
};

//////////////////////////////////////////////////////////////////////////

class Huestaceand : public QObject, public std::enable_shared_from_this<Huestaceand>
{
	Q_OBJECT

public:
	Huestaceand(QObject* parent = nullptr);
	bool isListening();

	deviceprovider_id addDeviceProvider(std::shared_ptr<class DeviceProvider> provider);
	void removeDeviceProvider(deviceprovider_id id);
	const std::shared_ptr<DeviceProvider> getDeviceProvider(deviceprovider_id id);
	const deviceProviderMap getDeviceProviders();

	//To access or write to the array of devices or the current state, you must use the appropriate lock
	DeviceProviderReadLock lockDeviceRead();
	DeviceProviderWriteLock lockDeviceWrite();

public slots:
	//Starts the daemon, begins listening for commands
	bool listen(int port);
	void stop();

signals:
	void desiredLightColorChanged();
	void devicesChanged();
	void listening();
	void stopped();

private:
	deviceprovider_id nextProviderId;
	device_id nextDeviceId;
	std::shared_ptr<class Server> m_server;

	std::unordered_map<device_id, std::shared_ptr<Device> > devices;
	std::vector<std::shared_ptr<class DeviceProviderDiscovery>> discoveries;

	deviceProviderMap m_deviceProviders;
	QMutex deviceProvidersLock;

	QReadWriteLock rwlock;
	friend BaseDeviceProviderLock;
	friend DeviceProviderReadLock;
	friend DeviceProviderWriteLock;
};