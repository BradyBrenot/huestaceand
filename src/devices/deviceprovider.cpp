#include "devices/deviceprovider.h"
#include "devices/archetypes.h"
#include "huestaceand.h"

DeviceProvider::DeviceProvider(DeviceProviderDiscovery* parent /* = nullptr */)
	: QObject(parent), enable_shared_from_this(), deviceDiscoveryParent(parent), rwlock()
{
	qRegisterMetaType<EDeviceState>("EDeviceState");
}

DeviceProvider::~DeviceProvider()
{

}

void DeviceProvider::link()
{
	doLink();
}

void DeviceProvider::setState(EDeviceState inState)
{
	state = (uint8_t) inState;
	emit stateChanged(inState);
}

std::shared_ptr<Device> DeviceProvider::createAndAddDeviceFromArchetype(archetype_id archetype, device_id id, QString name)
{
	auto lock = lockDeviceWrite();
	std::shared_ptr<Device> d = std::shared_ptr<Device>(new Device());
	d->id = id;

	devices[id] = d;
	d->archetype = archetype;
	d->id = id;
	d->name = name;

	auto& a = Archetype::devices.at(Archetype::HueLight);
	auto& aLights = a.Lights;

	for (auto& light : aLights)
	{
		d->lights.push_back(Light());
	}

	return d;
}

DeviceProviderDiscovery::DeviceProviderDiscovery(Huestaceand* parent /* = nullptr */)
	: QObject(parent), daemonParent(parent)
{

}

//////////////////////////////////////////////////////////////////////////


DeviceProviderReadLock DeviceProvider::lockDeviceRead()
{
	return DeviceProviderReadLock(shared_from_this());
}
DeviceProviderWriteLock DeviceProvider::lockDeviceWrite()
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

BaseDeviceProviderLock::BaseDeviceProviderLock(std::shared_ptr<class DeviceProvider> provider)
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

DeviceProviderReadLock::DeviceProviderReadLock(std::shared_ptr<class DeviceProvider> parent)
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

DeviceProviderWriteLock::DeviceProviderWriteLock(std::shared_ptr<class DeviceProvider> parent)
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

void DeviceProviderWriteLock::SetDesiredColor(double h, double s, double l, device_id device, light_id light)
{
	didDesiredLightColorChange = true;
	auto& d = *m_parent->devices[device].get();
	d.lights[light].desired_h = h;
	d.lights[light].desired_s = s;
	d.lights[light].desired_l = l;
}

//////////////////////////////////////////////////////////////////////////