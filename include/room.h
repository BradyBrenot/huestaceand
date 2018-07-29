#pragma once
#include <QObject>
#include "scene.h"
#include "devices/deviceprovider.h"

struct DeviceInRoom
{
	std::shared_ptr<Device> device;
	Box bounds;
	QString uniqueID;

	DeviceInRoom() : device(), bounds(), uniqueID() {

	}

	DeviceInRoom(std::shared_ptr<Device>& inDevice, Box& inBounds, QString& inUniqueId)
		: device(inDevice), bounds(inBounds), uniqueID(inUniqueId)
	{

	}
};

class Room : public QObject
{
	Q_OBJECT

public:
	Room(QObject* parent = nullptr);
	
	void AddDevice(device_id id, Box bounds);

	std::vector<std::shared_ptr<SceneInstanced> > scenes;
	std::vector<DeviceInRoom> devices;
};