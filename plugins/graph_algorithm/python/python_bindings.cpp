
#include "hal_core/python_bindings/python_bindings.h"

#include "graph_algorithm/plugin_graph_algorithm.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#ifdef COMPILER_CLANG
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor-modified"
#endif

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#pragma GCC diagnostic pop

namespace py = pybind11;
namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(graph_algorithm, m)
    {
        m.doc() = "hal GraphAlgorithmPlugin python bindings";
#else
    PYBIND11_PLUGIN(graph_algorithm)
    {
        py::module m("graph_algorithm", "hal GraphAlgorithmPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<GraphAlgorithmPlugin, RawPtrWrapper<GraphAlgorithmPlugin>, BasePluginInterface>(m, "GraphAlgorithmPlugin")
            .def_property_readonly("name", &GraphAlgorithmPlugin::get_name, R"(
                The name of the plugin.

                :type: str
                )")
            .def("get_name", &GraphAlgorithmPlugin::get_name, R"(
                Get the name of the plugin.

                :returns: Plugin name.
                :rtype: str
                )")
            .def_property_readonly("version", &GraphAlgorithmPlugin::get_version, R"(
                The version of the plugin.

                :type: str
                )")
            .def("get_version", &GraphAlgorithmPlugin::get_version, R"(
                Get the version of the plugin.

                :returns: Plugin version.
                :rtype: str
                )")
            .def("get_communities_fast_greedy",
                 &GraphAlgorithmPlugin::get_communities_fast_greedy,
                 py::arg("netlist"),
                 R"(
                Returns the map of community-IDs to communities running the fast-greedy clustering algorithm.

                :param hal_py.Netlist netlist: Netlist (internelly transformed to di-graph)
                :param set[hal_py.get_gate()] gates: Set of gates for which the strongly connected components are determined. (default = empty means that all gates of the netlist are considered)
                :returns: A map of clusters.
                :rtype: dict[set[hal_py.get_gate()]]
                )")
            .def("get_communities_multilevel",
                 &GraphAlgorithmPlugin::get_communities_multilevel,
                 py::arg("netlist"),
                 R"(
                Returns the map of community-IDs to communities running the multilevel clustering algorithm.

                :param hal_py.Netlist netlist: Netlist (internelly transformed to di-graph)
                :param set[hal_py.get_gate()] gates: Set of gates for which the strongly connected components are determined. (default = empty means that all gates of the netlist are considered)
                :returns: A map of clusters.
                :rtype: dict[int,set[hal_py.get_gate()]]
                )")
            .def("get_communities_spinglass", &GraphAlgorithmPlugin::get_communities_spinglass, py::arg("nl"), py::arg("spins"), R"(
                Returns the map of community-IDs to communities running the spinglass clustering algorithm.

                :param hal_py.Netlist netlist: Netlist (internelly transformed to di-graph)
                :param int spins: Amount of spins
                :returns: A map of clusters.
                :rtype: dict[int,set[hal_py.get_gate()]]
                )")
            .def("get_strongly_connected_components",
                 &GraphAlgorithmPlugin::get_strongly_connected_components,
                 py::arg("netlist"),
                 py::arg("gates"),
                 R"(
                Returns the set of strongly connected components.

                :param hal_py.Netlist netlist: Netlist (internelly transformed to di-graph)
                :param set[hal_py.get_gate()] gates: Set of gates for which the strongly connected components are determined. (default = empty means that all gates of the netlist are considered)
                :returns: A set of strongly connected components where each component is a set of gates.
                :rtype: set[set[hal_py.get_gate()]]
                )")
            .def("get_dijkstra_shortest_paths", &GraphAlgorithmPlugin::get_dijkstra_shortest_paths, py::arg("gate"), R"(
                Returns the shortest path distances for one gate to all other gates.

                :param hal_py.get_gate() gate: Gate (starting vertex for Dijkstra's algorithm)
                :returns: A map of path and distance to the starting gate for all pther gates in the netlist.
                :rtype: dict[hal_py.get_gate(),tuple(list[hal_py.get_gate()],int)]
                )")
            .def("get_graph_cut",
                 &GraphAlgorithmPlugin::get_graph_cut,
                 py::arg("netlist"),
                 py::arg("current_gate"),
                 py::arg("depth")              = std::numeric_limits<u32>::max(),
                 py::arg("terminal_gate_type") = std::set<std::string>(),
                 R"(
                Returns a graph cut for a specific gate and depth.

                :param hal_py.Netlist netlist: Netlist (internally transformed to di-graph)
                :param hal_py.get_gate() current_gate: Gate (starting vertex for graph cut)
                :param int depth: Graph cut depth
                :param terminal_gate_type: Marks terminal vertex gate types of graph cut (typically memory gates such as flip-flops).
                :type terminal_gate_type: set[str]
                :returns: A list of gate sets where each list entry refers to the distance to the starting gate.
                :rtype: list[set[hal_py.get_gate()]]
                )");

        // #ifndef PYBIND11_MODULE
        //         return m.ptr();
        // #endif    // PYBIND11_MODULE
        //     }
    }
}    // namespace hal