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
#include "rpc.grpc.pb.h";

class Server : public QThread, public HuestaceanServer::Service
{
	Q_OBJECT;

public:

	Server(QObject *parent = Q_NULLPTR, int inPort = 55589);
	virtual ~Server();
	void stop();
	bool isListening();

signals:
	void listening();

protected:
	virtual void run() override;

	QMutex serverMutex;
	int port;
	std::unique_ptr<grpc::Server> m_server;
};