#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>

typedef uint64_t archetype_id;
typedef uint64_t device_id;
typedef uint64_t light_id;
typedef uint64_t deviceprovider_id;

class Light
{
	double L;
	double C;
	double h;
};

class Device
{
	archetype_id archetype;
	const std::vector<Light>& GetLights() const;
};

class DeviceProvider
{
	const std::vector<std::weak_ptr<Device> >& GetDevices() const;
	
	/*
	
		Signal: found device
	
	*/
};

class LightArchetype
{
	//0.0 to 1.0, in device space
	double x;
	double y;
	double z;
};

class DeviceArchetype
{
	//QString name;
	const std::vector<LightArchetype>& GetLights() const;
};

class DeviceProviderDiscovery
{
	const std::unordered_map<archetype_id, DeviceArchetype>& GetDeviceArchetypes() const;
	const std::unordered_map<deviceprovider_id, DeviceProvider>& GetDeviceProviders() const;

	/*
	Property: Name
	Property: Can search for devices at IP address
	Property: # Low latency supported
	Slot: Search for device providers
	Slot: Search for device provider manually (IP address)
	Signal: Found device provider
	*/
};