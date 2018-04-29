#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"
#include "huestaceand.h"

#include <QSignalSpy>

TEST_CASE("daemon starts and stops on command", "") {

	Huestaceand* daemon = new Huestaceand(nullptr);

	SECTION("daemon can be started") {
		QSignalSpy spy(daemon, SIGNAL(listening()));
		daemon->listen();

		REQUIRE(spy.count() == 1);
		REQUIRE(daemon->isListening());
	}

	SECTION("daemon can be stopped") {
		QSignalSpy spy(daemon, SIGNAL(stopped()));
		daemon->stop();

		REQUIRE(spy.count() == 1);
		REQUIRE(!daemon->isListening());
	}
}