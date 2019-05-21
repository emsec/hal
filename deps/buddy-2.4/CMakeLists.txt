#cmake_minimum_required(VERSION 3.8)
#
#include(CMakeDependentOption)
#
##set default build type
#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
#endif(NOT CMAKE_BUILD_TYPE)
#
#if(UNIX AND NOT APPLE)
#  set(LINUX TRUE)
#endif()

##Change Compiler on macOS to one that supports OpenMP
#if(APPLE AND CMAKE_HOST_APPLE)
#  #Needs to be set before project(hal)!
#  message(STATUS "You need to install a compiler with OpenMP support:")
#  message(STATUS "    brew install llvm")
#  message(STATUS "    Set Environment Variable:")
#  message(STATUS "    LDFLAGS=\"-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib\"")
#  message(STATUS "    CPPFLAGS=\"-I/usr/local/opt/llvm/include\"")
#  set(CMAKE_C_COMPILER /usr/local/opt/llvm/bin/clang)
#  set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
#  set(ENV{LDFLAGS} "-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib")
#  set(ENV{CPPFLAGS} "-I/usr/local/opt/llvm/include")
#endif(APPLE AND CMAKE_HOST_APPLE)

project(BuDDy)

#set_property(GLOBAL PROPERTY USE_FOLDERS ON)


##Install Directive target directiories
#include(GNUInstallDirs)
#set(INCLUDE_INSTALL_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/hal)
#message(STATUS "INCLUDE_INSTALL_DIRECTORY: ${INCLUDE_INSTALL_DIRECTORY}")
#
#set(INCLUDE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_INCLUDEDIR}/hal)
#message(STATUS "INCLUDE_INSTALL_DIRECTORY_FULL: ${INCLUDE_INSTALL_DIRECTORY_FULL}")
#
#set(SHARE_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/hal)
#message(STATUS "SHARE_INSTALL_DIRECTORY: ${SHARE_INSTALL_DIRECTORY}")
#
#set(SHARE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_DATADIR}/hal)
#message(STATUS "SHARE_INSTALL_DIRECTORY_FULL: ${SHARE_INSTALL_DIRECTORY_FULL}")
#
#set(PKGCONFIG_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/pkgconfig)
#message(STATUS "PKGCONFIG_INSTALL_DIRECTORY: ${PKGCONFIG_INSTALL_DIRECTORY}")
#
#set(BINARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_BINDIR})
#message(STATUS "BINARY_INSTALL_DIRECTORY: ${BINARY_INSTALL_DIRECTORY}")
#
#set(BINARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_BINDIR})
#message(STATUS "BINARY_INSTALL_DIRECTORY_FULL: ${BINARY_INSTALL_DIRECTORY_FULL}")
#
#set(LIBRARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_LIBDIR})
#message(STATUS "LIBRARY_INSTALL_DIRECTORY: ${LIBRARY_INSTALL_DIRECTORY}")
#
#set(LIBRARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_LIBDIR})
#message(STATUS "LIBRARY_INSTALL_DIRECTORY_FULL: ${LIBRARY_INSTALL_DIRECTORY_FULL}")

# Version
set(BUDDY_VERSION_MAJOR 2)
set(BUDDY_VERSION_MINOR 4)
set(BUDDY_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(CheckIncludeFile)
check_include_file(dlfcn.h HAVE_DLFCN_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h HAVE_UNISTD_H)
set(YYTEXT_POINTER 1)

set(BUDDY_PACKAGE_BUGREPORT "")
set(BUDDY_PACKAGE_NAME "hal_buddy")
set(BUDDY_PACKAGE_STRING "hal_buddy 2.4")
set(BUDDY_PACKAGE_TARNAME "hal_buddy")
set(BUDDY_PACKAGE_VERSION "2.4")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in hal_bdd_config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(BUDDY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}")
set(BUDDY_INCLUDE_DIR ${BUDDY_INCLUDE_DIR} PARENT_SCOPE)
message(STATUS "BUDDY_INCLUDE_DIR: ${BUDDY_INCLUDE_DIR}")

add_subdirectory(src)

#option(BUDDY_BUILD_EXAMPLES "Build BuDDy Examples" OFF)
#if(BUDDY_BUILD_EXAMPLES)
#  add_subdirectory(examples)
#endif()
