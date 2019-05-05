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

    py::class_<plugin_graph_algorithm, std::shared_ptr<plugin_graph_algorithm>>(m, "graph_algorithm")
        .def(py::init<>())
        .def_property_readonly("name", &plugin_graph_algorithm::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def("get_name", &plugin_graph_algorithm::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def_property_readonly("version", &plugin_graph_algorithm::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)")
        .def("get_version", &plugin_graph_algorithm::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)")
        .def("get_strongly_connected_components",
             [](plugin_graph_algorithm& a, std::shared_ptr<netlist> const nl, const std::set<std::shared_ptr<gate>> gates = {}) -> std::vector<std::set<std::shared_ptr<gate>>> {
                 std::vector<std::set<std::shared_ptr<gate>>> s;
                 auto vals = a.get_strongly_connected_components(nl, gates);
                 std::copy(vals.begin(), vals.end(), std::back_inserter(s));
                 return s;
             },
             py::arg("netlist"),
             py::arg("gates"), R"(
Returns the set of strpngly connected components.

:param netlist: Netlist (internelly transformed to di-graph)
:type netlist: hal_py.netlist
:param gates: Set of gates for which the strongly connected components are determined. (default = empty means that all gates of the netlist are considered)
:type gates: set(hal_py.gate)
:returns: A set of strongly connected components where each component is a set of gates.
:rtype: set(set(hal_py.gate))
)")
        .def("get_dijkstra_shortest_paths", &plugin_graph_algorithm::get_dijkstra_shortest_paths, py::arg("gate"), R"(
Returns the shortest path distances for one gate to all other gates.

:param gate: Gate (starting vertex for Dijkstra's algorithm)
:type gate: hal_py.gate
:returns: A map of path and distance to the starting gate for all pther gates in the netlist.
:rtype: dict[hal_py.gate, tuple(list(hal_py.gate), int)]
)")
        .def("get_bdds", &plugin_graph_algorithm::get_bdds, py::arg("netlist"), py::arg("gates"), py::arg("input_net_to_ids") = std::map<std::shared_ptr<net>, int>(), R"(
Returns the BDD representation for a part of netlist.

:param netlist: Netlist
:type netlist: hal_py.netlist
:param gates: Set of gates defining the submodule for which the BDDs are determined.
:type gates: set(hal_py.gate)
:param input_net_to_ids: Map of submodule input net ids (variable definition X0, X1, ...), if empty the function determines the variable assignment and returns it.
:type input_net_to_ids: dict[hal_py.net, int]
:returns: A tuple of a map of submodule input net to ids (variable definition X0, X1, ...) and a map of submodule output net to BDDs (function defintion f1(x0, ..., x_n), f2(x0, ... x_n), ...).
:rtype: tuple(dict[hal_py.net, int], dict[hal_py.net, hal_py.bdd])
)")
        .def("get_graph_cut",
             &plugin_graph_algorithm::get_graph_cut,
             py::arg("netlist"),
             py::arg("current_gate"),
             py::arg("depth")              = std::numeric_limits<u32>::max(),
             py::arg("terminal_gate_type") = std::set<std::string>(), R"(
Returns a graph cut for a specific gate and depth.

:param netlist: Netlist (internally transformed to di-graph)
:type netlist: hal_py.netlist
:param current_gate: Gate (starting vertex for graph cut)
:type current_gate: hal_py.gate
:param int depth: Graph cut depth
:param terminal_gate_type: Marks terminal vertex gate types of graph cut (typically memory gates such as flip-flops).
:type terminal_gate_type: set(str)
:returns: A list of gate sets where each list entry refers to the distance to the starting gate.
:rtype: list(set(gate))
)");

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}
