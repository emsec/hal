#include "hal_core/python_bindings/python_bindings.h"
#include "machine_learning/features/edge_feature.h"
#include "machine_learning/features/gate_feature.h"
#include "machine_learning/features/gate_feature_bulk.h"
#include "machine_learning/features/gate_feature_single.h"
#include "machine_learning/features/gate_pair_feature.h"
#include "machine_learning/graph_neural_network.h"
#include "machine_learning/labels/gate_label.h"
#include "machine_learning/labels/gate_pair_label.h"
#include "machine_learning/plugin_machine_learning.h"
#include "machine_learning/types.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(machine_learning, m)
    {
        m.doc() = "Machine learning plugin for HAL.";
#else
    PYBIND11_PLUGIN(machine_learning)
    {
        py::module m("machine_learning", "Machine learning plugin for HAL.");
#endif    // ifdef PYBIND11_MODULE

        // Define submodules for namespaces
        py::module py_edge_feature      = m.def_submodule("edge_feature");
        py::module py_gate_feature      = m.def_submodule("gate_feature");
        py::module py_gate_pair_feature = m.def_submodule("gate_pair_feature");
        py::module py_gate_pair_label   = m.def_submodule("gate_pair_label");
        py::module py_gate_label        = m.def_submodule("gate_label");

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

        py::class_<machine_learning::Context> py_context(m, "Context", R"()");

        py_context.def(py::init<const Netlist*, const u32>(),
                       py::arg("netlist"),
                       py::arg("_num_threads") = 1,
                       R"(
Construct a Context object with the given netlist and number of threads.

:param hal_py.Netlist netlist: The netlist.
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

        // machine_learning::features::gate_feature

        // GateFeature class
        py::class_<machine_learning::gate_feature::GateFeature> py_gate_feature_class(m, "GateFeature", R"(
            Base class for gate features.
        )");

        py_gate_feature_class.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::GateFeature& self, machine_learning::Context& ctx, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = self.calculate_feature(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("feature_context"),
            py::arg("gates"),
            R"(
                Calculate the feature for the given gates in the given feature context.

                :param hal_py.machine_learning.gate_feature.Context feature_context: The feature context.
                :param list[hal_py.Gate] gates: The gates.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[list[FEATURE_TYPE]] or None
            )");

        py_gate_feature_class.def("to_string",
                                  &machine_learning::gate_feature::GateFeature::to_string,
                                  R"(
                Get the string representation of the gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // ConnectedGlobalIOs class
        py::class_<machine_learning::gate_feature::ConnectedGlobalIOs, machine_learning::gate_feature::GateFeature> py_connected_global_ios(py_gate_feature, "ConnectedGlobalIOs", R"(
            Gate feature representing connected global IOs.
        )");

        py_connected_global_ios.def(py::init<>(), R"(
            Construct a ConnectedGlobalIOs gate feature.
        )");

        py_connected_global_ios.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::ConnectedGlobalIOs& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("feature_context"),
            py::arg("gate"),
            R"(
                Calculate the feature for the given gate in the given feature context.

                :param hal_py.machine_learning.gate_feature.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_connected_global_ios.def("to_string",
                                    &machine_learning::gate_feature::ConnectedGlobalIOs::to_string,
                                    R"(
                Get the string representation of the ConnectedGlobalIOs gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // DistanceGlobalIO class
        py::class_<machine_learning::gate_feature::DistanceGlobalIO, machine_learning::gate_feature::GateFeature> py_distance_global_io(py_gate_feature, "DistanceGlobalIO", R"(
            Gate feature representing distance to global IOs.
        )");

        py_distance_global_io.def(py::init<const PinDirection&, const bool, const std::vector<PinType>&>(),
                                  py::arg("direction"),
                                  py::arg("directed")            = true,
                                  py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                  R"(
            Construct a DistanceGlobalIO gate feature.

            :param hal_py.PinDirection direction: The pin direction.
            :param bool directed: Whether to consider direction. Defaults to True.
            :param list[hal_py.PinType] forbidden_pin_types: The forbidden pin types. Defaults to an empty list.
        )");

        py_distance_global_io.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::DistanceGlobalIO& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("feature_context"),
            py::arg("gate"),
            R"(
                Calculate the feature for the given gate in the given feature context.

                :param hal_py.machine_learning.gate_feature.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_distance_global_io.def("to_string",
                                  &machine_learning::gate_feature::DistanceGlobalIO::to_string,
                                  R"(
                Get the string representation of the DistanceGlobalIO gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // SequentialDistanceGlobalIO class
        py::class_<machine_learning::gate_feature::SequentialDistanceGlobalIO, machine_learning::gate_feature::GateFeature> py_sequential_distance_global_io(
            py_gate_feature, "SequentialDistanceGlobalIO", R"(
            Gate feature representing sequential distance to global IOs.
        )");

        py_sequential_distance_global_io.def(py::init<const PinDirection&, const bool, const std::vector<PinType>&>(),
                                             py::arg("direction"),
                                             py::arg("directed")            = true,
                                             py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                             R"(
            Construct a SequentialDistanceGlobalIO gate feature.

            :param hal_py.PinDirection direction: The pin direction.
            :param bool directed: Whether to consider direction. Defaults to True.
            :param list[hal_py.PinType] forbidden_pin_types: The forbidden pin types. Defaults to an empty list.
        )");

        py_sequential_distance_global_io.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::SequentialDistanceGlobalIO& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("feature_context"),
            py::arg("gate"),
            R"(
                Calculate the feature for the given gate in the given feature context.

                :param hal_py.machine_learning.gate_feature.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_sequential_distance_global_io.def("to_string",
                                             &machine_learning::gate_feature::SequentialDistanceGlobalIO::to_string,
                                             R"(
                Get the string representation of the SequentialDistanceGlobalIO gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // IODegrees class
        py::class_<machine_learning::gate_feature::IODegrees, machine_learning::gate_feature::GateFeature> py_io_degrees(py_gate_feature, "IODegrees", R"(
            Gate feature representing input/output degrees.
        )");

        py_io_degrees.def(py::init<>(), R"(
            Construct an IODegrees gate feature.
        )");

        py_io_degrees.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::IODegrees& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("feature_context"),
            py::arg("gate"),
            R"(
                Calculate the feature for the given gate in the given feature context.

                :param hal_py.machine_learning.gate_feature.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_io_degrees.def("to_string",
                          &machine_learning::gate_feature::IODegrees::to_string,
                          R"(
                Get the string representation of the IODegrees gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // GateTypeOneHot class
        py::class_<machine_learning::gate_feature::GateTypeOneHot, machine_learning::gate_feature::GateFeature> py_gate_type_one_hot(py_gate_feature, "GateTypeOneHot", R"(
            Gate feature representing gate type in one-hot encoding.
        )");

        py_gate_type_one_hot.def(py::init<>(), R"(
            Construct a GateTypeOneHot gate feature.
        )");

        py_gate_type_one_hot.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::GateTypeOneHot& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("feature_context"),
            py::arg("gate"),
            R"(
                Calculate the feature for the given gate in the given feature context.

                :param hal_py.machine_learning.gate_feature.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_gate_type_one_hot.def("to_string",
                                 &machine_learning::gate_feature::GateTypeOneHot::to_string,
                                 R"(
                Get the string representation of the GateTypeOneHot gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // NeighboringGateTypes class
        py::class_<machine_learning::gate_feature::NeighboringGateTypes, machine_learning::gate_feature::GateFeature> py_neighboring_gate_types(py_gate_feature, "NeighboringGateTypes", R"(
            Gate feature representing neighboring gate types.
        )");

        py_neighboring_gate_types.def(py::init<const u32, const PinDirection&, const bool>(),
                                      py::arg("depth"),
                                      py::arg("direction"),
                                      py::arg("directed") = true,
                                      R"(
            Construct a NeighboringGateTypes gate feature.

            :param int depth: The depth to consider.
            :param hal_py.PinDirection direction: The pin direction.
            :param bool directed: Whether to consider direction. Defaults to True.
        )");

        py_neighboring_gate_types.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::NeighboringGateTypes& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("feature_context"),
            py::arg("gate"),
            R"(
                Calculate the feature for the given gate in the given feature context.

                :param hal_py.machine_learning.gate_feature.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_neighboring_gate_types.def("to_string",
                                      &machine_learning::gate_feature::NeighboringGateTypes::to_string,
                                      R"(
                Get the string representation of the NeighboringGateTypes gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        py::class_<machine_learning::gate_feature::BetweennessCentrality, machine_learning::gate_feature::GateFeature> py_betweenness_centrality(py_gate_feature, "BetweennessCentrality", R"(
            A class representing the betweenness centrality feature for gates.
            )");

        py_betweenness_centrality.def(py::init<bool, i32>(),
                                      py::arg("directed") = true,
                                      py::arg("cutoff")   = -1,
                                      R"(
            Construct a BetweennessCentrality object.

            :param bool directed: Whether the graph is directed. Defaults to True.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            )");

        py_betweenness_centrality.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::BetweennessCentrality& self,
               machine_learning::Context& ctx,
               const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = self.calculate_feature(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating feature: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
            Calculate the betweenness centrality feature for the given gates.

            :param hal_py.Context ctx: The analysis context.
            :param list[hal_py.Gate] gates: The gates to analyze.
            :returns: The calculated features or None on failure.
            :rtype: list[list[float]] or None
            )");

        py_betweenness_centrality.def("to_string", &machine_learning::gate_feature::BetweennessCentrality::to_string, R"(
            Get a string representation of the BetweennessCentrality object.

            :returns: A string description.
            :rtype: str
            )");

        // HarmonicCentrality class
        py::class_<machine_learning::gate_feature::HarmonicCentrality, machine_learning::gate_feature::GateFeature> py_harmonic_centrality(py_gate_feature, "HarmonicCentrality", R"(
            A class representing the harmonic centrality feature for gates.
            )");

        py_harmonic_centrality.def(py::init<PinDirection, i32>(),
                                   py::arg("direction"),
                                   py::arg("cutoff") = -1,
                                   R"(
            Construct a HarmonicCentrality object.

            :param hal_py.PinDirection direction: The pin direction to consider.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            )");

        py_harmonic_centrality.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::HarmonicCentrality& self,
               machine_learning::Context& ctx,
               const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = self.calculate_feature(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating feature: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
            Calculate the harmonic centrality feature for the given gates.

            :param hal_py.Context ctx: The analysis context.
            :param list[hal_py.Gate] gates: The gates to analyze.
            :returns: The calculated features or None on failure.
            :rtype: list[list[float]] or None
            )");

        py_harmonic_centrality.def("to_string", &machine_learning::gate_feature::HarmonicCentrality::to_string, R"(
            Get a string representation of the HarmonicCentrality object.

            :returns: A string description.
            :rtype: str
            )");

        // SequentialBetweennessCentrality class
        py::class_<machine_learning::gate_feature::SequentialBetweennessCentrality, machine_learning::gate_feature::GateFeature> py_sequential_betweenness_centrality(
            py_gate_feature, "SequentialBetweennessCentrality", R"(
            A class representing the sequential betweenness centrality feature for gates.
            )");

        py_sequential_betweenness_centrality.def(py::init<bool, i32>(),
                                                 py::arg("directed") = true,
                                                 py::arg("cutoff")   = -1,
                                                 R"(
            Construct a SequentialBetweennessCentrality object.

            :param bool directed: Whether the graph is directed. Defaults to True.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            )");

        py_sequential_betweenness_centrality.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::SequentialBetweennessCentrality& self,
               machine_learning::Context& ctx,
               const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = self.calculate_feature(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating feature: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
            Calculate the sequential betweenness centrality feature for the given gates.

            :param hal_py.Context ctx: The analysis context.
            :param list[hal_py.Gate] gates: The gates to analyze.
            :returns: The calculated features or None on failure.
            :rtype: list[list[float]] or None
            )");

        py_sequential_betweenness_centrality.def("to_string", &machine_learning::gate_feature::SequentialBetweennessCentrality::to_string, R"(
            Get a string representation of the SequentialBetweennessCentrality object.

            :returns: A string description.
            :rtype: str
            )");

        // SequentialHarmonicCentrality class
        py::class_<machine_learning::gate_feature::SequentialHarmonicCentrality, machine_learning::gate_feature::GateFeature> py_sequential_harmonic_centrality(
            py_gate_feature, "SequentialHarmonicCentrality", R"(
            A class representing the sequential harmonic centrality feature for gates.
            )");

        py_sequential_harmonic_centrality.def(py::init<PinDirection, i32>(),
                                              py::arg("direction"),
                                              py::arg("cutoff") = -1,
                                              R"(
            Construct a SequentialHarmonicCentrality object.

            :param hal_py.PinDirection direction: The pin direction to consider.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            )");

        py_sequential_harmonic_centrality.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::SequentialHarmonicCentrality& self,
               machine_learning::Context& ctx,
               const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = self.calculate_feature(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating feature: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
            Calculate the sequential harmonic centrality feature for the given gates.

            :param hal_py.Context ctx: The analysis context.
            :param list[hal_py.Gate] gates: The gates to analyze.
            :returns: The calculated features or None on failure.
            :rtype: list[list[float]] or None
            )");

        py_sequential_harmonic_centrality.def("to_string", &machine_learning::gate_feature::SequentialHarmonicCentrality::to_string, R"(
            Get a string representation of the SequentialHarmonicCentrality object.

            :returns: A string description.
            :rtype: str
            )");

        // Define build_feature_vecs functions
        py_gate_feature.def(
            "build_feature_vecs",
            [](const std::vector<const machine_learning::gate_feature::GateFeature*>& features, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = machine_learning::gate_feature::build_feature_vecs(features, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to build feature vectors:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("features"),
            py::arg("gates"),
            R"(
                Build feature vectors for a list of gates from multiple features.

                :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of features.
                :param list[hal_py.Gate] gates: The list of gates for which to build the feature vectors.
                :returns: The feature vectors if successful, None otherwise.
                :rtype: list[list[int]] or None
            )");

        py_gate_feature.def(
            "build_feature_vecs",
            [](machine_learning::Context& ctx,
               const std::vector<const machine_learning::gate_feature::GateFeature*>& features,
               const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = machine_learning::gate_feature::build_feature_vecs(ctx, features, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to build feature vectors:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("features"),
            py::arg("gates"),
            R"(
                Build feature vectors for a list of gates from multiple features within a context.

                :param hal_py.machine_learning.gate_feature.Context ctx: The feature context.
                :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of features.
                :param list[hal_py.Gate] gates: The list of gates for which to build the feature vectors.
                :returns: The feature vectors if successful, None otherwise.
                :rtype: list[list[int]] or None
            )");

        // machine_learning::features::gate_pair_feature
        py::class_<machine_learning::gate_pair_feature::GatePairFeature, std::shared_ptr<machine_learning::gate_pair_feature::GatePairFeature>> py_gate_pair_feature_class(
            py_gate_pair_feature, "GatePairFeature", R"(
            Base class for gate pair features.
        )");

        py_gate_pair_feature_class.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::GatePairFeature& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error calculating feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the feature vector for the given gate pair.

                :param gate_pair_feature.Context ctx: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_gate_pair_feature_class.def("to_string", &machine_learning::gate_pair_feature::GatePairFeature::to_string, R"(
            Get the string representation of the feature.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_pair_feature::LogicalDistance, machine_learning::gate_pair_feature::GatePairFeature, std::shared_ptr<machine_learning::gate_pair_feature::LogicalDistance>>
            py_logical_distance(py_gate_pair_feature, "LogicalDistance", R"(
            Calculates the logical distance between two gates.
        )");

        py_logical_distance.def(py::init<const PinDirection, const bool, const std::vector<PinType>&>(),
                                py::arg("direction"),
                                py::arg("directed")            = true,
                                py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                R"(
                Construct a new LogicalDistance feature.

                :param hal_py.PinDirection direction: The direction of traversal.
                :param bool directed: Whether the graph is directed. Defaults to True.
                :param list[hal_py.PinType] forbidden_pin_types: Pin types to ignore. Defaults to empty list.
            )");

        py_logical_distance.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::LogicalDistance& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error calculating logical distance:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the logical distance between two gates.

                :param gate_pair_feature.Context ctx: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_logical_distance.def("to_string", &machine_learning::gate_pair_feature::LogicalDistance::to_string, R"(
            Get the string representation of the logical distance feature.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_pair_feature::SequentialDistance,
                   machine_learning::gate_pair_feature::GatePairFeature,
                   std::shared_ptr<machine_learning::gate_pair_feature::SequentialDistance>>
            py_sequential_distance(py_gate_pair_feature, "SequentialDistance", R"(
            Calculates the sequential distance between two gates.
        )");

        py_sequential_distance.def(py::init<const PinDirection, const bool, const std::vector<PinType>&>(),
                                   py::arg("direction"),
                                   py::arg("directed")            = true,
                                   py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                   R"(
                Construct a new SequentialDistance feature.

                :param hal_py.PinDirection direction: The direction of traversal.
                :param bool directed: Whether the graph is directed. Defaults to True.
                :param list[hal_py.PinType] forbidden_pin_types: Pin types to ignore. Defaults to empty list.
            )");

        py_sequential_distance.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::SequentialDistance& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error calculating sequential distance:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the sequential distance between two gates.

                :param gate_pair_feature.Context ctx: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_sequential_distance.def("to_string", &machine_learning::gate_pair_feature::SequentialDistance::to_string, R"(
            Get the string representation of the sequential distance feature.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_pair_feature::PhysicalDistance, machine_learning::gate_pair_feature::GatePairFeature, std::shared_ptr<machine_learning::gate_pair_feature::PhysicalDistance>>
            py_physical_distance(py_gate_pair_feature, "PhysicalDistance", R"(
            Calculates the physical distance between two gates.
        )");

        py_physical_distance.def(py::init<>(), R"(
            Construct a new PhysicalDistance feature.
        )");

        py_physical_distance.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::PhysicalDistance& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error calculating physical distance:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the physical distance between two gates.

                :param gate_pair_feature.Context ctx: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_physical_distance.def("to_string", &machine_learning::gate_pair_feature::PhysicalDistance::to_string, R"(
            Get the string representation of the physical distance feature.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_pair_feature::SharedControlSignals,
                   machine_learning::gate_pair_feature::GatePairFeature,
                   std::shared_ptr<machine_learning::gate_pair_feature::SharedControlSignals>>
            py_shared_control_signals(py_gate_pair_feature, "SharedControlSignals", R"(
            Calculates the shared control signals between two gates.
        )");

        py_shared_control_signals.def(py::init<>(), R"(
            Construct a new SharedControlSignals feature.
        )");

        py_shared_control_signals.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::SharedControlSignals& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error calculating shared control signals:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the shared control signals between two gates.

                :param gate_pair_feature.Context ctx: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_shared_control_signals.def("to_string", &machine_learning::gate_pair_feature::SharedControlSignals::to_string, R"(
            Get the string representation of the shared control signals feature.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_pair_feature::SharedSequentialNeighbors,
                   machine_learning::gate_pair_feature::GatePairFeature,
                   std::shared_ptr<machine_learning::gate_pair_feature::SharedSequentialNeighbors>>
            py_shared_sequential_neighbors(py_gate_pair_feature, "SharedSequentialNeighbors", R"(
            Calculates the number of shared sequential neighbors between two gates.
        )");

        py_shared_sequential_neighbors.def(py::init<const u32, const PinDirection, const bool, const std::vector<PinType>&, const std::vector<PinType>&>(),
                                           py::arg("depth"),
                                           py::arg("direction"),
                                           py::arg("directed")            = true,
                                           py::arg("starting_pin_types")  = std::vector<PinType>(),
                                           py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                           R"(
                Construct a new SharedSequentialNeighbors feature.

                :param int depth: The depth of traversal.
                :param hal_py.PinDirection direction: The direction of traversal.
                :param bool directed: Whether the graph is directed. Defaults to True.
                :param list[hal_py.PinType] starting_pin_types: Starting pin types. Defaults to empty list.
                :param list[hal_py.PinType] forbidden_pin_types: Forbidden pin types. Defaults to empty list.
            )");

        py_shared_sequential_neighbors.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::SharedSequentialNeighbors& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error calculating shared sequential neighbors:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the number of shared sequential neighbors between two gates.

                :param gate_pair_feature.Context ctx: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_shared_sequential_neighbors.def("to_string", &machine_learning::gate_pair_feature::SharedSequentialNeighbors::to_string, R"(
            Get the string representation of the shared sequential neighbors feature.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_pair_feature::SharedNeighbors, machine_learning::gate_pair_feature::GatePairFeature, std::shared_ptr<machine_learning::gate_pair_feature::SharedNeighbors>>
            py_shared_neighbors(py_gate_pair_feature, "SharedNeighbors", R"(
            Calculates the number of shared neighbors between two gates.
        )");

        py_shared_neighbors.def(py::init<const u32, const PinDirection, const bool, const std::vector<PinType>&, const std::vector<PinType>&>(),
                                py::arg("depth"),
                                py::arg("direction"),
                                py::arg("directed")            = true,
                                py::arg("starting_pin_types")  = std::vector<PinType>(),
                                py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                R"(
                Construct a new SharedNeighbors feature.

                :param int depth: The depth of traversal.
                :param hal_py.PinDirection direction: The direction of traversal.
                :param bool directed: Whether the graph is directed. Defaults to True.
                :param list[hal_py.PinType] starting_pin_types: Starting pin types. Defaults to empty list.
                :param list[hal_py.PinType] forbidden_pin_types: Forbidden pin types. Defaults to empty list.
            )");

        py_shared_neighbors.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::SharedNeighbors& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error calculating shared neighbors:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the number of shared neighbors between two gates.

                :param gate_pair_feature.Context ctx: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_shared_neighbors.def("to_string", &machine_learning::gate_pair_feature::SharedNeighbors::to_string, R"(
            Get the string representation of the shared neighbors feature.

            :returns: The string representation.
            :rtype: str
        )");

        // Binding for build_feature_vec functions
        py_gate_pair_feature.def(
            "build_feature_vec",
            [](const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vec(features, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while building feature vector:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("features"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Build the feature vector for a pair of gates.

            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param hal_py.Gate g_a: The first gate.
            :param hal_py.Gate g_b: The second gate.
            :returns: The feature vector.
            :rtype: list[FEATURE_TYPE] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vec",
            [](machine_learning::Context& ctx, const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vec(ctx, features, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while building feature vector with context:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("features"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Build the feature vector for a pair of gates with a given feature context.

            :param hal_py.machine_learning.gate_pair_feature.Context ctx: The feature context.
            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param hal_py.Gate g_a: The first gate.
            :param hal_py.Gate g_b: The second gate.
            :returns: The feature vector.
            :rtype: list[FEATURE_TYPE] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vec",
            [](const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features, const std::pair<Gate*, Gate*>& gate_pair) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vec(features, gate_pair);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while building feature vector:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("features"),
            py::arg("gate_pair"),
            R"(
            Build the feature vector for a pair of gates.

            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param tuple[hal_py.Gate, hal_py.Gate] gate_pair: The pair of gates.
            :returns: The feature vector.
            :rtype: list[FEATURE_TYPE] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vec",
            [](machine_learning::Context& ctx,
               const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features,
               const std::pair<Gate*, Gate*>& gate_pair) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vec(ctx, features, gate_pair);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while building feature vector with context:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("features"),
            py::arg("gate_pair"),
            R"(
            Build the feature vector for a pair of gates with a given feature context.

            :param hal_py.machine_learning.gate_pair_feature.Context ctx: The feature context.
            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param tuple[hal_py.Gate, hal_py.Gate] gate_pair: The pair of gates.
            :returns: The feature vector.
            :rtype: list[FEATURE_TYPE] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vecs",
            [](const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vecs(features, gate_pairs);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while building feature vectors:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("features"),
            py::arg("gate_pairs"),
            R"(
            Build the feature vectors for a list of gate pairs.

            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The list of gate pairs.
            :returns: The list of feature vectors.
            :rtype: list[list[int]] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vecs",
            [](machine_learning::Context& ctx,
               const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<FEATURE_TYPE>>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vecs(ctx, features, gate_pairs);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while building feature vectors with context:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("features"),
            py::arg("gate_pairs"),
            R"(
            Build the feature vectors for a list of gate pairs with a given feature context.

            :param hal_py.machine_learning.gate_pair_feature.Context ctx: The feature context.
            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The list of gate pairs.
            :returns: The list of feature vectors.
            :rtype: list[list[int]] or None
        )");

        // machine_learning::labels::gate_pair_label
        py::class_<machine_learning::gate_pair_label::GatePairLabel> py_gate_pair_label_class(py_gate_pair_label,
                                                                                              "GatePairLabel",
                                                                                              R"(
            Base class for calculating gate pairs and labels for machine learning models.

            This abstract class provides methods for calculating gate pairs and labels based on various criteria.
        )");

        py_gate_pair_label_class.def(
            "calculate_gate_pairs",
            [](const machine_learning::gate_pair_label::GatePairLabel& self, machine_learning::Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates)
                -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.calculate_gate_pairs(ctx, nl, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating gate pairs:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
            Calculate gate pairs based on the provided labeling context and netlist.

            :param machine_learning.Context ctx: The labeling context.
            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hal_py.Gate] gates: The gates to be paired.
            :returns: A list of gate pairs on success, None otherwise.
            :rtype: list[tuple[hal_py.Gate, hal_py.Gate]] or None
        )");

        py_gate_pair_label_class.def(
            "calculate_label",
            [](const machine_learning::gate_pair_label::GatePairLabel& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating label:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Calculate labels for a given gate pair.

            :param machine_learning.Context ctx: The labeling context.
            :param hal_py.Gate g_a: The first gate in the pair.
            :param hal_py.Gate g_b: The second gate in the pair.
            :returns: A list of labels on success, None otherwise.
            :rtype: list[int] or None
        )");

        py_gate_pair_label_class.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::GatePairLabel& self,
               machine_learning::Context& ctx,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, gate_pairs);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gate_pairs"),
            R"(
            Calculate labels for multiple gate pairs.

            :param machine_learning.Context ctx: The labeling context.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
            :returns: A list of label vectors for each pair on success, None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_pair_label_class.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::GatePairLabel& self,
               machine_learning::Context& ctx) -> std::optional<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate both gate pairs and their labels within the labeling context.

            :param hal_py.netlist: The netlist to create labels for.
            :returns: A pair containing gate pairs and corresponding labels on success, None otherwise.
            :rtype: tuple[list[tuple[hal_py.Gate, hal_py.Gate]], list[list[int]]] or None
        )");

        py_gate_pair_label_class.def("to_string",
                                     &machine_learning::gate_pair_label::GatePairLabel::to_string,
                                     R"(
                Get the string representation of the gate pair label.

                :returns: The string representation.
                :rtype: str
            )");

        py::class_<machine_learning::gate_pair_label::SharedSignalGroup, machine_learning::gate_pair_label::GatePairLabel> py_shared_signal_group(py_gate_pair_label,
                                                                                                                                                  "SharedSignalGroup",
                                                                                                                                                  R"(
            Labels gate pairs based on shared signal groups.
        )");

        py_shared_signal_group.def(py::init<const PinDirection&, const u32, const double>(),
                                   py::arg("direction"),
                                   py::arg("min_pair_count"),
                                   py::arg("negative_to_positive_factor"),
                                   R"(
                Construct a SharedSignalGroup.

                :param hal_py.PinDirection direction: The pin direction.
                :param int min_pair_count: The minimum pair count.
                :param float negative_to_positive_factor: The negative to positive factor.
            )");
        ;

        py_shared_signal_group.def(
            "calculate_gate_pairs",
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self, machine_learning::Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates)
                -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.calculate_gate_pairs(ctx, nl, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating gate pairs:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
            Calculate gate pairs based on shared signal groups.

            :param machine_learning.Context ctx: The labeling context.
            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hal_py.Gate] gates: The gates to be paired.
            :returns: A list of gate pairs on success, None otherwise.
            :rtype: list[tuple[hal_py.Gate, hal_py.Gate]] or None
        )");

        py_shared_signal_group.def(
            "calculate_label",
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating label:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Calculate labels for a given gate pair based on shared signal groups.

            :param machine_learning.Context ctx: The labeling context.
            :param hal_py.Gate g_a: The first gate in the pair.
            :param hal_py.Gate g_b: The second gate in the pair.
            :returns: A list of labels on success, None otherwise.
            :rtype: list[int] or None
        )");

        py_shared_signal_group.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self,
               machine_learning::Context& ctx,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, gate_pairs);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gate_pairs"),
            R"(
            Calculate labels for multiple gate pairs based on shared signal groups.

            :param machine_learning.Context ctx: The labeling context.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
            :returns: A list of label vectors for each pair on success, None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_shared_signal_group.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self,
               machine_learning::Context& ctx) -> std::optional<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate both gate pairs and their labels based on shared signal groups within the labeling context.

            :param hal_py.netlist: The netlist to create labels for.
            :returns: A pair containing gate pairs and corresponding labels on success, None otherwise.
            :rtype: tuple[list[tuple[hal_py.Gate, hal_py.Gate]], list[list[int]]] or None
        )");

        py_shared_signal_group.def("to_string",
                                   &machine_learning::gate_pair_label::SharedSignalGroup::to_string,
                                   R"(
                Get the string representation of the gate pair label.

                :returns: The string representation.
                :rtype: str
            )");

        py::class_<machine_learning::gate_pair_label::BitIndexOrdering, machine_learning::gate_pair_label::GatePairLabel> py_bit_index_ordering(py_gate_pair_label, "BitIndexOrdering", R"(
            Labels gate pairs based on their order in a shared control word.
            )");

        py_bit_index_ordering.def(py::init<const PinDirection&, u32, double>(),
                                  py::arg("direction"),
                                  py::arg("min_pair_count"),
                                  py::arg("negative_to_positive_factor"),
                                  R"(
            Default constructor.

            :param hal_py.PinDirection direction: The pin direction.
            :param int min_pair_count: The minimum pair count.
            :param float negative_to_positive_factor: The negative to positive factor.
            )");

        py_bit_index_ordering
            .def_readonly("LOWER", &machine_learning::gate_pair_label::BitIndexOrdering::LOWER, R"(
            The LOWER label vector.

            :type: list[int]
            )")
            .def_readonly("HIGHER", &machine_learning::gate_pair_label::BitIndexOrdering::HIGHER, R"(
            The HIGHER label vector.

            :type: list[int]
            )")
            .def_readonly("NA", &machine_learning::gate_pair_label::BitIndexOrdering::NA, R"(
            The NA label vector.

            :type: list[int]
            )");

        py_bit_index_ordering.def(
            "calculate_gate_pairs",
            [](machine_learning::gate_pair_label::BitIndexOrdering& self, machine_learning::Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates)
                -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.calculate_gate_pairs(ctx, nl, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating gate pairs:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
            Calculate gate pairs based on the provided labeling context and netlist.

            :param hal_py.Context ctx: The machine learning context.
            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hal_py.Gate] gates: The gates to be paired.
            :returns: A list of gate pairs on success, None otherwise.
            :rtype: list[tuple(hal_py.Gate, hal_py.Gate)] or None
            )");

        py_bit_index_ordering.def(
            "calculate_label",
            [](machine_learning::gate_pair_label::BitIndexOrdering& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating label:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Calculate labels for a given gate pair.

            :param hal_py.Context ctx: The machine learning context.
            :param hal_py.Gate g_a: The first gate in the pair.
            :param hal_py.Gate g_b: The second gate in the pair.
            :returns: A list of labels on success, None otherwise.
            :rtype: list[int] or None
            )");

        py_bit_index_ordering.def(
            "calculate_labels",
            [](machine_learning::gate_pair_label::BitIndexOrdering& self,
               machine_learning::Context& ctx,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, gate_pairs);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gate_pairs"),
            R"(
            Calculate labels for multiple gate pairs.

            :param hal_py.Context ctx: The machine learning context.
            :param list[tuple(hal_py.Gate, hal_py.Gate)] gate_pairs: The gate pairs to label.
            :returns: A list of label vectors for each pair on success, None otherwise.
            :rtype: list[list[int]] or None
            )");

        py_bit_index_ordering.def(
            "calculate_labels",
            [](machine_learning::gate_pair_label::BitIndexOrdering& self,
               machine_learning::Context& ctx) -> std::optional<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate both gate pairs and their labels within the labeling context.

            :param hal_py.Context ctx: The machine learning context.
            :returns: A tuple containing gate pairs and corresponding labels on success, None otherwise.
            :rtype: tuple(list[tuple(hal_py.Gate, hal_py.Gate)], list[list[int]]) or None
            )");

        py_bit_index_ordering.def("to_string",
                                  &machine_learning::gate_pair_label::BitIndexOrdering::to_string,
                                  R"(
            Get a string representation of the BitIndexOrdering.

            :returns: A string representation.
            :rtype: str
            )");

        py::class_<machine_learning::gate_pair_label::SharedConnection, machine_learning::gate_pair_label::GatePairLabel> py_shared_connection(py_gate_pair_label,
                                                                                                                                               "SharedConnection",
                                                                                                                                               R"(
            Labels gate pairs based on shared connections.
        )");

        py_shared_connection.def(py::init<>(),
                                 R"(
            Default constructor.
        )");

        py_shared_connection.def(
            "calculate_gate_pairs",
            [](const machine_learning::gate_pair_label::SharedConnection& self, machine_learning::Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates)
                -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.calculate_gate_pairs(ctx, nl, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating gate pairs:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
            Calculate gate pairs based on shared connections.

            :param machine_learning.Context ctx: The labeling context.
            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hal_py.Gate] gates: The gates to be paired.
            :returns: A list of gate pairs on success, None otherwise.
            :rtype: list[tuple[hal_py.Gate, hal_py.Gate]] or None
        )");

        py_shared_connection.def(
            "calculate_label",
            [](const machine_learning::gate_pair_label::SharedConnection& self, machine_learning::Context& ctx, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(ctx, g_a, g_b);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating label:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Calculate labels for a given gate pair based on shared connections.

            :param machine_learning.Context ctx: The labeling context.
            :param hal_py.Gate g_a: The first gate in the pair.
            :param hal_py.Gate g_b: The second gate in the pair.
            :returns: A list of labels on success, None otherwise.
            :rtype: list[int] or None
        )");

        py_shared_connection.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedConnection& self,
               machine_learning::Context& ctx,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, gate_pairs);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gate_pairs"),
            R"(
            Calculate labels for multiple gate pairs based on shared connections.

            :param machine_learning.Context ctx: The labeling context.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
            :returns: A list of label vectors for each pair on success, None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_shared_connection.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedConnection& self,
               machine_learning::Context& ctx) -> std::optional<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate both gate pairs and their labels based on shared connections within the labeling context.

            :param hal_py.netlist: The netlist to create labels for.
            :returns: A pair containing gate pairs and corresponding labels on success, None otherwise.
            :rtype: tuple[list[tuple[hal_py.Gate, hal_py.Gate]], list[list[int]]] or None
        )");

        py_shared_connection.def("to_string",
                                 &machine_learning::gate_pair_label::SharedConnection::to_string,
                                 R"(
                Get the string representation of the gate pair label.

                :returns: The string representation.
                :rtype: str
            )");

        py::class_<machine_learning::gate_label::GateLabel, std::shared_ptr<machine_learning::gate_label::GateLabel>> py_gate_label_class(py_gate_label, "GateLabel", R"(
            Base class for calculating labels for machine learning models.

            This abstract class provides methods for calculating labels based on various criteria.
        )");

        py_gate_label_class.def(
            "calculate_label",
            [](const machine_learning::gate_label::GateLabel& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating label: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g"),
            R"(
            Calculate labels for a given gate.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param hal_py.Gate g: The gate.
            :returns: A list of labels on success, or None otherwise.
            :rtype: list[int] or None
        )");

        py_gate_label_class.def(
            "calculate_labels",
            [](const machine_learning::gate_label::GateLabel& self, machine_learning::Context& ctx, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating labels: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
            Calculate labels for multiple gates.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param list[hal_py.Gate] gates: The gates to label.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_label_class.def(
            "calculate_labels",
            [](const machine_learning::gate_label::GateLabel& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating labels: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate labels within the labeling context.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_label_class.def("to_string",
                                &machine_learning::gate_label::GateLabel::to_string,
                                R"(
            Get a string representation of the gate label.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_label::GateNameKeyWord, machine_learning::gate_label::GateLabel, std::shared_ptr<machine_learning::gate_label::GateNameKeyWord>> py_gate_name_key_word(
            py_gate_label, "GateNameKeyWord", R"(
            Labels gates based on whether their name includes a keyword or not.
        )");

        py_gate_name_key_word.def(py::init<const std::string&, const std::vector<GateTypeProperty>&>(),
                                  py::arg("key_word"),
                                  py::arg("applicable_to") = std::vector<GateTypeProperty>(),
                                  R"(
            Construct a GateNameKeyWord labeler.

            :param str key_word: The keyword to check for in gate names.
            :param list[hal_py.machine_learning.GateTypeProperty] applicable_to: The gate type properties this label applies to. Defaults to an empty list.
        )");

        py_gate_name_key_word.def_readonly("MATCH", &machine_learning::gate_label::GateNameKeyWord::MATCH, R"(
            A label vector indicating a match.

            :type: list[int]
        )");

        py_gate_name_key_word.def_readonly("MISMATCH", &machine_learning::gate_label::GateNameKeyWord::MISMATCH, R"(
            A label vector indicating a mismatch.

            :type: list[int]
        )");

        py_gate_name_key_word.def_readonly("NA", &machine_learning::gate_label::GateNameKeyWord::NA, R"(
            A label vector indicating not applicable.

            :type: list[int]
        )");

        py_gate_name_key_word.def(
            "calculate_label",
            [](const machine_learning::gate_label::GateNameKeyWord& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating label: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g"),
            R"(
            Calculate labels for a given gate.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param hal_py.Gate g: The gate.
            :returns: A list of labels on success, or None otherwise.
            :rtype: list[int] or None
        )");

        py_gate_name_key_word.def(
            "calculate_labels",
            [](const machine_learning::gate_label::GateNameKeyWord& self, machine_learning::Context& ctx, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating labels: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
            Calculate labels for multiple gates.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param list[hal_py.Gate] gates: The gates to label.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_name_key_word.def(
            "calculate_labels",
            [](const machine_learning::gate_label::GateNameKeyWord& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating labels: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate labels within the labeling context.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_name_key_word.def("to_string",
                                  &machine_learning::gate_label::GateNameKeyWord::to_string,
                                  R"(
            Get a string representation of the gate labeler.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_label::NetNameKeyWord, machine_learning::gate_label::GateLabel, std::shared_ptr<machine_learning::gate_label::NetNameKeyWord>> py_net_name_key_word(
            py_gate_label, "NetNameKeyWord", R"(
Labels gate based on whether one of the net names at specified pins includes a keyword or not.
)");

        // Constructor:
        // NetNameKeyWord(const std::string& key_word, const std::vector<PinType>& pin_types, const std::vector<GateTypeProperty>& applicable_to = {})
        py_net_name_key_word.def(py::init<const std::string&, const std::vector<PinType>&, const std::vector<GateTypeProperty>&>(),
                                 py::arg("key_word"),
                                 py::arg("pin_types"),
                                 py::arg("applicable_to") = std::vector<GateTypeProperty>(),
                                 R"(
Construct a new NetNameKeyWord labeler.

:param str key_word: The keyword to search in the net names.
:param list[hal_py.PinType] pin_types: The pin types to check for the keyword.
:param list[hal_py.GateTypeProperty] applicable_to: The gate type properties to which this labeling applies. Defaults to an empty list.
)");

        py_net_name_key_word.def_readonly("MATCH", &machine_learning::gate_label::NetNameKeyWord::MATCH, R"(
            A label vector indicating a match.

            :type: list[int]
        )");

        py_net_name_key_word.def_readonly("MISMATCH", &machine_learning::gate_label::NetNameKeyWord::MISMATCH, R"(
            A label vector indicating a mismatch.

            :type: list[int]
        )");

        py_net_name_key_word.def_readonly("NA", &machine_learning::gate_label::NetNameKeyWord::NA, R"(
            A label vector indicating not applicable.

            :type: list[int]
        )");

        // NetNameKeyWord::calculate_label
        py_net_name_key_word.def(
            "calculate_label",
            [](machine_learning::gate_label::NetNameKeyWord& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<unsigned int>> {
                auto res = self.calculate_label(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating label:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g"),
            R"(
Calculate labels for a given gate based on net names at specified pins containing the keyword.

:param hal_py.Context ctx: The machine learning context.
:param hal_py.Gate g: The gate.
:returns: A list of labels on success, None otherwise.
:rtype: list[int] or None
)");

        // NetNameKeyWord::calculate_labels(Context&, const std::vector<Gate*>&)
        py_net_name_key_word.def(
            "calculate_labels",
            [](machine_learning::gate_label::NetNameKeyWord& self, machine_learning::Context& ctx, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<unsigned int>>> {
                auto res = self.calculate_labels(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels for gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
Calculate labels for multiple gates based on net names at specified pins containing the keyword.

:param hal_py.Context ctx: The machine learning context.
:param list[hal_py.Gate] gates: The gates to label.
:returns: A list of label vectors for each gate on success, None otherwise.
:rtype: list[list[int]] or None
)");

        // NetNameKeyWord::calculate_labels(Context&)
        py_net_name_key_word.def(
            "calculate_labels",
            [](machine_learning::gate_label::NetNameKeyWord& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<unsigned int>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels in context:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
Calculate labels within the labeling context based on net names at specified pins containing the keyword.

:param hal_py.Context ctx: The machine learning context.
:returns: A list of label vectors on success, None otherwise.
:rtype: list[list[int]] or None
)");

        // NetNameKeyWord::to_string()
        py_net_name_key_word.def("to_string",
                                 &machine_learning::gate_label::NetNameKeyWord::to_string,
                                 R"(
Convert the NetNameKeyWord labeler to a string.

:returns: The string representation.
:rtype: str
)");

        py::class_<machine_learning::gate_label::GateNameKeyWords, machine_learning::gate_label::GateLabel, std::shared_ptr<machine_learning::gate_label::GateNameKeyWords>> py_gate_name_key_words(
            py_gate_label, "GateNameKeyWords", R"(
            Labels gates based on whether their name includes a keyword of a selection or not.
        )");

        py_gate_name_key_words.def(py::init<const std::vector<std::string>&, const std::vector<GateTypeProperty>&, const bool>(),
                                   py::arg("key_words"),
                                   py::arg("applicable_to")  = std::vector<GateTypeProperty>(),
                                   py::arg("allow_multiple") = false,
                                   R"(
            Construct a GateNameKeyWords labeler.

            :param str key_word: The keyword to check for in gate names.
            :param list[hal_py.machine_learning.GateTypeProperty] applicable_to: The gate type properties this label applies to. Defaults to an empty list.
        )");

        // py_gate_name_key_words.def_readonly("MATCH", &machine_learning::gate_label::GateNameKeyWords::MATCH, R"(
        //     A label vector indicating a match.

        //     :type: list[int]
        // )");

        py_gate_name_key_words.def_readonly("NO_MATCH", &machine_learning::gate_label::GateNameKeyWords::NO_MATCH, R"(
            A label vector indicating no match was found.

            :type: list[int]
        )");

        py_gate_name_key_words.def_readonly("NA", &machine_learning::gate_label::GateNameKeyWords::NA, R"(
            A label vector indicating not applicable.

            :type: list[int]
        )");

        py_gate_name_key_words.def(
            "calculate_label",
            [](const machine_learning::gate_label::GateNameKeyWords& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating label: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g"),
            R"(
            Calculate labels for a given gate.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param hal_py.Gate g: The gate.
            :returns: A list of labels on success, or None otherwise.
            :rtype: list[int] or None
        )");

        py_gate_name_key_words.def(
            "calculate_labels",
            [](const machine_learning::gate_label::GateNameKeyWords& self, machine_learning::Context& ctx, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating labels: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
            Calculate labels for multiple gates.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param list[hal_py.Gate] gates: The gates to label.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_name_key_words.def(
            "calculate_labels",
            [](const machine_learning::gate_label::GateNameKeyWords& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating labels: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate labels within the labeling context.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_name_key_words.def("to_string",
                                   &machine_learning::gate_label::GateNameKeyWords::to_string,
                                   R"(
            Get a string representation of the gate labeler.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::gate_label::NetNameKeyWords, machine_learning::gate_label::GateLabel, std::shared_ptr<machine_learning::gate_label::NetNameKeyWords>> py_net_name_key_words(
            py_gate_label, "NetNameKeyWords", R"(
Labels gate based on whether one of the net names at specified pins includes a keyword or not.
)");

        // Constructor:
        // NetNameKeyWords(const std::string& key_word, const std::vector<PinType>& pin_types, const std::vector<GateTypeProperty>& applicable_to = {})
        py_net_name_key_words.def(py::init<const std::vector<std::string>&, const std::vector<PinType>&, const std::vector<GateTypeProperty>&, const bool>(),
                                  py::arg("key_word"),
                                  py::arg("pin_types"),
                                  py::arg("applicable_to")  = std::vector<GateTypeProperty>(),
                                  py::arg("allow_multiple") = false,
                                  R"(
Construct a new NetNameKeyWords labeler.

:param str key_word: The keyword to search in the net names.
:param list[hal_py.PinType] pin_types: The pin types to check for the keyword.
:param list[hal_py.GateTypeProperty] applicable_to: The gate type properties to which this labeling applies. Defaults to an empty list.
)");

        // py_net_name_key_words.def_readonly("MATCH", &machine_learning::gate_label::NetNameKeyWords::MATCH, R"(
        //     A label vector indicating a match.

        //     :type: list[int]
        // )");

        py_net_name_key_words.def_readonly("NO_MATCH", &machine_learning::gate_label::NetNameKeyWords::NO_MATCH, R"(
            A label vector indicating that no match was found.

            :type: list[int]
        )");

        py_net_name_key_words.def_readonly("NA", &machine_learning::gate_label::NetNameKeyWords::NA, R"(
            A label vector indicating not applicable.

            :type: list[int]
        )");

        // NetNameKeyWords::calculate_label
        py_net_name_key_words.def(
            "calculate_label",
            [](machine_learning::gate_label::NetNameKeyWords& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<unsigned int>> {
                auto res = self.calculate_label(ctx, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating label:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("g"),
            R"(
Calculate labels for a given gate based on net names at specified pins containing the keyword.

:param hal_py.Context ctx: The machine learning context.
:param hal_py.Gate g: The gate.
:returns: A list of labels on success, None otherwise.
:rtype: list[int] or None
)");

        // NetNameKeyWords::calculate_labels(Context&, const std::vector<Gate*>&)
        py_net_name_key_words.def(
            "calculate_labels",
            [](machine_learning::gate_label::NetNameKeyWords& self, machine_learning::Context& ctx, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<unsigned int>>> {
                auto res = self.calculate_labels(ctx, gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels for gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("gates"),
            R"(
Calculate labels for multiple gates based on net names at specified pins containing the keyword.

:param hal_py.Context ctx: The machine learning context.
:param list[hal_py.Gate] gates: The gates to label.
:returns: A list of label vectors for each gate on success, None otherwise.
:rtype: list[list[int]] or None
)");

        // NetNameKeyWords::calculate_labels(Context&)
        py_net_name_key_words.def(
            "calculate_labels",
            [](machine_learning::gate_label::NetNameKeyWords& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<unsigned int>>> {
                auto res = self.calculate_labels(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while calculating labels in context:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
Calculate labels within the labeling context based on net names at specified pins containing the keyword.

:param hal_py.Context ctx: The machine learning context.
:returns: A list of label vectors on success, None otherwise.
:rtype: list[list[int]] or None
)");

        // NetNameKeyWords::to_string()
        py_net_name_key_words.def("to_string",
                                  &machine_learning::gate_label::NetNameKeyWords::to_string,
                                  R"(
Convert the NetNameKeyWords labeler to a string.

:returns: The string representation.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::EdgeFeature, RawPtrWrapper<machine_learning::edge_feature::EdgeFeature>> py_edge_feature_class(py_edge_feature,
                                                                                                                                                  "EdgeFeature",
                                                                                                                                                  R"(
Abstract base class representing a feature for an edge between two endpoints.
)");

        py_edge_feature_class.def(
            "calculate_feature",
            [](machine_learning::edge_feature::EdgeFeature& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in EdgeFeature::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
Calculate the feature vector for the given edge.

:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_edge_feature_class.def("to_string",
                                  &machine_learning::edge_feature::EdgeFeature::to_string,
                                  R"(
Get a string representation of this feature.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::PinTypesOnehot, RawPtrWrapper<machine_learning::edge_feature::PinTypesOnehot>, machine_learning::edge_feature::EdgeFeature> py_pin_types_onehot(
            py_edge_feature,
            "PinTypesOnehot",
            R"(
One-hot encoding feature for pin types on an edge.
)");

        py_pin_types_onehot.def(py::init<>(), R"(
Construct a PinTypesOnehot feature.
)");

        py_pin_types_onehot.def(
            "calculate_feature",
            [](machine_learning::edge_feature::PinTypesOnehot& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in PinTypesOnehot::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
One-hot encoding feature vector for pin types.

:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_pin_types_onehot.def("to_string",
                                &machine_learning::edge_feature::PinTypesOnehot::to_string,
                                R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::PinDirectionOnehot, RawPtrWrapper<machine_learning::edge_feature::PinDirectionOnehot>, machine_learning::edge_feature::EdgeFeature>
            py_pin_direction_onehot(py_edge_feature,
                                    "PinDirectionOnehot",
                                    R"(
One-hot encoding feature for pin directions on an edge.
)");

        py_pin_direction_onehot.def(py::init<>(), R"(
Construct a PinDirectionOnehot feature.
)");

        py_pin_direction_onehot.def(
            "calculate_feature",
            [](machine_learning::edge_feature::PinDirectionOnehot& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination)
                -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in PinDirectionOnehot::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
One-hot encoding feature vector for pin directions.

:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_pin_direction_onehot.def("to_string",
                                    &machine_learning::edge_feature::PinDirectionOnehot::to_string,
                                    R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::PinIndices, RawPtrWrapper<machine_learning::edge_feature::PinIndices>, machine_learning::edge_feature::EdgeFeature> py_pin_indices(py_edge_feature,
                                                                                                                                                                                      "PinIndices",
                                                                                                                                                                                      R"(
)");

        py_pin_indices.def(py::init<>(), R"(
Construct a PinIndices feature.
)");

        py_pin_indices.def(
            "calculate_feature",
            [](machine_learning::edge_feature::PinIndices& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in PinIndices::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_pin_indices.def("to_string",
                           &machine_learning::edge_feature::PinIndices::to_string,
                           R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::GateTypeIndices, RawPtrWrapper<machine_learning::edge_feature::GateTypeIndices>, machine_learning::edge_feature::EdgeFeature> py_gate_type_indices(
            py_edge_feature,
            "GateTypeIndices",
            R"(
)");

        py_gate_type_indices.def(py::init<>(), R"(
Construct a GateTypeIndices feature.
)");

        py_gate_type_indices.def(
            "calculate_feature",
            [](machine_learning::edge_feature::GateTypeIndices& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in GateTypeIndices::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_gate_type_indices.def("to_string",
                                 &machine_learning::edge_feature::GateTypeIndices::to_string,
                                 R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py_edge_feature.def(
            "build_feature_vec",
            [](machine_learning::Context& ctx, const std::vector<const machine_learning::edge_feature::EdgeFeature*>& features, const Endpoint* source, const Endpoint* destination)
                -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = machine_learning::edge_feature::build_feature_vec(ctx, features, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in build_feature_vec:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("features"),
            py::arg("source"),
            py::arg("destination"),
            R"(
Build a combined feature vector from multiple features.

:param hal_py.Context ctx: The context.
:param list[hal_py.edge_feature.EdgeFeature] features: A list of edge features.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The combined feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
