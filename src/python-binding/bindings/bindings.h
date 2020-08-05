#pragma once

#include "core/plugin_interface_gui.h"
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

namespace hal
{
    using map_string_to_set_of_string = std::map<std::string, std::set<std::string>>;

    namespace py = pybind11;

    void path_init(py::module& m);

    void data_container_init(py::module& m);

    void core_utils_init(py::module& m);

    void gate_type_init(py::module& m);

    void gate_type_lut_init(py::module& m);

    void gate_type_sequential_init(py::module& m);

    void gate_library_init(py::module& m);

    void endpoint_init(py::module& m);

    void netlist_init(py::module& m);

    void gate_init(py::module& m);

    void net_init(py::module& m);

    void module_init(py::module& m);

    void netlist_factory_init(py::module& m);

    void hdl_writer_dispatcher_init(py::module& m);

    void plugin_manager_init(py::module& m);

    void plugin_interfaces_init(py::module& m);

    void boolean_function_init(py::module& m);
}    // namespace hal
