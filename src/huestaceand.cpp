#include "huestaceand.h"
#include "server.h"

static void doDeleteLater(QObject *obj)
{
	obj->deleteLater();
}

Huestaceand::Huestaceand(QObject* parent /*= nullptr*/) 
	: QObject(parent),
	m_server()
{

}

bool Huestaceand::listen(int port)
{
	if (m_server)
	{
		return false;
	}

	m_server = QSharedPointer<Server>(new Server(nullptr, port), doDeleteLater);

	if (!m_server) {
		return false;
	}

	connect(m_server.data(), SIGNAL(listening()),
		this, SIGNAL(listening()));

	m_server->start();

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
	return !m_server.isNull() ? m_server->isListening() : false;
}