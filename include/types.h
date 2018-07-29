#pragma once

typedef uint64_t archetype_id;
typedef QString device_id;
typedef uint64_t light_id;
typedef QString deviceprovider_id;

namespace std {
	template <> struct hash<QString>
	{
		size_t operator()(const QString & x) const
		{
			return qHash(x);
		}
	};
}

//axis-aligned bounding box
struct Box
{
	double minX;
	double minY;
	double minZ;

	double maxX;
	double maxY;
	double maxZ;

	Box(double inMinX, double inMinY, double inMinZ, double inMaxX, double inMaxY, double inMaxZ) 
		: minX(inMinX), minY(inMinY), minZ(inMinZ), maxX(inMaxX), maxY(inMaxY), maxZ(inMaxZ)
	{

	}

	Box() : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0)
	{

	}
};