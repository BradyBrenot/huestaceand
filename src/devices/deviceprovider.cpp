#include "devices/deviceprovider.h"
#include "devices/archetypes.h"
#include "huestaceand.h"

DeviceProvider::DeviceProvider(DeviceProviderDiscovery* parent /* = nullptr */)
	: QObject(parent), deviceDiscoveryParent(parent)
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

std::shared_ptr<Device> DeviceProvider::createAndAddDeviceFromArchetype(archetype_id archetype, QString name)
{
	auto lock = deviceDiscoveryParent->daemonParent->lockDeviceWrite();
	std::shared_ptr<Device> d = std::shared_ptr<Device>(new Device());
	auto id = lock.addDevice(d);

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