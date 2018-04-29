#include <QThread>
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "rpc.grpc.pb.h";

class Server : public QThread, public HuestaceanServer::Service
{

};