#pragma once

#include <QObject>

class Huestaceand : public QObject
{
	Q_OBJECT

public:
	Huestaceand(QObject* parent = nullptr);

public slots:
	//Starts the daemon, begins listening for commands
	void listen();
	void stop();

signals:
	void listening();
	void stopped();
};