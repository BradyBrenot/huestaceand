#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"
#include "huestaceand.h"
#include "server.h"

#include <QSignalSpy>
#include <QSharedPointer>
#include <QElapsedTimer>

static void doDeleteLater(QObject *obj)
{
	obj->deleteLater();
}

TEST_CASE("daemon starts and stops on command", "") {

	QSharedPointer<Huestaceand> daemon = QSharedPointer<Huestaceand>(new Huestaceand(nullptr), doDeleteLater);

	SECTION("daemon can be started") {
		QSignalSpy spy(daemon.data(), SIGNAL(listening()));
		daemon->listen();

		REQUIRE(spy.count() == 1);
		REQUIRE(daemon->isListening());
	}

	SECTION("daemon can be stopped") {
		QSignalSpy spy(daemon.data(), SIGNAL(stopped()));
		daemon->stop();

		REQUIRE(spy.count() == 1);
		REQUIRE(!daemon->isListening());
	}
}

TEST_CASE("server can start and stop", "") {
	//////////////////////////////////////////////////////////////////////////
	//Startup
	Sleep(100);
	QSharedPointer<Server> server = QSharedPointer<Server>(new Server(nullptr), doDeleteLater);

	REQUIRE(!server->isRunning());
	QSignalSpy startedSpy(server.data(), SIGNAL(started()));
	QSignalSpy listeningSpy(server.data(), SIGNAL(listening()));
	server->start();

	QElapsedTimer timer;
	timer.start();

	//wait until the server has started
	while (startedSpy.count() != 1 && !timer.hasExpired(1000))
	{
		Sleep(100);
	}
	
	REQUIRE(startedSpy.count() == 1);
	REQUIRE(listeningSpy.count() == 1);
	REQUIRE(server->isRunning());

	//////////////////////////////////////////////////////////////////////////
	// Shutdown
	QSignalSpy stoppedSpy(server.data(), SIGNAL(finished()));
	server->stop();
	Sleep(100);
	REQUIRE(stoppedSpy.count() == 1);
}