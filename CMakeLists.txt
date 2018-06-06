cmake_minimum_required(VERSION 3.8)
project(huestaceand)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
endif()

add_subdirectory(libhuestacean)
add_subdirectory(screen_capture_lite EXCLUDE_FROM_ALL)
add_subdirectory(mbedtls EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

include(libhuestacean/cmake/Win32Macro.cmake)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

set(MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

set(SOURCES
   ${SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/src/huestaceand.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/devices/deviceprovider.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/devices/hue/huediscovery.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/devices/hue/huebridge.cpp
)
set(HEADERS
   ${HEADERS}
   ${CMAKE_CURRENT_SOURCE_DIR}/include/huestaceand.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/server.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/devices/deviceprovider.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/devices/hue/huediscovery.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/devices/hue/huebridge.h
)

QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/libhuestacean/include 
	${CMAKE_CURRENT_BINARY_DIR}/libhuestacean
	${CMAKE_CURRENT_SOURCE_DIR}
	${Qt5Core_INCLUDES}
	${Qt5Network_INCLUDES}
)

include(ExternalProject REQUIRED)


set(LIBRARIES
   ${LIBRARIES}
   Qt5::Core
   Qt5::Network
   screen_capture_lite
   mbedtls
   mbedx509
   mbedcrypto
   libhuestacean
)

#screen capture additional requirements
if(WIN32)
   set(LIBRARIES
      ${LIBRARIES}
      Dwmapi
	  wsock32
	  ws2_32
	  legacy_stdio_definitions
   )
elseif(APPLE)
   find_library(corefoundation_lib CoreFoundation)
   find_library(cocoa_lib Cocoa)
   find_library(coremedia_lib CoreMedia)
   find_library(avfoundation_lib AVFoundation)
   find_library(coregraphics_lib CoreGraphics)
   find_library(corevideo_lib CoreVideo)

   set(LIBRARIES
      ${LIBRARIES}
      ${corefoundation_lib}
      ${cocoa_lib}
      ${coremedia_lib}
      ${avfoundation_lib}
      ${coregraphics_lib}
      ${corevideo_lib}
   )
    set(SOURCES
        ${SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mac/screencapture.mm
    )
    set(HEADERS
        ${HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mac/screencapture.h
    )
elseif(LINUX)
	set(LIBRARIES
      ${LIBRARIES}
      pthread
	  X11
	  Xext
	  Xinerama
	  Xfixes
   )
endif()

add_executable(huestaceand ${MAIN_SOURCE} ${SOURCES} ${HEADERS} ${HEADERS_MOC})
target_link_libraries(huestaceand ${LIBRARIES})

### Tests

enable_testing()

find_package(Qt5Test REQUIRED)
include_directories(
	${Qt5Test_INCLUDES}
)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

set(TEST_SOURCES
	${SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/tests/api.cpp
)

set(TEST_HEADERS
	${HEADERS}
	${HEADERS_MOC}
)

set(TEST_LIBRARIES
   ${LIBRARIES}
   Catch
   Qt5::Test
)

# Make test executable
add_executable(huestaceand_tests ${TEST_SOURCES} ${TEST_HEADERS})
target_link_libraries(huestaceand_tests ${TEST_LIBRARIES})

#Problem: you need to rerun cmake every time you add/remove a test
#There's another script, CatchAddTests, but it won't fully function without cmake 3.10 (VS2017 is on 3.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/tests/catch/cmake/ParseAndAddCatchTests.cmake)
ParseAndAddCatchTests(huestaceand_tests)

#include(${CMAKE_CURRENT_SOURCE_DIR}/tests/catch/cmake/Catch.cmake)
#catch_discover_tests(huestaceand_tests)