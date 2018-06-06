#include "huestaceand.h"
#include "server.h"
#include "utility.h"
#include <QNetworkAccessManager>

#include "devices/hue/huediscovery.h"

static void doDeleteLater(QObject *obj)
{
	obj->deleteLater();
}

QNetworkAccessManager* qnam = nullptr;

Huestaceand::Huestaceand(QObject* parent /*= nullptr*/) 
	: QObject(parent),
	m_server()
{
	if (qnam == nullptr)
		qnam = new QNetworkAccessManager(nullptr);
}

bool Huestaceand::listen(int port)
{
	if (m_server)
	{
		return false;
	}

	m_server = std::shared_ptr<Server>(new Server(nullptr, port), doDeleteLater);

	if (!m_server) {
		return false;
	}

	connect(m_server.get(), SIGNAL(listening()),
		this, SIGNAL(listening()));

	m_server->start();

	discoveries.push_back(std::unique_ptr<class DeviceProviderDiscovery>(new BridgeDiscovery()));

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