#include "huestaceand.h"
#include "server.h"
#include "utility.h"
#include <QNetworkAccessManager>

#include "devices/hue/huediscovery.h"

static void doDeleteLater(QObject *obj)
{
	obj->deleteLater();
}

Huestaceand::Huestaceand(QObject* parent /*= nullptr*/) 
	: QObject(parent),
	enable_shared_from_this(),
	m_server()
{
	connect(&tickTimer, SIGNAL(timeout()), this, SLOT(tick()));
}

bool Huestaceand::listen(int port)
{
	if (m_server) {
		return false;
	}

	m_server = std::shared_ptr<Server>(new Server(this, port), doDeleteLater);

	if (!m_server) {
		return false;
	}

	connect(m_server.get(), SIGNAL(listening()),
		this, SIGNAL(listening()));

	m_server->start();

	discoveries.push_back(std::unique_ptr<class DeviceProviderDiscovery>(new BridgeDiscovery(this)));

	for (auto& disco : discoveries)
	{
		disco->searchForDeviceProviders();
	}

	tickTimer.start(20);
	
	return true;
}

void Huestaceand::stop()
{
	m_server->stop();
	m_server = nullptr;
	tickTimer.stop();
	emit stopped();
}

void Huestaceand::tick()
{
	//Gather lights
}

bool Huestaceand::isListening()
{
	return m_server ? m_server->isListening() : false;
}

deviceprovider_id Huestaceand::addDeviceProvider(std::shared_ptr<class DeviceProvider> provider) {
	deviceProvidersLock.lock();
	m_deviceProviders[provider->id] = provider;
	deviceProvidersLock.unlock();

	return provider->id;
}

const std::shared_ptr<DeviceProvider> Huestaceand::getDeviceProvider(deviceprovider_id id) {
	QMutexLocker lock(&deviceProvidersLock);
	auto provider = m_deviceProviders.find(id);
	if (provider == m_deviceProviders.end()) {
		return std::shared_ptr<DeviceProvider>(nullptr);
	}
	return provider->second;
}

const deviceProviderMap Huestaceand::getDeviceProviders() {
	QMutexLocker lock(&deviceProvidersLock);
	return m_deviceProviders;
}