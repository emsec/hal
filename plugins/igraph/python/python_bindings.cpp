#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "plugin_igraph.h"

namespace py = pybind11;

#ifdef PYBIND11_MODULE
PYBIND11_MODULE(igraph, m)
{
    m.doc() = "hal igraph python bindings";
#else
PYBIND11_PLUGIN(igraph)
{
    py::module m("igraph", "hal igraph python bindings");
#endif    // ifdef PYBIND11_MODULE

    py::class_<plugin_igraph, std::shared_ptr<plugin_igraph>, i_base>(m, "igraph")
        .def(py::init<>())
        .def_property_readonly("name", &plugin_igraph::get_name)
        .def("get_name", &plugin_igraph::get_name)
        .def_property_readonly("version", &plugin_igraph::get_version)
        .def("get_version", &plugin_igraph::get_version)
        ;

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}
