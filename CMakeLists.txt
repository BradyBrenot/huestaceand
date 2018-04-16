cmake_minimum_required(VERSION 3.9)
project(huestaceand)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(HUESTACEAN_NO_CLIENT 1)

add_subdirectory(libhuestacean)
add_subdirectory(screen_capture_lite EXCLUDE_FROM_ALL)
add_subdirectory(mbedtls EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

set(SOURCES
   ${SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/src/bridgediscovery.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/entertainment.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/huebridge.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/huestacean.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
set(HEADERS
   ${HEADERS}
   ${CMAKE_CURRENT_SOURCE_DIR}/include/bridgediscovery.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/entertainment.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/huebridge.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/huestacean.h
)

QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/libhuestacean/include 
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

add_executable(huestaceand ${SOURCES} ${HEADERS} ${HEADERS_MOC})
target_link_libraries(huestaceand ${LIBRARIES})
