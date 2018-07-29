#pragma once

#include "types.h"

struct LightArchetype
{
	//0.0 to 1.0, in device space
	Box bounds;

	LightArchetype() :
		bounds()
	{

	}

	LightArchetype(Box inBounds) :
		bounds(inBounds)
	{

	}
};

struct DeviceArchetype
{
	QString name;
	std::vector<LightArchetype> Lights;
	DeviceArchetype(QString inName, std::vector<LightArchetype>& inLights) : name(inName), Lights(inLights) {}
};

namespace Archetype
{
	//////////////////////////////////////////////////////////////////////////

	const archetype_id HueBridge = 0x0;
	const archetype_id ChromaSDK = 0x1;

	//////////////////////////////////////////////////////////////////////////

	const archetype_id HueLight = 0x0;

	const std::unordered_map<archetype_id, DeviceArchetype> devices
	{
		{
			HueLight,
			DeviceArchetype(QString("HueLight"), std::vector<LightArchetype>({ LightArchetype(Box(0, 0, 0, 1, 1, 1)) }))
		}
	};
};

