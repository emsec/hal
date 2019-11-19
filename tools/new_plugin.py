#!/usr/bin/env python3
import sys
import os

def print_usage():
    print("HAL plugin template generator")
    print("  usage: new_plugin <name>")
    print("")
    print("Sets up the directory structure and respective files in the current directory:")
    print("<name>/")
    print(" |- include/")
    print(" | |- plugin_<name>.h")
    print(" |- python/")
    print(" | |- python_bindings.cpp")
    print(" |- src/")
    print(" | |- plugin_<name>.cpp")
    print(" |- CMakeLists.txt")
    print("")


#################################################################
##############             Templates               ##############
#################################################################

CMAKE_TEMPLATE ="""option(PL_##UPPER## "PL_##UPPER##" OFF)
if(PL_##UPPER## OR BUILD_ALL_PLUGINS)
    include_directories(SYSTEM ${PYBIND11_INCLUDE_DIR} SYSTEM ${PYTHON_INCLUDE_DIRS})
    include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}/include")

    file(GLOB_RECURSE ##UPPER##_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE ##UPPER##_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE ##UPPER##_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    add_library(##LOWER## SHARED ${##UPPER##_SRC} ${##UPPER##_PYTHON_SRC} ${##UPPER##_INC})

    set_target_properties(##LOWER## PROPERTIES PREFIX "")
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(##LOWER## PROPERTIES SUFFIX ".so")
        set_target_properties(##LOWER## PROPERTIES INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY})
    endif(APPLE AND CMAKE_HOST_APPLE)

    target_link_libraries(##LOWER## ${LINK_LIBS} ${PYTHON_LIBRARIES} pybind11::module)

    install(TARGETS ##LOWER## LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/##LOWER##/include/)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(##LOWER##)
    endif()
endif()
"""

#################################################################
#################################################################

PLUGIN_H_TEMPLATE = """#pragma once

#include "core/interface_base.h"

class PLUGIN_API plugin_##LOWER## : virtual public i_base
{
public:

    std::string get_name() const override;
    std::string get_version() const override;

    void initialize() override;
};

// suppress warning about incomplete type "shared_ptr" for C
// required so that "get_plugin_instance" can be found by name in the dynamic library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
extern "C" PLUGIN_API std::shared_ptr<i_base> get_plugin_instance();
#pragma GCC diagnostic pop
"""

#################################################################
#################################################################

PLUGIN_CPP_TEMPLATE = """#include "plugin_##LOWER##.h"

extern std::shared_ptr<i_base> get_plugin_instance()
{
    return std::make_shared<plugin_##LOWER##>();
}

std::string plugin_##LOWER##::get_name() const
{
    return std::string("##LOWER##");
}

std::string plugin_##LOWER##::get_version() const
{
    return std::string("0.1");
}

void plugin_##LOWER##::initialize()
{

}
"""

#################################################################
#################################################################

PYTHON_CPP_TEMPLATE = """#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "plugin_##LOWER##.h"

namespace py = pybind11;

#ifdef PYBIND11_MODULE
PYBIND11_MODULE(##LOWER##, m)
{
    m.doc() = "hal ##LOWER## python bindings";
#else
PYBIND11_PLUGIN(##LOWER##)
{
    py::module m("##LOWER##", "hal ##LOWER## python bindings");
#endif    // ifdef PYBIND11_MODULE

    py::class_<plugin_##LOWER##, std::shared_ptr<plugin_##LOWER##>, i_base>(m, "##LOWER##")
        .def_property_readonly("name", &plugin_##LOWER##::get_name)
        .def("get_name", &plugin_##LOWER##::get_name)
        .def_property_readonly("version", &plugin_##LOWER##::get_version)
        .def("get_version", &plugin_##LOWER##::get_version)
        ;

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}
"""


#################################################################
##############                CORE                 ##############
#################################################################

def create_plugin(name):
    lower = name.lower()
    upper = name.upper()

    os.makedirs(name)
    with open(name+"/CMakeLists.txt", "wt") as f:
        f.write(CMAKE_TEMPLATE.replace("##UPPER##", upper).replace("##LOWER##", lower))

    os.makedirs(name+"/include")
    with open(name+"/include/plugin_"+lower+".h", "wt") as f:
        f.write(PLUGIN_H_TEMPLATE.replace("##UPPER##", upper).replace("##LOWER##", lower))

    os.makedirs(name+"/src")
    with open(name+"/src/plugin_"+lower+".cpp", "wt") as f:
        f.write(PLUGIN_CPP_TEMPLATE.replace("##UPPER##", upper).replace("##LOWER##", lower))

    os.makedirs(name+"/python")
    with open(name+"/python/python_bindings.cpp", "wt") as f:
        f.write(PYTHON_CPP_TEMPLATE.replace("##UPPER##", upper).replace("##LOWER##", lower))

if len(sys.argv) != 2:
    print_usage()
    sys.stderr.write("ERROR: unsupported number of parameters\n")
    sys.exit(-1)

name = sys.argv[1].lower()

if not name.replace("_","").isalnum() or name[0] == "_" or name[0].isnumeric():
    print_usage()
    sys.stderr.write("ERROR: '{}' is not a valid C++ identifier\n".format(name))
    sys.exit(-1)

if os.path.exists(name):
    print_usage()
    sys.stderr.write("ERROR: directory '{}' already exists\n".format(name))
    sys.exit(-1)

create_plugin(sys.argv[1])
