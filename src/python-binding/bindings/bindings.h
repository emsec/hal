#pragma once

#include "core/interface_gui.h"
#include "core/log.h"
#include "core/plugin_manager.h"
#include "core/utils.h"
#include "def.h"
#include "netlist/boolean_function.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_type/gate_type.h"
#include "netlist/gate_library/gate_type/gate_type_lut.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"
#include "netlist/hdl_parser/hdl_parser_dispatcher.h"
#include "netlist/hdl_writer/hdl_writer_dispatcher.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#ifdef COMPILER_CLANG
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor-modified"
#endif

#include "pybind11/functional.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#pragma GCC diagnostic pop

using map_string_to_set_of_string = std::map<std::string, std::set<std::string>>;

namespace py = pybind11;

void data_container_init(py::module &m);
