
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
            .def("get_communities", &GraphAlgorithmPlugin::get_communities, py::arg("netlist"), R"(
                Get a dict of community IDs to communities. Each community is represented by a set of gates.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :returns: A dict from community IDs to communities.
                :rtype: dict[int,set[hal_py.get_gate()]]
                )")
            .def("get_communities_spinglass", &GraphAlgorithmPlugin::get_communities_spinglass, py::arg("netlist"), py::arg("spins"), R"(
                Get a dict of community IDs to communities running the spinglass clustering algorithm. Each community is represented by a set of gates.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :param int spins: The number of spins.
                :returns: A dict from community IDs to communities.
                :rtype: dict[int,set[hal_py.get_gate()]]
                )")
            .def("get_communities_fast_greedy", &GraphAlgorithmPlugin::get_communities_fast_greedy, py::arg("netlist"), R"(
                Get a dict of community IDs to communities running the fast greedy clustering algorithm from igraph. Each community is represented by a set of gates.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :returns: A dict from community IDs to communities.
                :rtype: dict[set[hal_py.get_gate()]]
                )")
            .def("get_communities_multilevel", &GraphAlgorithmPlugin::get_communities_multilevel, py::arg("netlist"), R"(
                Get a dict of community IDs to communities running the multilevel clustering algorithm from igraph. Each community is represented by a set of gates.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :returns: A dict from community IDs to communities.
                :rtype: dict[int,set[hal_py.get_gate()]]
                )")
            .def("get_strongly_connected_components", &GraphAlgorithmPlugin::get_strongly_connected_components, py::arg("netlist"), R"(
                Get a list of strongly connected components (SCC) with each SSC being represented by a list of gates.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :returns: A list of SCCs.
                :rtype: list[list[hal_py.get_gate()]]
                )")
            .def("get_graph_cut",
                 &GraphAlgorithmPlugin::get_graph_cut,
                 py::arg("netlist"),
                 py::arg("gate"),
                 py::arg("depth")              = std::numeric_limits<u32>::max(),
                 py::arg("terminal_gate_type") = std::set<std::string>(),
                 R"(
                Get a graph cut for a specific gate and depth. Further, a set of gates can be specified that limit the graph cut, i.e., flip-flops and memory cells.
                The graph cut is returned as a list of sets of gates with the list's index representing the distance of each set to the starting point.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :param hal_py.get_gate() gate: The gate that is the starting point for the graph cut.
                :param int depth: The depth of the graph cut.
                :param set[str] terminal_gate_type:  A set of gates at which to terminate the graph cut.
                :returns: The graph cut as a list of sets of gates.
                :rtype: list[set[hal_py.get_gate()]]
                )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal