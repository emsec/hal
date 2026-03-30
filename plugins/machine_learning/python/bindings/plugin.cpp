#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_plugin(py::module& m)
            {
        py::class_<MachineLearningPlugin, RawPtrWrapper<MachineLearningPlugin>, BasePluginInterface> py_machine_learning_plugin(
            m, "MachineLearningPlugin", R"(Provides machine learning functionality as a plugin within the HAL framework.)");

        py_machine_learning_plugin.def("get_name", &MachineLearningPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The plugin name.
            :rtype: str
        )");

        py_machine_learning_plugin.def("get_version", &MachineLearningPlugin::get_version, R"(
            Get the plugin version.

            :returns: The plugin version.
            :rtype: str
        )");

        py_machine_learning_plugin.def("get_description", &MachineLearningPlugin::get_description, R"(
            Get the plugin description.

            :returns: The plugin description.
            :rtype: str
        )");

        py_machine_learning_plugin.def("get_dependencies", &MachineLearningPlugin::get_dependencies, R"(
            Get the plugin dependencies.

            :returns: The plugin dependencies.
            :rtype: set(str)
        )");

        py::enum_<machine_learning::GraphDirection>(m, "GraphDirection", R"(
Specifies whether the graph is considered undirected or directed.
)")
            .value("undirected", machine_learning::GraphDirection::undirected, R"(
An undirected graph.
)")
            .value("directed", machine_learning::GraphDirection::directed, R"(
A directed graph.
)")
            .export_values();

        py::class_<machine_learning::MultiBitInformation> py_multibitinformation(m, "MultiBitInformation", R"(
Holds mappings between word labels and gates, and gates and word labels.

This struct provides a bi-directional mapping between specific word pairs and their corresponding gates, 
as well as between gates and associated word pairs.
)");

        py_multibitinformation.def("are_gates_considered_a_pair",
                                   &machine_learning::MultiBitInformation::are_gates_considered_a_pair,
                                   py::arg("direction"),
                                   py::arg("g_a"),
                                   py::arg("g_b"),
                                   R"(
Check if two gates are considered a pair.

:param hal_py.PinDirection direction: The pin direction.
:param hal_py.Gate g_a: The first gate.
:param hal_py.Gate g_b: The second gate.
:returns: True if the gates are considered a pair, False otherwise.
:rtype: bool
)");

        py_multibitinformation.def("is_index_a_smaller_index_b",
                                   &machine_learning::MultiBitInformation::is_index_a_smaller_index_b,
                                   py::arg("direction"),
                                   py::arg("g_a"),
                                   py::arg("g_b"),
                                   R"(
Check if the index of gate g_a is smaller than the index of gate g_b in the specified direction.

:param hal_py.PinDirection direction: The pin direction.
:param hal_py.Gate g_a: The first gate.
:param hal_py.Gate g_b: The second gate.
:returns: True if g_a's index is smaller, False if g_b's index is smaller, or None if no information is available.
:rtype: bool or None
)");

        py_multibitinformation.def_readwrite("word_to_gates",
                                             &machine_learning::MultiBitInformation::word_to_gates,
                                             R"(
Maps word pairs to corresponding gates.

:type: dict[(str, hal_py.PinDirection, str), list[hal_py.Gate]]
)");

        py_multibitinformation.def_readwrite("gate_to_words",
                                             &machine_learning::MultiBitInformation::gate_to_words,
                                             R"(
Maps gates to associated word pairs.

:type: dict[hal_py.Gate, list[tuple(str, hal_py.PinDirection, str)]]
)");

        py_multibitinformation.def_readwrite("gate_word_to_index",
                                             &machine_learning::MultiBitInformation::gate_word_to_index,
                                             R"(
Maps a (gate, word) pair to an index.

:type: dict[(hal_py.Gate,(str, hal_py.PinDirection, str)), int]
)");

        py::enum_<machine_learning::NetlistFlavor>(m, "NetlistFlavor", R"(
Describes the netlist's originating flavor (vendor/synthesizer/backend/encoding style).
)")
            .value("Default", machine_learning::NetlistFlavor::Default, R"(
Default / default behavior.
)")
            .value("Yosys", machine_learning::NetlistFlavor::Yosys, R"(
Yosys-style netlist encoding.
)")
            .value("Vivado", machine_learning::NetlistFlavor::Vivado, R"(
Vivado-style netlist encoding.
)")
            .export_values();

        py::enum_<machine_learning::MultiBitProcessingPolicy>(m, "MultiBitProcessingPolicy", R"(
Specifies how multi-bit word candidates are deduplicated/selected.
)")
            .value("Default", machine_learning::MultiBitProcessingPolicy::Default, R"(
Lowest average distance wins (current behavior).
)")
            .value("Yosys", machine_learning::MultiBitProcessingPolicy::Yosys, R"(
Prefer output-pin word if ambiguous; otherwise like Default.
)")
            .value("Vivado", machine_learning::MultiBitProcessingPolicy::Vivado, R"(
Prefer gate-name encoding (PinDirection::none, distance 0) if ambiguous; otherwise like Default.
)")
            .export_values();

        py::class_<machine_learning::Context> py_context(m, "Context", R"()");

        py_context.def(py::init<const Netlist*, const machine_learning::NetlistFlavor, const u32>(),
                       py::arg("netlist"),
                       py::arg("flavor")       = machine_learning::NetlistFlavor::Default,
                       py::arg("_num_threads") = 1,
                       R"(
Construct a Context object with the given netlist, netlist flavor, and number of threads.

:param hal_py.Netlist netlist: The netlist.
:param machine_learning.NetlistFlavor flavor: Netlist flavor (affects multi-bit processing policy).
:param int _num_threads: The number of threads. Defaults to 1.
)");

        // get_sequential_abstraction returns a Result<NetlistAbstraction*>
        py_context.def(
            "get_sequential_abstraction",
            [](machine_learning::Context& self) -> std::optional<NetlistAbstraction*> {
                auto res = self.get_sequential_abstraction();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting sequential abstraction:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
Get the sequential netlist abstraction.

:returns: The sequential netlist abstraction on success, None otherwise.
:rtype: hal_py.NetlistAbstraction or None
)");

        // get_original_abstraction returns a Result<NetlistAbstraction*>
        py_context.def(
            "get_original_abstraction",
            [](machine_learning::Context& self) -> std::optional<NetlistAbstraction*> {
                auto res = self.get_original_abstraction();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting original abstraction:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
Get the original netlist abstraction.

:returns: The original netlist abstraction on success, None otherwise.
:rtype: hal_py.NetlistAbstraction or None
)");

        // get_sequential_netlist_graph returns a Result<graph_algorithm::NetlistGraph*>
        py_context.def(
            "get_sequential_netlist_graph",
            [](machine_learning::Context& self) -> std::optional<graph_algorithm::NetlistGraph*> {
                auto res = self.get_sequential_netlist_graph();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting sequential netlist graph:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
Get the sequential netlist graph.

:returns: The sequential netlist graph on success, None otherwise.
:rtype: hal_py.NetlistGraph or None
)");

        // get_original_netlist_graph returns a Result<graph_algorithm::NetlistGraph*>
        py_context.def(
            "get_original_netlist_graph",
            [](machine_learning::Context& self) -> std::optional<graph_algorithm::NetlistGraph*> {
                auto res = self.get_original_netlist_graph();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting original netlist graph:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
Get the original netlist graph.

:returns: The original netlist graph on success, None otherwise.
:rtype: hal_py.NetlistGraph or None
)");

        // get_possible_gate_type_properties returns const std::vector<GateTypeProperty>&
        py_context.def("get_possible_gate_type_properties",
                       &machine_learning::Context::get_possible_gate_type_properties,
                       R"(
Get the possible gate type properties.

:returns: A list of possible gate type properties.
:rtype: list[hal_py.GateTypeProperty]
)");

        // get_multi_bit_information returns const MultiBitInformation&
        py_context.def("get_multi_bit_information",
                       &machine_learning::Context::get_multi_bit_information,
                       py::return_value_policy::reference_internal,
                       R"(
Get the multi-bit information.

:returns: The multi-bit information object.
:rtype: machine_learning.MultiBitInformation
)");

        // get_gates returns const std::vector<Gate*>&
        py_context.def("get_gates",
                       &machine_learning::Context::get_gates,
                       R"(
Get the gates of the context.

:returns: A list of gates.
:rtype: list[hal_py.Gate]
)");

        py_context.def("get_netlist_flavor",
                       &machine_learning::Context::get_netlist_flavor,
                       R"(
Get the netlist flavor configured for this context.

:returns: The netlist flavor.
:rtype: machine_learning.NetlistFlavor
)");

        py_context.def("get_multi_bit_processing_policy",
                       &machine_learning::Context::get_multi_bit_processing_policy,
                       R"(
Get the multi-bit processing policy derived from the configured netlist flavor.

:returns: The policy used for multi-bit processing.
:rtype: machine_learning.MultiBitProcessingPolicy
)");

        // Bindings for NetlistGraph
        py::class_<machine_learning::NetlistGraph> py_netlist_graph(m, "NetlistGraph", R"(
            Represents a graph of the netlist.
        )");

        py_netlist_graph.def_readwrite("edge_list", &machine_learning::NetlistGraph::edge_list, R"(
            Edge list of the graph as a tuple of source and target node indices.

            :type: tuple[list[int], list[int]]
        )");

        py_netlist_graph.def_readwrite("edge_features", &machine_learning::NetlistGraph::edge_features, R"(
            List of features corresponding to the edge list

            :type: list[list[FEATURE_TYPE]]
        )");

        py_netlist_graph.def_readwrite("direction", &machine_learning::NetlistGraph::direction, R"(
            Direction of the graph.

            :type: hal_py.machine_learning.GraphDirection
        )");

        // Bindings for construct_netlist_graph
        m.def(
            "construct_netlist_graph",
            [](machine_learning::Context& ctx,
               const machine_learning::GraphDirection& dir,
               const std::vector<const machine_learning::edge_feature::EdgeFeature*>& edge_features) -> std::optional<machine_learning::NetlistGraph> {
                auto res = machine_learning::construct_netlist_graph(ctx, dir, edge_features);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while constructing sequential netlist graph:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("direction"),
            py::arg("edge_features"),
            R"(
            Constructs a netlist graph from the given netlist and gates.

            :param hal_py.Netlist netlist: The netlist.
            :param list[hal_py.Gate] gates: The gates to include in the graph.
            :param hal_py.machine_learning.GraphDirection direction: The direction of the graph.
            :returns: A NetlistGraph object.
            :rtype: hal_py.machine_learning.graph.NetlistGraph
        )");

        m.def(
            "construct_sequential_netlist_graph",
            [](machine_learning::Context& ctx,
               const machine_learning::GraphDirection& dir,
               const std::vector<const machine_learning::edge_feature::EdgeFeature*>& edge_features) -> std::optional<machine_learning::NetlistGraph> {
                auto res = machine_learning::construct_sequential_netlist_graph(ctx, dir, edge_features);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while constructing sequential netlist graph:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("direction"),
            py::arg("edge_features"),
            R"(
                Constructs a sequential netlist graph representation. The connections are an edge list of indices representing the position of the gates in the gates vector.

                This function constructs a sequential graph representation of the netlist, capturing only the sequential dependencies between gates.

                :param hal_py.Netlist nl: The netlist to operate on.
                :param list[hal_py.Gate] gates: The order of the gates, needed for the index representation.
                :param machine_learning.GraphDirection dir: The direction of the graph.
                :returns: A NetlistGraph representing the sequential connections within the netlist on success, None otherwise.
                :rtype: machine_learning.NetlistGraph or None
            )");

        // Bindings for annotate_netlist_graph
        m.def("annotate_netlist_graph",
              &machine_learning::annotate_netlist_graph,
              py::arg("netlist"),
              py::arg("gates"),
              py::arg("netlist_graph"),
              py::arg("node_features"),
              R"(
            Annotates the netlist graph with the given node features.

            :param hal_py.Netlist netlist: The netlist.
            :param list[hal_py.Gate] gates: The gates included in the graph.
            :param hal_py.machine_learning.graph.NetlistGraph netlist_graph: The netlist graph.
            :param list[list[FEATURE_TYPE]] node_features: The features for each node.
        )");
            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
