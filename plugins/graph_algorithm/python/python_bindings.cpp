
#include "hal_core/python_bindings/python_bindings.h"

#include "graph_algorithm/algorithms/components.h"
#include "graph_algorithm/algorithms/neighborhood.h"
#include "graph_algorithm/algorithms/shortest_path.h"
#include "graph_algorithm/algorithms/subgraph.h"
#include "graph_algorithm/netlist_graph.h"
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

        py::class_<GraphAlgorithmPlugin, RawPtrWrapper<GraphAlgorithmPlugin>, BasePluginInterface> py_graph_algorithm_plugin(m, "GraphAlgorithmPlugin");

        py_graph_algorithm_plugin.def_property_readonly("name", &GraphAlgorithmPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_graph_algorithm_plugin.def("get_name", &GraphAlgorithmPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: Plugin name.
            :rtype: str
        )");

        py_graph_algorithm_plugin.def_property_readonly("version", &GraphAlgorithmPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_graph_algorithm_plugin.def("get_version", &GraphAlgorithmPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: Plugin version.
            :rtype: str
        )");

        py::class_<graph_algorithm::NetlistGraph, RawPtrWrapper<graph_algorithm::NetlistGraph>> py_netlist_graph(m, "NetlistGraph", R"(
            Holds a directed graph corresponding to a netlist.
        )");

        py::enum_<graph_algorithm::NetlistGraph::Direction>(py_netlist_graph, "Direction", R"(
            Defines the direction of a pin.
        )")
            .value("NONE", graph_algorithm::NetlistGraph::Direction::NONE, R"(Invalid direction.)")
            .value("IN", graph_algorithm::NetlistGraph::Direction::IN, R"(Vertex fan-in.)")
            .value("OUT", graph_algorithm::NetlistGraph::Direction::OUT, R"(Vertex fan-out.)")
            .value("ALL", graph_algorithm::NetlistGraph::Direction::ALL, R"(All directions.)")
            .export_values();

        py_netlist_graph.def_static(
            "from_netlist",
            [](Netlist* nl, bool create_dummy_vertices = false, const std::function<bool(const Net*)>& filter = nullptr) -> std::unique_ptr<graph_algorithm::NetlistGraph> {
                auto res = graph_algorithm::NetlistGraph::from_netlist(nl, create_dummy_vertices, filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating a graph from a netlist:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("nl"),
            py::arg("create_dummy_vertices") = false,
            py::arg("filter")                = nullptr,
            R"(Create a directed graph from a netlist. Optionally create dummy nodes at nets missing a source or destination. An optional filter can be applied to exclude undesired edges.
             
             :param hal_py.Netlist nl: The netlist.
             :param bool create_dummy_vertices: Set ``True`` to create dummy vertices, ``False`` otherwise. Defaults to ``False``.
             :param lambda filter: An optional filter that is evaluated on every net of the netlist. Defaults to ``None``.
             :returns: The netlist graph on success, ``None`` otherwise.
             :rtype: graph_algorithm.NetlistGraph or None
        )");

        py_netlist_graph.def_static(
            "from_netlist_no_edges",
            [](Netlist* nl) -> std::unique_ptr<graph_algorithm::NetlistGraph> {
                auto res = graph_algorithm::NetlistGraph::from_netlist_no_edges(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating a graph from a netlist:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("nl"),
            R"(Create an empty directed graph from a netlist, i.e., vertices for all gates are created, but no edges are added.
             
             :param hal_py.Netlist nl: The netlist.
             :returns: The netlist graph on success, ``None`` otherwise.
             :rtype: graph_algorithm.NetlistGraph or None
        )");

        py_netlist_graph.def("get_netlist", &graph_algorithm::NetlistGraph::get_netlist, R"(
            Get the netlist associated with the netlist graph.

            :returns: The netlist.
            :rtype: hal_py.Netlist
        )");

        py_netlist_graph.def(
            "get_gates_from_vertices",
            [](const graph_algorithm::NetlistGraph& self, const std::vector<u32>& vertices) -> std::optional<std::vector<Gate*>> {
                auto res = self.get_gates_from_vertices(vertices);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting gates from vertices:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("vertices"),
            R"(
                Get the gates corresponding to the specified list of vertices.
                The result may contain ``None`` for dummy vertices.

                :param list[int] vertices: A list of vertices.
                :returns: A list of gates on success, ``None`` otherwise.
                :rtype: list[hal_py.Gate] or None
        )");

        py_netlist_graph.def(
            "get_gates_from_vertices",
            [](const graph_algorithm::NetlistGraph& self, const std::set<u32>& vertices) -> std::optional<std::vector<Gate*>> {
                const auto res = self.get_gates_from_vertices(vertices);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting gates from vertices:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("vertices"),
            R"(
                Get the gates corresponding to the specified set of vertices.
                The result may contain ``None`` for dummy vertices.

                :param set[int] vertices: A set of vertices.
                :returns: A list of gates on success, ``None`` otherwise.
                :rtype: list[hal_py.Gate] or None
        )");

        py_netlist_graph.def(
            "get_gate_from_vertex",
            [](const graph_algorithm::NetlistGraph& self, const u32 vertex) -> Gate* {
                const auto res = self.get_gate_from_vertex(vertex);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting gate from vertex:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("vertex"),
            R"(
                Get the gates corresponding to the specified vertex.

                :param int vertex: A vertex.
                :returns: A gate on success, ``None`` otherwise.
                :rtype: hal_py.Gate or None
        )");

        py_netlist_graph.def(
            "get_vertices_from_gates",
            [](const graph_algorithm::NetlistGraph& self, const std::vector<Gate*>& gates) -> std::optional<std::vector<u32>> {
                auto res = self.get_vertices_from_gates(gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting vertices from gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gates"),
            R"(
                Get the vertices corresponding to the specified list of gates.

                :param list[hal_py.Gate] gates: A list of gates.
                :returns: A list of vertices on success, ``None`` otherwise.
                :rtype: list[int] or None
        )");

        py_netlist_graph.def(
            "get_vertices_from_gates",
            [](const graph_algorithm::NetlistGraph& self, const std::set<Gate*>& gates) -> std::optional<std::vector<u32>> {
                auto res = self.get_vertices_from_gates(gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting vertices from gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gates"),
            R"(
                Get the vertices corresponding to the specified set of gates.

                :param set[hal_py.Gate] gates: A set of gates.
                :returns: A list of vertices on success, ``None`` otherwise.
                :rtype: list[int] or None
        )");

        py_netlist_graph.def(
            "get_vertex_from_gate",
            [](const graph_algorithm::NetlistGraph& self, Gate* g) -> std::optional<u32> {
                auto res = self.get_vertex_from_gate(g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting vertex from gate:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("g"),
            R"(
                Get the vertex corresponding to the specified gate.

                :param hal_py.Gate g: A gate.
                :returns: A vertex on success, ``None`` otherwise.
                :rtype: int or None
        )");

        py_netlist_graph.def("get_num_vertices", &graph_algorithm::NetlistGraph::get_num_vertices, py::arg("only_connected") = false, R"(
            Get the number of vertices in the netlist graph.

            :param bool only_connected: Set ``True`` to only count vertices connected to at least one edge, ``False`` otherwise. Defaults to ``False``.
            :returns: The number of vertices in the netlist graph.
            :rtype: int
        )");

        py_netlist_graph.def("get_num_edges", &graph_algorithm::NetlistGraph::get_num_edges, R"(
            Get the number of edges in the netlist graph.

            :returns: The number of edges in the netlist graph.
            :rtype: int
        )");

        py_netlist_graph.def(
            "get_vertices",
            [](const graph_algorithm::NetlistGraph& self, bool only_connected = false) -> std::optional<std::vector<u32>> {
                auto res = self.get_vertices(only_connected);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting vertices:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("only_connected") = false,
            R"(
            Get the vertices in the netlist graph.

            :param bool only_connected: Set ``True`` to only return vertices connected to at least one edge, ``False`` otherwise. Defaults to ``False``.
            :returns: A list of vertices on success, ``None`` otherwise.
            :rtype: list[int] or None
        )");

        py_netlist_graph.def(
            "get_edges",
            [](const graph_algorithm::NetlistGraph& self) -> std::optional<std::vector<std::pair<u32, u32>>> {
                auto res = self.get_edges();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting edges:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
            Get the edges between vertices in the netlist graph.

            :returns: A list of edges on success, ``None`` otherwise.
            :rtype: list[tuple(int,int)] or None
        )");

        py_netlist_graph.def(
            "get_edges_in_netlist",
            [](const graph_algorithm::NetlistGraph& self) -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.get_edges_in_netlist();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting edges:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
            Get the edges between gates in the netlist corresponding to the netlist graph.

            :returns: A list of edges on success, ``None`` otherwise.
            :rtype: list[tuple(hal_py.Gate,hal_py.Gate)] or None
        )");

        py_netlist_graph.def(
            "add_edges",
            [](graph_algorithm::NetlistGraph& self, const std::vector<std::pair<Gate*, Gate*>>& edges) -> bool {
                auto res = self.add_edges(edges);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while adding edges:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("edges"),
            R"(
                Add edges between the specified pairs of source and destination gates to the netlist graph.
                The gates must already correspond to vertices in the graph.

                :param list[tuple(hal_py.Gate,hal_py.Gate)] edges: The edges to add as pairs of gates.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
        )");

        py_netlist_graph.def(
            "add_edges",
            [](graph_algorithm::NetlistGraph& self, const std::vector<std::pair<u32, u32>>& edges) -> bool {
                auto res = self.add_edges(edges);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while adding edges:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("edges"),
            R"(
                Add edges between the specified pairs of source and destination vertices to the netlist graph.
                The vertices must already exist in the graph.

                :param list[tuple(int,int)] edges: The edges to add as pairs of vertices.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
        )");

        py_netlist_graph.def(
            "delete_edges",
            [](graph_algorithm::NetlistGraph& self, const std::vector<std::pair<Gate*, Gate*>>& edges) -> bool {
                auto res = self.delete_edges(edges);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while deleting edges:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("edges"),
            R"(
                Delete edges between the specified pairs of source and destination gates from the netlist graph.

                :param list[tuple(hal_py.Gate,hal_py.Gate)] edges: The edges to delete as pairs of gates.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
        )");

        py_netlist_graph.def(
            "delete_edges",
            [](graph_algorithm::NetlistGraph& self, const std::vector<std::pair<u32, u32>>& edges) -> bool {
                auto res = self.delete_edges(edges);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while deleting edges:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("edges"),
            R"(
                Delete edges between the specified pairs of source and destination vertices from the netlist graph.

                :param list[tuple(int,int)] edges: The edges to delete as pairs of vertices.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
        )");

        py_netlist_graph.def("print", &graph_algorithm::NetlistGraph::print, R"(
            Print the edge list of the graph to stdout.
        )");

        m.def(
            "get_connected_components",
            [](graph_algorithm::NetlistGraph* graph, bool strong, u32 min_size = 0) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = graph_algorithm::get_connected_components(graph, strong, min_size);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing connected components:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("strong"),
            py::arg("min_size") = 0,
            R"(
                Compute the (strongly) connected components of the specified graph.
                Returns each connected component as a list of vertices in the netlist graph.

                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param bool strong: Set ``True`` to compute strongly connected components, ``False`` otherwise.
                :param int min_size: Minimal size of a connected component to be part of the result. Set to ``0`` to include all components. Defaults to ``0``.
                :returns: A list of strongly connected components on success, ``None`` otherwise.
                :rtype: list[list[int]] or None
        )");

        m.def(
            "get_neighborhood",
            [](graph_algorithm::NetlistGraph* graph, const std::vector<Gate*>& start_gates, u32 order, graph_algorithm::NetlistGraph::Direction direction, u32 min_dist = 0)
                -> std::optional<std::vector<std::vector<u32>>> {
                auto res = graph_algorithm::get_neighborhood(graph, start_gates, order, direction, min_dist);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing neighborhood:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("start_gates"),
            py::arg("order"),
            py::arg("direction"),
            py::arg("min_dist") = 0,
            R"(
                Compute the neighborhood of the given order for each of the specified gates within the given netlist graph.
                For order 0, only the vertex itself is returned. For order 1, the vertex itself and all vertices that are its direct predecessors and/or successors (depending on the specified direction). For order 2, the neighborhood of order 1 plus all direct predecessors and/or successors of the vertices in order 1 are returned, etc.
                Returns each neighborhood as a list of vertices in the netlist graph.

                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param list[hal_py.Gate] start_gates: A list of gates for which to compute the neighborhood.
                :param int order: The order of the neighborhood to compute.
                :param graph_algorithm.NetlistGraph.Direction direction: The direction in which the neighborhood should be computed.
                :param int min_dist: The minimum distance of the vertices to include in the result.
                :returns: A list of neighborhoods of each of the provided start gates (in order) on success, ``None`` otherwise.
                :rtype: list[list[int]] or None
        )");

        m.def(
            "get_neighborhood",
            [](graph_algorithm::NetlistGraph* graph, const std::vector<u32>& start_vertices, u32 order, graph_algorithm::NetlistGraph::Direction direction, u32 min_dist = 0)
                -> std::optional<std::vector<std::vector<u32>>> {
                auto res = graph_algorithm::get_neighborhood(graph, start_vertices, order, direction, min_dist);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing neighborhood:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("start_vertices"),
            py::arg("order"),
            py::arg("direction"),
            py::arg("min_dist") = 0,
            R"(
                Compute the neighborhood of the given order for each of the specified vertices within the given netlist graph.
                For order 0, only the vertex itself is returned. For order 1, the vertex itself and all vertices that are its direct predecessors and/or successors (depending on the specified direction). For order 2, the neighborhood of order 1 plus all direct predecessors and/or successors of the vertices in order 1 are returned, etc.
                Returns each neighborhood as a list of vertices in the netlist graph.

                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param list[int] start_vertices: A list of vertices for which to compute the neighborhood.
                :param int order: The order of the neighborhood to compute.
                :param graph_algorithm.NetlistGraph.Direction direction: The direction in which the neighborhood should be computed.
                :param int min_dist: The minimum distance of the vertices to include in the result.
                :returns: A list of neighborhoods of each of the provided start vertices (in order) on success, ``None`` otherwise.
                :rtype: list[list[int]] or None
        )");

        m.def(
            "get_shortest_paths",
            [](graph_algorithm::NetlistGraph* graph, Gate* from_gate, const std::vector<Gate*>& to_gates, graph_algorithm::NetlistGraph::Direction direction)
                -> std::optional<std::vector<std::vector<u32>>> {
                auto res = graph_algorithm::get_shortest_paths(graph, from_gate, to_gates, direction);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing shortest paths:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("from_gate"),
            py::arg("to_gates"),
            py::arg("direction"),
            R"(
                Compute a shortest path from the specified ``from_gate`` to each of the given ``to_gates`` by traversing in the provided direction.
                Returns one shortest path for each end gate, even if multiple shortest paths exist.
                Each shortest path is given as a list of vertices in the order of traversal.

                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param hal_py.Gate from_gate: The start gate of the shortest path.
                :param list[hal_py.Gate] to_gates: A list of end gates of the shortest path.
                :param graph_algorithm.NetlistGraph.Direction direction: The direction in which to compute the shortest paths starting at the ``from_gate``.
                :returns: The shortest paths in order of the ``to_gates`` on success, an error otherwise.
                :rtype: list[list[int]] or None
        )");

        m.def(
            "get_shortest_paths",
            [](graph_algorithm::NetlistGraph* graph, u32 from_vertice, const std::vector<u32>& to_vertices, graph_algorithm::NetlistGraph::Direction direction)
                -> std::optional<std::vector<std::vector<u32>>> {
                auto res = graph_algorithm::get_shortest_paths(graph, from_vertice, to_vertices, direction);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing shortest paths:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("from_vertex"),
            py::arg("to_vertices"),
            py::arg("direction"),
            R"(
                Compute a shortest path from the specified ``from_vertex`` to each of the given ``to_vertices`` by traversing in the provided direction.
                Returns one shortest path for each end vertex, even if multiple shortest paths exist.
                Each shortest path is given as a list of vertices in the order of traversal.
                
                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param int from_vertex: The start vertex of the shortest path.
                :param list[int] to_vertices: A list of end vertices of the shortest path.
                :param graph_algorithm.NetlistGraph.Direction direction: The direction in which to compute the shortest paths starting at the ``from_vertex``.
                :returns: The shortest paths in order of the ``to_vertices`` on success, an error otherwise.
                :rtype: list[list[int]] or None
        )");

        m.def(
            "get_all_shortest_paths",
            [](graph_algorithm::NetlistGraph* graph, Gate* from_gate, const std::vector<Gate*>& to_gates, graph_algorithm::NetlistGraph::Direction direction)
                -> std::optional<std::vector<std::vector<u32>>> {
                auto res = graph_algorithm::get_all_shortest_paths(graph, from_gate, to_gates, direction);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing all shortest paths:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("from_gate"),
            py::arg("to_gates"),
            py::arg("direction"),
            R"(
                Compute a shortest path from the specified ``from_gate`` to each of the given ``to_gates`` by traversing in the provided direction.
                Returns all shortest paths for each end gate.
                Each shortest path is given as a list of vertices in the order of traversal.

                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param hal_py.Gate from_gate: The start gate of the shortest path.
                :param list[hal_py.Gate] to_gates: A list of end gates of the shortest path.
                :param graph_algorithm.NetlistGraph.Direction direction: The direction in which to compute the shortest paths starting at the ``from_gate``.
                :returns: The shortest paths in order of the ``to_gates`` on success, an error otherwise.
                :rtype: list[list[int]] or None
        )");

        m.def(
            "get_all_shortest_paths",
            [](graph_algorithm::NetlistGraph* graph, u32 from_vertice, const std::vector<u32>& to_vertices, graph_algorithm::NetlistGraph::Direction direction)
                -> std::optional<std::vector<std::vector<u32>>> {
                auto res = graph_algorithm::get_all_shortest_paths(graph, from_vertice, to_vertices, direction);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing all shortest paths:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("from_vertex"),
            py::arg("to_vertices"),
            py::arg("direction"),
            R"(
                Compute a shortest path from the specified ``from_vertex`` to each of the given ``to_vertices`` by traversing in the provided direction.
                Returns all shortest paths for each end gate.
                Each shortest path is given as a list of vertices in the order of traversal.
                
                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param int from_vertex: The start vertex of the shortest path.
                :param list[int] to_vertices: A list of end vertices of the shortest path.
                :param graph_algorithm.NetlistGraph.Direction direction: The direction in which to compute the shortest paths starting at the ``from_vertex``.
                :returns: The shortest paths in order of the ``to_vertices`` on success, an error otherwise.
                :rtype: list[list[int]] or None
        )");

        m.def(
            "get_subgraph",
            [](graph_algorithm::NetlistGraph* graph, const std::vector<Gate*>& subgraph_gates) -> std::optional<std::unique_ptr<graph_algorithm::NetlistGraph>> {
                auto res = graph_algorithm::get_subgraph(graph, subgraph_gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing subgraph:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("subgraph_gates"),
            R"(
                Compute the subgraph induced by the specified gates, including all edges between the corresponding vertices.

                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param list[hal_py.Gate] subgraph_gates: A list of gates that make up the subgraph.
                :returns: The subgraph as a new netlist graph on success, ``None`` otherwise.
                :rtype: graph_algorithm.NetlistGraph or None
        )");

        m.def(
            "get_subgraph",
            [](graph_algorithm::NetlistGraph* graph, const std::vector<u32>& subgraph_vertices) -> std::optional<std::unique_ptr<graph_algorithm::NetlistGraph>> {
                auto res = graph_algorithm::get_subgraph(graph, subgraph_vertices);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while computing subgraph:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("graph"),
            py::arg("subgraph_vertices"),
            R"(
                Compute the subgraph induced by the specified vertices, including all edges between these vertices.
                
                :param graph_algorithm.NetlistGraph graph: The netlist graph.
                :param list[int] subgraph_vertices: A list of vertices that make up the subgraph.
                :returns: The subgraph as a new netlist graph on success, ``None`` otherwise.
                :rtype: graph_algorithm.NetlistGraph or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal