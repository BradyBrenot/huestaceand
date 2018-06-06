#include "server.h"
#include "utility.h"
#include <string>
#include <QDebug>

using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

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
	} else {
		qWarning() << "failed to start server!";
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

Status Server::GetDeviceProviders(ServerContext* context, const libhuestacean::GetDeviceProvidersRequest* request, libhuestacean::GetDeviceProvidersResponse* response)
{
	return Status::OK;
}

Status Server::GetDevices(ServerContext* context, const libhuestacean::GetDevicesRequest* request, libhuestacean::GetDevicesResponse* response)
{
	return Status::OK;
}
Status Server::GetRooms(ServerContext* context, const libhuestacean::GetRoomsRequest* request, libhuestacean::GetRoomsResponse* response)
{
	return Status::OK;
}
Status Server::GetDeviceProviderArchetypes(ServerContext* context, const libhuestacean::GetDeviceProviderArchetypesRequest* request, libhuestacean::GetDeviceProviderArchetypesResponse* response)
{
	return Status::OK;
}
Status Server::GetDeviceArchetypes(ServerContext* context, const libhuestacean::GetDeviceArchetypesRequest* request, libhuestacean::GetDeviceArchetypesResponse* response)
{
	return Status::OK;
}