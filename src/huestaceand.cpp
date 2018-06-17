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
	m_server(),
	nextProviderId(0),
	nextDeviceId(0)
{

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
	
	return true;
}

void Huestaceand::stop()
{
	m_server->stop();
	m_server = nullptr;
	emit stopped();
}

bool Huestaceand::isListening()
{
	return m_server ? m_server->isListening() : false;
}

deviceprovider_id Huestaceand::addDeviceProvider(std::shared_ptr<class DeviceProvider> provider) {
	auto id = nextProviderId++;
	deviceProvidersLock.lock();
	m_deviceProviders[id] = provider;
	deviceProvidersLock.unlock();

	return id;
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

DeviceProviderReadLock Huestaceand::lockDeviceRead()
{
	return DeviceProviderReadLock(shared_from_this());
}
DeviceProviderWriteLock Huestaceand::lockDeviceWrite()
{
	return DeviceProviderWriteLock(shared_from_this());
}

//////////////////////////////////////////////////////////////////////////

std::vector<device_id> BaseDeviceProviderLock::keys() {
	std::vector<device_id> keys;
	for (const auto& d : m_parent->devices) {
		keys.push_back(d.first);
	}
	return keys;
}

bool BaseDeviceProviderLock::contains(device_id id)
{
	return m_parent->devices.find(id) != m_parent->devices.end();
}

BaseDeviceProviderLock::BaseDeviceProviderLock(std::shared_ptr<class Huestaceand> provider)
	: m_parent(provider)
{

}
BaseDeviceProviderLock::BaseDeviceProviderLock(BaseDeviceProviderLock&& from) {
	if (m_parent) {
		m_parent->rwlock.unlock();
	}

	m_parent = from.m_parent;
	from.m_parent = nullptr;
}

BaseDeviceProviderLock::~BaseDeviceProviderLock() {
	if (m_parent) {
		m_parent->rwlock.unlock();
	}
}

const Device& BaseDeviceProviderLock::operator[](device_id id)
{
	Q_ASSERT(contains(id));
	return *m_parent->devices[id].get();
}

//////////////////////////////////////////////////////////////////////////

DeviceProviderReadLock::DeviceProviderReadLock(std::shared_ptr<class Huestaceand> parent)
	: BaseDeviceProviderLock(parent)
{
	m_parent->rwlock.lockForRead();
}

DeviceProviderReadLock::DeviceProviderReadLock(DeviceProviderReadLock&& from) {
	if (m_parent) {
		m_parent->rwlock.unlock();
	}

	m_parent = from.m_parent;
	from.m_parent = nullptr;
}

//////////////////////////////////////////////////////////////////////////

DeviceProviderWriteLock::DeviceProviderWriteLock(std::shared_ptr<class Huestaceand> parent)
	: BaseDeviceProviderLock(parent),
	didDesiredLightColorChange(false)
{
	m_parent->rwlock.lockForWrite();
}

DeviceProviderWriteLock::DeviceProviderWriteLock(DeviceProviderWriteLock&& from) {
	if (m_parent) {
		m_parent->rwlock.unlock();
	}

	m_parent = from.m_parent;
	didDesiredLightColorChange = from.didDesiredLightColorChange;
	from.m_parent = nullptr;
}

DeviceProviderWriteLock::~DeviceProviderWriteLock()
{
	if (didDesiredLightColorChange) {
		emit m_parent->desiredLightColorChanged();
	}
}

deviceprovider_id DeviceProviderWriteLock::addDevice(std::shared_ptr<struct Device> device)
{
	deviceprovider_id id = m_parent->nextDeviceId++;
	m_parent->devices[id] = device;
	return id;
}
void DeviceProviderWriteLock::removeDevice(device_id id)
{
	m_parent->devices.erase(id);
}

void DeviceProviderWriteLock::SetDesiredColor(double h, double s, double l, device_id device, light_id light)
{
	didDesiredLightColorChange = true;
	auto& d = *m_parent->devices[device].get();
	d.lights[light].desired_h = h;
	d.lights[light].desired_s = s;
	d.lights[light].desired_l = l;
}

//////////////////////////////////////////////////////////////////////////