#include "devices/deviceprovider.h"

DeviceProvider::DeviceProvider(DeviceProviderDiscovery* parent /* = nullptr */)
	: QObject(parent), enable_shared_from_this(), rwlock()
{

}

DeviceProvider::~DeviceProvider()
{

}

DeviceProviderReadLock DeviceProvider::lockDeviceRead()
{
	return DeviceProviderReadLock(shared_from_this());
}
DeviceProviderWriteLock DeviceProvider::lockDeviceWrite()
{
	return DeviceProviderWriteLock(shared_from_this());
}

void DeviceProvider::link()
{
	auto lock = lockDeviceWrite();
	doLink();
}

EDeviceState DeviceProvider::getState() const
{
	return m_state;
}
void DeviceProvider::setState(EDeviceState state)
{
	auto lock = lockDeviceWrite();
	m_state = state;
	emit stateChanged(m_state);
}

std::unique_ptr<Device>& DeviceProvider::createAndAddDeviceFromArchetype(device_id id, archetype_id archetype, QString name)
{
	auto& d =  devices[id] = std::unique_ptr<Device>(new Device());
	d->archetype = archetype;
	d->id = id;
	d->name = name;

	auto& a = qobject_cast<DeviceProviderDiscovery*>(parent())->m_deviceArchetypes[archetype];
	auto& aLights = a->Lights;

	for (auto& light : aLights)
	{
		d->lights.push_back(Light());
	}

	return d;
}

//////////////////////////////////////////////////////////////////////////

std::vector<device_id> BaseDeviceProviderLock::keys() {
	std::vector<device_id> keys;
	for (const auto& d : m_provider->devices) {
		keys.push_back(d.first);
	}
	return keys;
}

EDeviceState BaseDeviceProviderLock::state() {
	return m_provider->getState();
}

bool BaseDeviceProviderLock::contains(device_id id)
{
	return m_provider->devices.find(id) != m_provider->devices.end();
}

BaseDeviceProviderLock::BaseDeviceProviderLock(std::shared_ptr<class DeviceProvider> provider) 
	: m_provider(provider) 
{

}
BaseDeviceProviderLock::BaseDeviceProviderLock(BaseDeviceProviderLock&& from) {
	if (m_provider) {
		m_provider->rwlock.unlock();
	}

	m_provider = from.m_provider;
	from.m_provider = nullptr;
}

BaseDeviceProviderLock::~BaseDeviceProviderLock() {
	if (m_provider) {
		m_provider->rwlock.unlock();
	}
}

const Device& BaseDeviceProviderLock::operator[](device_id id)
{
	Q_ASSERT(contains(id));
	return *m_provider->devices[id].get();
}

//////////////////////////////////////////////////////////////////////////

DeviceProviderReadLock::DeviceProviderReadLock(std::shared_ptr<class DeviceProvider> provider)
	: BaseDeviceProviderLock(m_provider)
{
	m_provider->rwlock.lockForRead();
}

DeviceProviderReadLock::DeviceProviderReadLock(DeviceProviderReadLock&& from) {
	if (m_provider) {
		m_provider->rwlock.unlock();
	}

	m_provider = from.m_provider;
	from.m_provider = nullptr;
}

//////////////////////////////////////////////////////////////////////////

DeviceProviderWriteLock::DeviceProviderWriteLock(std::shared_ptr<class DeviceProvider> provider)
	: BaseDeviceProviderLock(m_provider),
	didDesiredLightColorChange(false)
{
	m_provider->rwlock.lockForWrite();
}

DeviceProviderWriteLock::DeviceProviderWriteLock(DeviceProviderWriteLock&& from) {
	if (m_provider) {
		m_provider->rwlock.unlock();
	}

	m_provider = from.m_provider;
	didDesiredLightColorChange = from.didDesiredLightColorChange;
	from.m_provider = nullptr;
}

DeviceProviderWriteLock::~DeviceProviderWriteLock()
{
	if (didDesiredLightColorChange) {
		emit m_provider->desiredLightColorChanged();
	}
}

void DeviceProviderWriteLock::SetDesiredColor(double h, double s, double l, device_id device, light_id light)
{
	didDesiredLightColorChange = true;
	auto& d = *m_provider->devices[device].get();
	d.lights[light].desired_h = h;
	d.lights[light].desired_s = s;
	d.lights[light].desired_l = l;
}

//////////////////////////////////////////////////////////////////////////

DeviceProviderDiscovery::DeviceProviderDiscovery(QObject* parent /* = nullptr */)
	: QObject(parent),
	m_deviceArchetypes(), m_deviceProviders(), archetypesLock(), deviceProvidersLock()
{

}

//////////////////////////////////////////////////////////////////////////