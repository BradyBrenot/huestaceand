#pragma once

#include <QObject>
#include <QSharedPointer>

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
	QSharedPointer<class Server> m_server;
};