#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "hal_core/defines.h"
#include "plugin_test_plugin.h"

namespace py = pybind11;

#ifdef PYBIND11_MODULE
PYBIND11_MODULE(libtest_plugin, m)
{
    m.doc() = "hal libtest_plugin python bindings";
#else
PYBIND11_PLUGIN(libtest_plugin)
        {
                py::Module m("libtest_plugin", "hal test plugin");
#endif    // ifdef PYBIND11_MODULE

        py::class_<boolean_function_t>(m, "boolean_function_t")
        .def_readwrite("function", &boolean_function_t::function)
        .def_readwrite("num_of_clauses", &boolean_function_t::num_of_clauses)
        .def_readwrite("num_of_literals", &boolean_function_t::num_of_literals)
        ;

        py::class_<plugin_test_plugin, std::shared_ptr<plugin_test_plugin>>(m, "test_plugin")
        .def(py::init<>())
        .def_property_readonly("name", &plugin_test_plugin::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def("get_name", &plugin_test_plugin::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def_property_readonly("version", &plugin_test_plugin::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)")
        .def("get_version", &plugin_test_plugin::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)")
        .def("get_boolean_function", py::overload_cast<const std::vector<bool>&>(&plugin_test_plugin::get_boolean_function))
        .def("get_boolean_function_str", py::overload_cast<Gate* const, bool>(&plugin_test_plugin::get_boolean_function_str), py::arg("gate"), py::arg("css_beautified") = false)
        ;

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
        }
