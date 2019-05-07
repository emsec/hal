#include "core/log.h"
#include "core/utils.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "def.h"
#include "plugin_quine_mccluskey.h"

namespace py = pybind11;

#ifdef PYBIND11_MODULE
PYBIND11_MODULE(libquine_mccluskey, m)
{
    m.doc() = "hal libquine_mccluskey python bindings";
#else
PYBIND11_PLUGIN(libquine_mccluskey)
{
    py::module m("libquine_mccluskey", "hal quine mccluskey python bindings");
#endif    // ifdef PYBIND11_MODULE

    py::class_<boolean_function_t>(m, "boolean_function_t")
        .def_readwrite("function", &boolean_function_t::function)
        .def_readwrite("num_of_clauses", &boolean_function_t::num_of_clauses)
        .def_readwrite("num_of_literals", &boolean_function_t::num_of_literals)
        ;

    py::class_<plugin_quine_mccluskey, std::shared_ptr<plugin_quine_mccluskey>>(m, "quine_mccluskey")
        .def(py::init<>())
        .def_property_readonly("name", &plugin_quine_mccluskey::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def("get_name", &plugin_quine_mccluskey::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def_property_readonly("version", &plugin_quine_mccluskey::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)")
        .def("get_version", &plugin_quine_mccluskey::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)")
        .def("get_boolean_function", py::overload_cast<const std::vector<bool>&>(&plugin_quine_mccluskey::get_boolean_function))
        .def("get_boolean_function_str", py::overload_cast<std::shared_ptr<gate> const, bool>(&plugin_quine_mccluskey::get_boolean_function_str), py::arg("gate"), py::arg("css_beautified") = false)
        ;

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}
