#include "server.h"
#include "utility.h"
#include "huestaceand.h"
#include "devices/deviceprovider.h"
#include <string>
#include <QDebug>
#include <QMetaObject>

using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

Server::Server(class Huestaceand *parent /*= Q_NULLPTR*/, int inPort /*= 55589*/)
	: QThread(parent),
	m_server(),
	port(inPort),
	serverMutex(),
	daemonParent(parent)
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
	auto rpcProviders = response->mutable_providers();

	auto providers = daemonParent->getDeviceProviders();
	for (auto& provider : providers)
	{
		::libhuestacean::DeviceProvider& rpcProvider = (*rpcProviders)[provider.first];
		const QString name = provider.second->getName();
		rpcProvider.set_archetype_id((uint32_t)provider.second->getArchetype());
		rpcProvider.set_name(name.toUtf8());
		rpcProvider.set_state((libhuestacean::DeviceProvider_State) provider.second->state.load());
	}

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

//////////////////////////////////////////////////////////////////////////

Status Server::Link(ServerContext* context, const libhuestacean::LinkRequest* request, libhuestacean::LinkResponse* response)
{
	auto deviceProvider = daemonParent->getDeviceProvider(request->device_id());
	if (deviceProvider)
	{
		QMetaObject::invokeMethod(deviceProvider.get(), "link", Qt::QueuedConnection);
		return Status::OK;
	}

	return Status(grpc::StatusCode::INVALID_ARGUMENT, grpc::string("requested device provider does not exist"));
}