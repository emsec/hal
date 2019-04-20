cmake_minimum_required(VERSION 3.8)

include(CMakeDependentOption)

#set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#Change Compiler on macOS to one that supports OpenMP
if(APPLE AND CMAKE_HOST_APPLE)
    #Needs to be set before project(hal)!
    message(STATUS "You need to install a compiler with OpenMP support:")
    message(STATUS "    brew install llvm")
    message(STATUS "    Set Environment Variable:")
    message(STATUS "    LDFLAGS=\"-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib\"")
    message(STATUS "    CPPFLAGS=\"-I/usr/local/opt/llvm/include\"")
    set(CMAKE_C_COMPILER /usr/local/opt/llvm/bin/clang)
    set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
    set(ENV{LDFLAGS} "-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib")
    set(ENV{CPPFLAGS} "-I/usr/local/opt/llvm/include")
endif(APPLE AND CMAKE_HOST_APPLE)

project(BuDDy)

#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUDDY_TARGET bdd)

#Install Directive target directiories
include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/hal)
message(STATUS "INCLUDE_INSTALL_DIRECTORY: ${INCLUDE_INSTALL_DIRECTORY}")

set(INCLUDE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_INCLUDEDIR}/hal)
message(STATUS "INCLUDE_INSTALL_DIRECTORY_FULL: ${INCLUDE_INSTALL_DIRECTORY_FULL}")

set(SHARE_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/hal)
message(STATUS "SHARE_INSTALL_DIRECTORY: ${SHARE_INSTALL_DIRECTORY}")

set(SHARE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_DATADIR}/hal)
message(STATUS "SHARE_INSTALL_DIRECTORY_FULL: ${SHARE_INSTALL_DIRECTORY_FULL}")

set(PKGCONFIG_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/pkgconfig)
message(STATUS "PKGCONFIG_INSTALL_DIRECTORY: ${PKGCONFIG_INSTALL_DIRECTORY}")

set(BINARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_BINDIR})
message(STATUS "BINARY_INSTALL_DIRECTORY: ${BINARY_INSTALL_DIRECTORY}")

set(BINARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_BINDIR})
message(STATUS "BINARY_INSTALL_DIRECTORY_FULL: ${BINARY_INSTALL_DIRECTORY_FULL}")

set(LIBRARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_LIBDIR})
message(STATUS "LIBRARY_INSTALL_DIRECTORY: ${LIBRARY_INSTALL_DIRECTORY}")

set(LIBRARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_LIBDIR})
message(STATUS "LIBRARY_INSTALL_DIRECTORY_FULL: ${LIBRARY_INSTALL_DIRECTORY_FULL}")

# Version
set(BUDDY_VERSION_MAJOR 2)
set(BUDDY_VERSION_MINOR 4)
set(BUDDY_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)

#option(BUDDY_BUILD_EXAMPLES "Build BuDDy Examples" ON)
#if(BUDDY_BUILD_EXAMPLES)
#  add_subdirectory(examples)
#endif()
