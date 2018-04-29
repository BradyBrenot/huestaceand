#include "server.h"
#include <string>
#include <QDebug>

Server::Server(QObject *parent /*= Q_NULLPTR*/, int inPort /*= 55589*/)
	: QThread(parent),
	m_server(),
	port(inPort),
	serverMutex()
{

}

Server::~Server()
{
	stop();
}

void Server::run()
{
	std::string server_address = std::string("0.0.0.0:") + std::to_string(port);
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(this);

	serverMutex.lock();
	m_server = builder.BuildAndStart();

	if (m_server) {
		emit listening();
		serverMutex.unlock();

		qInfo() << "Server listening on " << QString::fromStdString(server_address);
		m_server->Wait();

		serverMutex.lock();
		m_server = nullptr;
	}
	
	serverMutex.unlock();
}

void Server::stop()
{
	while (isListening())
	{
		serverMutex.lock();
		if (m_server) {
			m_server->Shutdown();
		}
		serverMutex.unlock();

		QThread::msleep(100);
	}
}

bool Server::isListening()
{
	serverMutex.lock();
	bool hasServer = m_server != nullptr;
	serverMutex.unlock();

	return hasServer;
}