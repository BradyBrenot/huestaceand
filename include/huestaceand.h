#pragma once

#include <QObject>
#include <memory>

class Huestaceand : public QObject
{
	Q_OBJECT

public:
	Huestaceand(QObject* parent = nullptr);
	bool isListening();

public slots:
	//Starts the daemon, begins listening for commands
	bool listen(int port);
	void stop();

signals:
	void listening();
	void stopped();

private:
	std::shared_ptr<class Server> m_server;

	std::vector<std::unique_ptr<class DeviceProviderDiscovery>> discoveries;
};