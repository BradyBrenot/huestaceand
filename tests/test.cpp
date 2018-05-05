#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"
#include "huestaceand.h"
#include "server.h"

#include <QSignalSpy>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QThread>

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	int result = Catch::Session().run(argc, argv);

	return (result < 0xff ? result : 0xff);
}

static void doDeleteLater(QObject *obj)
{
	obj->deleteLater();
}

static int test_port = 55510;

TEST_CASE("daemon starts and stops on command", "") {

	QSharedPointer<Huestaceand> daemon = QSharedPointer<Huestaceand>(new Huestaceand(nullptr), doDeleteLater);

	SECTION("daemon can be started and signals it's been started") {
		QSignalSpy listeningSpy(daemon.data(), SIGNAL(listening()));
		bool listenCommandAccepted = daemon->listen(test_port++);

		REQUIRE(listenCommandAccepted);

		REQUIRE(listeningSpy.wait(1000));

		REQUIRE(daemon->isListening());
	}

	SECTION("daemon can be stopped") {
		daemon->listen(test_port++);
		QSignalSpy spy(daemon.data(), SIGNAL(stopped()));
		daemon->stop();

		REQUIRE(spy.count() == 1);
		REQUIRE(!daemon->isListening());
	}
}

TEST_CASE("server can start and stop", "") {
	//////////////////////////////////////////////////////////////////////////
	//Startup
	QSharedPointer<Server> server = QSharedPointer<Server>(new Server(nullptr, test_port++), doDeleteLater);

	REQUIRE(!server->isRunning());
	QSignalSpy startedSpy(server.data(), SIGNAL(started()));
	QSignalSpy listeningSpy(server.data(), SIGNAL(listening()));
	server->start();

	bool startedSuccessfully = startedSpy.wait();
	bool listening = listeningSpy.count() == 1 || listeningSpy.wait();
	
	REQUIRE(startedSuccessfully);
	REQUIRE(listening);
	REQUIRE(server->isRunning());

	//////////////////////////////////////////////////////////////////////////
	// Shutdown
	QSignalSpy stoppedSpy(server.data(), SIGNAL(finished()));
	server->stop();
	REQUIRE((stoppedSpy.count() == 1 || stoppedSpy.wait()));
}