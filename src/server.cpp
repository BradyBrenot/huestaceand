#include "server.h"
#include <string>
#include <QDebug>

Server::Server(QObject *parent /*= Q_NULLPTR*/)
	: QThread(parent)
{

}

Server::~Server()
{
	stop();
}

void Server::run()
{
	std::string server_address("0.0.0.0:50051");
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(this);
	m_server = builder.BuildAndStart();
	if (m_server) {
		emit listening();
		qInfo() << "Server listening on " << QString::fromStdString(server_address);
		m_server->Wait();
	}
}

void Server::stop()
{
	if (m_server) {
		m_server->Shutdown();
		m_server = nullptr;
	}
}