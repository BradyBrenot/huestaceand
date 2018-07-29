#pragma once

#include <QObject>
#include <memory>
#include <QReadWriteLock>
#include <QTimer>
#include "devices/deviceprovider.h"

typedef std::unordered_map<deviceprovider_id, std::shared_ptr<DeviceProvider>> deviceProviderMap;

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

public slots:
	//Starts the daemon, begins listening for commands
	bool listen(int port);
	void stop();
	void tick();

signals:
	void listening();
	void stopped();

private:
	std::shared_ptr<class Server> m_server;

	std::vector<std::shared_ptr<class DeviceProviderDiscovery>> discoveries;

	deviceProviderMap m_deviceProviders;
	QMutex deviceProvidersLock;

	friend BaseDeviceProviderLock;
	friend DeviceProviderReadLock;
	friend DeviceProviderWriteLock;

	QTimer tickTimer;
};