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
#include "plugin_graph_algorithm.h"

namespace py = pybind11;

#ifdef PYBIND11_MODULE
PYBIND11_MODULE(libgraph_algorithm, m)
{
    m.doc() = "hal libgraph_algorithm python bindings";
#else
PYBIND11_PLUGIN(libgraph_algorithm)
{
    py::module m("libgraph_algorithm", "hal graph_algorithm python bindings");
#endif    // ifdef PYBIND11_MODULE
    
    py::implicitly_convertible<std::string, hal::path>();

    py::class_<plugin_graph_algorithm, std::shared_ptr<plugin_graph_algorithm>, i_base>(m, "graph_algorithm")
        .def(py::init<>())
        .def_property_readonly("name", &plugin_graph_algorithm::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
)")
        .def("get_name", &plugin_graph_algorithm::get_name)
        .def_property_readonly("version", &plugin_graph_algorithm::get_version)
        .def("get_version", &plugin_graph_algorithm::get_version)
        .def("get_strongly_connected_components",
             [](plugin_graph_algorithm& a, std::shared_ptr<netlist> const nl, const std::set<std::shared_ptr<gate>> gates = {}) -> std::vector<std::set<std::shared_ptr<gate>>> {
                 std::vector<std::set<std::shared_ptr<gate>>> s;
                 auto vals = a.get_strongly_connected_components(nl, gates);
                 std::copy(vals.begin(), vals.end(), std::back_inserter(s));
                 return s;
             },
             py::arg("netlist"),
             py::arg("gates"))
        .def("get_dijkstra_shortest_paths", &plugin_graph_algorithm::get_dijkstra_shortest_paths, py::arg("gates"))
        .def("get_bdds", &plugin_graph_algorithm::get_bdds, py::arg("netlist"), py::arg("gates"), py::arg("input_net_to_ids") = std::map<std::shared_ptr<net>, int>())
        .def("get_graph_cut",
             &plugin_graph_algorithm::get_graph_cut,
             py::arg("netlist"),
             py::arg("current_gates"),
             py::arg("depth")              = std::numeric_limits<u32>::max(),
             py::arg("terminal_gate_type") = std::set<std::string>());

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}
