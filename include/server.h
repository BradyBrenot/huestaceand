#pragma once

#include <memory>

#include <QThread>
#include <QSharedPointer>
#include <QAtomicInt>
#include <QMutex>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "rpc.grpc.pb.h"

class Server : public QThread, public libhuestacean::HuestaceanServer::Service
{
	Q_OBJECT

public:

	Server(class Huestaceand *parent, int inPort = 55610);
	virtual ~Server();
	void stop();
	bool isListening();

	virtual ::grpc::Status GetDeviceProviders(::grpc::ServerContext* context, const libhuestacean::GetDeviceProvidersRequest* request, libhuestacean::GetDeviceProvidersResponse* response) override;
	virtual ::grpc::Status GetDevices(::grpc::ServerContext* context, const libhuestacean::GetDevicesRequest* request, libhuestacean::GetDevicesResponse* response) override;
	virtual ::grpc::Status GetRooms(::grpc::ServerContext* context, const libhuestacean::GetRoomsRequest* request, libhuestacean::GetRoomsResponse* response) override;
	virtual ::grpc::Status GetDeviceProviderArchetypes(::grpc::ServerContext* context, const libhuestacean::GetDeviceProviderArchetypesRequest* request, libhuestacean::GetDeviceProviderArchetypesResponse* response) override;
	virtual ::grpc::Status GetDeviceArchetypes(::grpc::ServerContext* context, const libhuestacean::GetDeviceArchetypesRequest* request, libhuestacean::GetDeviceArchetypesResponse* response) override;
	virtual ::grpc::Status Link(::grpc::ServerContext* context, const libhuestacean::LinkRequest* request, libhuestacean::LinkResponse* response) override;

signals:
	void listening();

protected:
	virtual void run() override;

	QMutex serverMutex;
	int port;
	std::unique_ptr<grpc::Server> m_server;
	Huestaceand* daemonParent;
};