# require 3.15 for GNUInstallDirs
cmake_minimum_required(VERSION 3.15...3.18)

project(QuaZip VERSION 1.3)

set(CMAKE_CXX_STANDARD 14)

set(QUAZIP_LIB_VERSION ${QuaZip_VERSION})
set(QUAZIP_LIB_SOVERSION 1.3.0)

option(BUILD_SHARED_LIBS "" ON)
option(QUAZIP_INSTALL "" ON)
option(QUAZIP_USE_QT_ZLIB "" OFF)
option(QUAZIP_ENABLE_TESTS "Build QuaZip tests" OFF)

# Set the default value of `${QUAZIP_QT_MAJOR_VERSION}`.
# We search quietly for Qt5 and Qt4 in that order.
# Qt5 provides config files for CMake.
# Qt4 relies on `FindQt4.cmake`.
find_package(
  QT NAMES Qt5
  QUIET COMPONENTS Core
)
set(QT_VERSION_MAJOR 5)

set(QUAZIP_QT_MAJOR_VERSION ${QT_VERSION_MAJOR} CACHE STRING "Qt version to use 5, defaults to ${QT_VERSION_MAJOR}")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX d)

set(QUAZIP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QUAZIP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(QUAZIP_LIB_FILE_NAME quazip${QuaZip_VERSION_MAJOR}-qt${QUAZIP_QT_MAJOR_VERSION})
set(QUAZIP_LIB_TARGET_NAME QuaZip)
set(QUAZIP_DIR_NAME QuaZip-Qt${QUAZIP_QT_MAJOR_VERSION}-${QUAZIP_LIB_VERSION})
set(QUAZIP_PACKAGE_NAME QuaZip-Qt${QUAZIP_QT_MAJOR_VERSION})

if(QUAZIP_QT_MAJOR_VERSION EQUAL 5)
	find_package(Qt5 REQUIRED COMPONENTS Core
                         OPTIONAL_COMPONENTS Network Test)
	set(QUAZIP_LIB_LIBRARIES Qt5::Core)
	set(QUAZIP_TEST_QT_LIBRARIES Qt5::Core Qt5::Network Qt5::Test)
	set(QUAZIP_PKGCONFIG_REQUIRES "zlib, Qt5Core")
else()
	message(FATAL_ERROR "Qt version ${QUAZIP_QT_MAJOR_VERSION} is not supported")
endif()

message(STATUS "Using Qt version ${QUAZIP_QT_MAJOR_VERSION}")

set(QUAZIP_QT_ZLIB_USED OFF)
if(QUAZIP_USE_QT_ZLIB)
    find_package(Qt${QUAZIP_QT_MAJOR_VERSION} OPTIONAL_COMPONENTS Zlib)
    if (Qt${QUAZIP_QT_MAJOR_VERSION}Zlib_FOUND)
        set(QUAZIP_LIB_LIBRARIES ${QUAZIP_LIB_LIBRARIES} Qt${QUAZIP_QT_MAJOR_VERSION}::Zlib)
        set(QUAZIP_QT_ZLIB_USED ON)
    endif()
endif()
if(NOT QUAZIP_QT_ZLIB_USED)
    find_package(ZLIB REQUIRED)
    set(QUAZIP_LIB_LIBRARIES ${QUAZIP_LIB_LIBRARIES} ZLIB::ZLIB)
endif()

add_subdirectory(quazip)

if(QUAZIP_ENABLE_TESTS)
    message(STATUS "Building QuaZip tests")
    enable_testing()
    add_subdirectory(qztest)
endif()
