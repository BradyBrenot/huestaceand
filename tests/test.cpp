#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include "huestaceand.h"

TEST_CASE("daemon starts and stops on command", "") {

	Huestaceand* daemon = new Huestaceand(nullptr);

	SECTION("daemon can be started") {
		daemon->listen();
	}

	SECTION("daemon can be stopped") {
		daemon->stop();
		//int* bean = nullptr;
		//int hello = *bean;
		//REQUIRE(hello == 3);
	}

	SECTION("daemon can be something") {
		REQUIRE(1 == 2);
	}
}

TEST_CASE("asdfasdfasdfommand", "") {
	REQUIRE(2 == 4);
}