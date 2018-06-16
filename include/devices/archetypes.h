#pragma once

#include "deviceprovider.h"

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
			DeviceArchetype(QString("HueLight"), std::vector<LightArchetype>({ LightArchetype(0, 0, 0, 1, 1, 1) }))
		}
	};
};

