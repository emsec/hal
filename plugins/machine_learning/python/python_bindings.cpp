#include "hal_core/python_bindings/python_bindings.h"
#include "machine_learning/features/gate_feature.h"
#include "machine_learning/features/gate_pair_feature.h"
#include "machine_learning/graph_neural_network.h"
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
        py::module py_gate_feature      = m.def_submodule("gate_feature");
        py::module py_gate_pair_feature = m.def_submodule("gate_pair_feature");
        py::module py_gate_pair_label   = m.def_submodule("gate_pair_label");

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

        // machine_learning::types
        py::enum_<machine_learning::GraphDirection> py_graph_direction(m, "GraphDirection", R"(
        Enumeration of graph traversal directions.
    )");

        py_graph_direction
            .value("directed", machine_learning::GraphDirection::directed, R"(
            Directed traversal.

            :type: int
        )")
            .value("undirected", machine_learning::GraphDirection::undirected, R"(
            Undirected traversal.

            :type: int
        )")
            .export_values();

        // Bindings for NetlistGraph
        py::class_<machine_learning::NetlistGraph> py_netlist_graph(m, "NetlistGraph", R"(
            Represents a graph of the netlist.
        )");

        py_netlist_graph.def_readwrite("edge_list", &machine_learning::NetlistGraph::edge_list, R"(
            Edge list of the graph as a tuple of source and target node indices.

            :type: tuple[list[int], list[int]]
        )");

        py_netlist_graph.def_readwrite("direction", &machine_learning::NetlistGraph::direction, R"(
            Direction of the graph.

            :type: hal_py.machine_learning.GraphDirection
        )");

        // Bindings for construct_netlist_graph
        m.def("construct_netlist_graph",
              &machine_learning::construct_netlist_graph,
              py::arg("netlist"),
              py::arg("gates"),
              py::arg("direction"),
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
            [](const Netlist* nl, const std::vector<Gate*>& gates, const machine_learning::GraphDirection& dir) -> std::optional<machine_learning::NetlistGraph> {
                auto res = machine_learning::construct_sequential_netlist_graph(nl, gates, dir);
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
            py::arg("nl"),
            py::arg("gates"),
            py::arg("dir"),
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
            :param list[list[int]] node_features: The features for each node.
        )");

        // machine_learning::features::gate_feature
        // FeatureContext class
        py::class_<machine_learning::gate_feature::FeatureContext> py_gate_feature_context(py_gate_feature, "FeatureContext", R"(
            This class holds the context for gate feature calculations.
        )");

        py_gate_feature_context.def(py::init<const Netlist*>(), py::arg("netlist"), R"(
            Construct a FeatureContext with the given netlist.

            :param hal_py.Netlist netlist: The netlist.
        )");

        py_gate_feature_context.def(
            "get_sequential_abstraction",
            [](machine_learning::gate_feature::FeatureContext& self) -> std::optional<NetlistAbstraction*> {
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
                Get the sequential abstraction of the netlist.

                :returns: The sequential NetlistAbstraction on success, None otherwise.
                :rtype: hal_py.NetlistAbstraction or None
            )");

        py_gate_feature_context.def(
            "get_original_abstraction",
            [](machine_learning::gate_feature::FeatureContext& self) -> std::optional<NetlistAbstraction*> {
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
                Get the original abstraction of the netlist.

                :returns: The original NetlistAbstraction on success, None otherwise.
                :rtype: hal_py.NetlistAbstraction or None
            )");

        py_gate_feature_context.def("get_possible_gate_type_properties",
                                    &machine_learning::gate_feature::FeatureContext::get_possible_gate_type_properties,
                                    R"(
                Get the possible gate type properties.

                :returns: A list of GateTypeProperties.
                :rtype: list[hal_py.GateTypeProperty]
            )");

        py_gate_feature_context.def_readonly("nl", &machine_learning::gate_feature::FeatureContext::nl, R"(
            The netlist.

            :type: hal_py.Netlist
        )");

        // GateFeature class
        py::class_<machine_learning::gate_feature::GateFeature> py_gate_feature_class(py_gate_feature, "GateFeature", R"(
            Base class for gate features.
        )");

        py_gate_feature_class.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::GateFeature& self, machine_learning::gate_feature::FeatureContext& fc, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g);
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

                :param hal_py.machine_learning.gate_feature.FeatureContext feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[int] or None
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
            [](const machine_learning::gate_feature::ConnectedGlobalIOs& self, machine_learning::gate_feature::FeatureContext& fc, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g);
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

                :param hal_py.machine_learning.gate_feature.FeatureContext feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[int] or None
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
            [](const machine_learning::gate_feature::DistanceGlobalIO& self, machine_learning::gate_feature::FeatureContext& fc, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g);
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

                :param hal_py.machine_learning.gate_feature.FeatureContext feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[int] or None
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
            [](const machine_learning::gate_feature::SequentialDistanceGlobalIO& self, machine_learning::gate_feature::FeatureContext& fc, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g);
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

                :param hal_py.machine_learning.gate_feature.FeatureContext feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[int] or None
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
            [](const machine_learning::gate_feature::IODegrees& self, machine_learning::gate_feature::FeatureContext& fc, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g);
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

                :param hal_py.machine_learning.gate_feature.FeatureContext feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[int] or None
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
            [](const machine_learning::gate_feature::GateTypeOneHot& self, machine_learning::gate_feature::FeatureContext& fc, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g);
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

                :param hal_py.machine_learning.gate_feature.FeatureContext feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[int] or None
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
            [](const machine_learning::gate_feature::NeighboringGateTypes& self, machine_learning::gate_feature::FeatureContext& fc, const Gate* g) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g);
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

                :param hal_py.machine_learning.gate_feature.FeatureContext feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of unsigned integers representing the feature on success, None otherwise.
                :rtype: list[int] or None
            )");

        py_neighboring_gate_types.def("to_string",
                                      &machine_learning::gate_feature::NeighboringGateTypes::to_string,
                                      R"(
                Get the string representation of the NeighboringGateTypes gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // Define build_feature_vec functions
        py_gate_feature.def(
            "build_feature_vec",
            [](const std::vector<const hal::machine_learning::gate_feature::GateFeature*>& features, const hal::Gate* g) -> std::optional<std::vector<u32>> {
                auto res = hal::machine_learning::gate_feature::build_feature_vec(features, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to build feature vector:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("features"),
            py::arg("g"),
            R"(
                Build a feature vector for a gate from multiple features.

                :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of features.
                :param hal_py.Gate g: The gate for which to build the feature vector.
                :returns: The feature vector if successful, None otherwise.
                :rtype: list[int] or None
            )");

        py_gate_feature.def(
            "build_feature_vec",
            [](hal::machine_learning::gate_feature::FeatureContext& fc,
               const std::vector<const hal::machine_learning::gate_feature::GateFeature*>& features,
               const hal::Gate* g) -> std::optional<std::vector<u32>> {
                auto res = hal::machine_learning::gate_feature::build_feature_vec(fc, features, g);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to build feature vector:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("fc"),
            py::arg("features"),
            py::arg("g"),
            R"(
                Build a feature vector for a gate from multiple features within a context.

                :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
                :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of features.
                :param hal_py.Gate g: The gate for which to build the feature vector.
                :returns: The feature vector if successful, None otherwise.
                :rtype: list[int] or None
            )");

        // Define build_feature_vecs functions
        py_gate_feature.def(
            "build_feature_vecs",
            [](const std::vector<const hal::machine_learning::gate_feature::GateFeature*>& features, const std::vector<hal::Gate*>& gates) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = hal::machine_learning::gate_feature::build_feature_vecs(features, gates);
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
            [](hal::machine_learning::gate_feature::FeatureContext& fc,
               const std::vector<const hal::machine_learning::gate_feature::GateFeature*>& features,
               const std::vector<hal::Gate*>& gates) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = hal::machine_learning::gate_feature::build_feature_vecs(fc, features, gates);
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
            py::arg("fc"),
            py::arg("features"),
            py::arg("gates"),
            R"(
                Build feature vectors for a list of gates from multiple features within a context.

                :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
                :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of features.
                :param list[hal_py.Gate] gates: The list of gates for which to build the feature vectors.
                :returns: The feature vectors if successful, None otherwise.
                :rtype: list[list[int]] or None
            )");

        // machine_learning::features::gate_pair_feature
        // Binding for struct FeatureContext
        py::class_<machine_learning::gate_pair_feature::FeatureContext> py_gate_pair_feature_context(py_gate_pair_feature, "FeatureContext", R"(
            This class provides the context for feature calculation, including the netlist and abstractions.
        )");

        py_gate_pair_feature_context.def(py::init<const Netlist*>(), py::arg("netlist"), R"(
            Construct a new FeatureContext with the given netlist.

            :param hal_py.Netlist netlist: The netlist.
        )");

        py_gate_pair_feature_context.def_readonly("nl", &machine_learning::gate_pair_feature::FeatureContext::nl, R"(
            The netlist associated with this context.

            :type: hal_py.Netlist
        )");

        py_gate_pair_feature_context.def(
            "get_original_abstraction",
            [](machine_learning::gate_pair_feature::FeatureContext& self) -> std::optional<NetlistAbstraction*> {
                auto res = self.get_original_abstraction();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error getting original abstraction:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
                Get the original netlist abstraction.

                :returns: The original netlist abstraction on success, ``None`` otherwise.
                :rtype: hal_py.NetlistAbstraction or None
            )");

        py_gate_pair_feature_context.def(
            "get_sequential_abstraction",
            [](machine_learning::gate_pair_feature::FeatureContext& self) -> std::optional<NetlistAbstraction*> {
                auto res = self.get_sequential_abstraction();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error getting sequential abstraction:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
                Get the sequential netlist abstraction.

                :returns: The sequential netlist abstraction on success, ``None`` otherwise.
                :rtype: hal_py.NetlistAbstraction or None
            )");

        py::class_<machine_learning::gate_pair_feature::GatePairFeature, std::shared_ptr<machine_learning::gate_pair_feature::GatePairFeature>> py_gate_pair_feature_class(
            py_gate_pair_feature, "GatePairFeature", R"(
            Base class for gate pair features.
        )");

        py_gate_pair_feature_class.def(
            "calculate_feature",
            [](machine_learning::gate_pair_feature::GatePairFeature& self, machine_learning::gate_pair_feature::FeatureContext& fc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g_a, g_b);
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
            py::arg("fc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the feature vector for the given gate pair.

                :param gate_pair_feature.FeatureContext fc: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[int] or None
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
            [](machine_learning::gate_pair_feature::LogicalDistance& self, machine_learning::gate_pair_feature::FeatureContext& fc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g_a, g_b);
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
            py::arg("fc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the logical distance between two gates.

                :param gate_pair_feature.FeatureContext fc: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[int] or None
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
            [](machine_learning::gate_pair_feature::SequentialDistance& self, machine_learning::gate_pair_feature::FeatureContext& fc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g_a, g_b);
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
            py::arg("fc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the sequential distance between two gates.

                :param gate_pair_feature.FeatureContext fc: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[int] or None
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
            [](machine_learning::gate_pair_feature::PhysicalDistance& self, machine_learning::gate_pair_feature::FeatureContext& fc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g_a, g_b);
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
            py::arg("fc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the physical distance between two gates.

                :param gate_pair_feature.FeatureContext fc: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[int] or None
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
            [](machine_learning::gate_pair_feature::SharedControlSignals& self, machine_learning::gate_pair_feature::FeatureContext& fc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g_a, g_b);
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
            py::arg("fc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the shared control signals between two gates.

                :param gate_pair_feature.FeatureContext fc: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[int] or None
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
            [](machine_learning::gate_pair_feature::SharedSequentialNeighbors& self, machine_learning::gate_pair_feature::FeatureContext& fc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g_a, g_b);
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
            py::arg("fc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the number of shared sequential neighbors between two gates.

                :param gate_pair_feature.FeatureContext fc: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[int] or None
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
            [](machine_learning::gate_pair_feature::SharedNeighbors& self, machine_learning::gate_pair_feature::FeatureContext& fc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_feature(fc, g_a, g_b);
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
            py::arg("fc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate the number of shared neighbors between two gates.

                :param gate_pair_feature.FeatureContext fc: The feature context.
                :param hal_py.Gate g_a: The first gate.
                :param hal_py.Gate g_b: The second gate.
                :returns: The feature vector on success, ``None`` otherwise.
                :rtype: list[int] or None
            )");

        py_shared_neighbors.def("to_string", &machine_learning::gate_pair_feature::SharedNeighbors::to_string, R"(
            Get the string representation of the shared neighbors feature.

            :returns: The string representation.
            :rtype: str
        )");

        // Binding for build_feature_vec functions
        py_gate_pair_feature.def(
            "build_feature_vec",
            [](const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<u32>> {
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
            :rtype: list[int] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vec",
            [](machine_learning::gate_pair_feature::FeatureContext& fc, const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vec(fc, features, g_a, g_b);
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
            py::arg("fc"),
            py::arg("features"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Build the feature vector for a pair of gates with a given feature context.

            :param hal_py.machine_learning.gate_pair_feature.FeatureContext fc: The feature context.
            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param hal_py.Gate g_a: The first gate.
            :param hal_py.Gate g_b: The second gate.
            :returns: The feature vector.
            :rtype: list[int] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vec",
            [](const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features, const std::pair<Gate*, Gate*>& gate_pair) -> std::optional<std::vector<u32>> {
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
            :rtype: list[int] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vec",
            [](machine_learning::gate_pair_feature::FeatureContext& fc,
               const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features,
               const std::pair<Gate*, Gate*>& gate_pair) -> std::optional<std::vector<u32>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vec(fc, features, gate_pair);
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
            py::arg("fc"),
            py::arg("features"),
            py::arg("gate_pair"),
            R"(
            Build the feature vector for a pair of gates with a given feature context.

            :param hal_py.machine_learning.gate_pair_feature.FeatureContext fc: The feature context.
            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param tuple[hal_py.Gate, hal_py.Gate] gate_pair: The pair of gates.
            :returns: The feature vector.
            :rtype: list[int] or None
        )");

        py_gate_pair_feature.def(
            "build_feature_vecs",
            [](const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
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
            [](machine_learning::gate_pair_feature::FeatureContext& fc,
               const std::vector<const machine_learning::gate_pair_feature::GatePairFeature*>& features,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = machine_learning::gate_pair_feature::build_feature_vecs(fc, features, gate_pairs);
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
            py::arg("fc"),
            py::arg("features"),
            py::arg("gate_pairs"),
            R"(
            Build the feature vectors for a list of gate pairs with a given feature context.

            :param hal_py.machine_learning.gate_pair_feature.FeatureContext fc: The feature context.
            :param list[hal_py.machine_learning.gate_pair_feature.GatePairFeature] features: The list of features.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The list of gate pairs.
            :returns: The list of feature vectors.
            :rtype: list[list[int]] or None
        )");

        // machine_learning::labels::gate_pair_label
        // MultiBitInformation
        py::class_<machine_learning::gate_pair_label::MultiBitInformation> py_multi_bit_information(py_gate_pair_label,
                                                                                                    "MultiBitInformation",
                                                                                                    R"(
            Holds mappings between word labels and gates, and gates and word labels.

            This struct provides a bi-directional mapping between specific word pairs and their corresponding gates,
            as well as between gates and associated word pairs.
        )");

        py_multi_bit_information.def_readwrite("word_to_gates",
                                               &machine_learning::gate_pair_label::MultiBitInformation::word_to_gates,
                                               R"(
            Maps word pairs to corresponding gates.

            :type: dict[tuple[str, hal_py.PinDirection, str], list[hal_py.Gate]]
        )");

        py_multi_bit_information.def_readwrite("gate_to_words",
                                               &machine_learning::gate_pair_label::MultiBitInformation::gate_to_words,
                                               R"(
            Maps gates to associated word pairs.

            :type: dict[hal_py.Gate, list[tuple[str, hal_py.PinDirection, str]]]
        )");

        py::class_<machine_learning::gate_pair_label::LabelContext> py_label_context(py_gate_pair_label,
                                                                                     "LabelContext",
                                                                                     R"(
            Provides context for gate-pair labeling within a netlist.

            This struct is initialized with a reference to the netlist and the gates involved in the labeling.
            It also provides access to multi-bit information for use in labeling calculations.
        )");

        py_label_context.def(py::init<const Netlist*, const std::vector<Gate*>&>(),
                             py::arg("netlist"),
                             py::arg("gates"),
                             R"(
            Constructs a LabelContext with the specified netlist and gates.

            :param hal_py.Netlist netlist: The netlist to which the gates belong.
            :param list[hal_py.Gate] gates: The gates to be labeled.
        )");

        py_label_context.def("get_multi_bit_information",
                             &machine_learning::gate_pair_label::LabelContext::get_multi_bit_information,
                             py::return_value_policy::reference_internal,
                             R"(
            Retrieves the multi-bit information, initializing it if not already done.

            :returns: The MultiBitInformation object.
            :rtype: machine_learning.gate_pair_label.MultiBitInformation
        )");

        py_label_context.def_readonly("nl",
                                      &machine_learning::gate_pair_label::LabelContext::nl,
                                      R"(
            The netlist to which the gates belong.

            :type: hal_py.Netlist
        )");

        py_label_context.def_readonly("gates",
                                      &machine_learning::gate_pair_label::LabelContext::gates,
                                      R"(
            The gates that are part of this labeling context.

            :type: list[hal_py.Gate]
        )");

        py_label_context.def_readwrite("mbi",
                                       &machine_learning::gate_pair_label::LabelContext::mbi,
                                       R"(
            Optional storage for multi-bit information, initialized on demand.

            :type: Optional[machine_learning.gate_pair_label.MultiBitInformation]
        )");

        py::class_<machine_learning::gate_pair_label::GatePairLabel> py_gate_pair_label_class(py_gate_pair_label,
                                                                                              "GatePairLabel",
                                                                                              R"(
            Base class for calculating gate pairs and labels for machine learning models.

            This abstract class provides methods for calculating gate pairs and labels based on various criteria.
        )");

        py_gate_pair_label_class.def(
            "calculate_gate_pairs",
            [](const machine_learning::gate_pair_label::GatePairLabel& self, machine_learning::gate_pair_label::LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates)
                -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.calculate_gate_pairs(lc, nl, gates);
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
            py::arg("lc"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
            Calculate gate pairs based on the provided labeling context and netlist.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hal_py.Gate] gates: The gates to be paired.
            :returns: A list of gate pairs on success, None otherwise.
            :rtype: list[tuple[hal_py.Gate, hal_py.Gate]] or None
        )");

        py_gate_pair_label_class.def(
            "calculate_label",
            [](const machine_learning::gate_pair_label::GatePairLabel& self, machine_learning::gate_pair_label::LabelContext& lc, const Gate* g_a, const Gate* g_b) -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(lc, g_a, g_b);
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
            py::arg("lc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Calculate labels for a given gate pair.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param hal_py.Gate g_a: The first gate in the pair.
            :param hal_py.Gate g_b: The second gate in the pair.
            :returns: A list of labels on success, None otherwise.
            :rtype: list[int] or None
        )");

        py_gate_pair_label_class.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::GatePairLabel& self,
               machine_learning::gate_pair_label::LabelContext& lc,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(lc, gate_pairs);
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
            py::arg("lc"),
            py::arg("gate_pairs"),
            R"(
            Calculate labels for multiple gate pairs.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
            :returns: A list of label vectors for each pair on success, None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_gate_pair_label_class.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::GatePairLabel& self, const Netlist* netlist) -> std::optional<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> {
                auto res = self.calculate_labels(netlist);
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
            py::arg("lc"),
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
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self, machine_learning::gate_pair_label::LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates)
                -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.calculate_gate_pairs(lc, nl, gates);
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
            py::arg("lc"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
            Calculate gate pairs based on shared signal groups.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hal_py.Gate] gates: The gates to be paired.
            :returns: A list of gate pairs on success, None otherwise.
            :rtype: list[tuple[hal_py.Gate, hal_py.Gate]] or None
        )");

        py_shared_signal_group.def(
            "calculate_label",
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self, machine_learning::gate_pair_label::LabelContext& lc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(lc, g_a, g_b);
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
            py::arg("lc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Calculate labels for a given gate pair based on shared signal groups.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param hal_py.Gate g_a: The first gate in the pair.
            :param hal_py.Gate g_b: The second gate in the pair.
            :returns: A list of labels on success, None otherwise.
            :rtype: list[int] or None
        )");

        py_shared_signal_group.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self,
               machine_learning::gate_pair_label::LabelContext& lc,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(lc, gate_pairs);
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
            py::arg("lc"),
            py::arg("gate_pairs"),
            R"(
            Calculate labels for multiple gate pairs based on shared signal groups.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
            :returns: A list of label vectors for each pair on success, None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_shared_signal_group.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedSignalGroup& self,
               const Netlist* netlist) -> std::optional<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> {
                auto res = self.calculate_labels(netlist);
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
            py::arg("lc"),
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
            [](const machine_learning::gate_pair_label::SharedConnection& self, machine_learning::gate_pair_label::LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates)
                -> std::optional<std::vector<std::pair<Gate*, Gate*>>> {
                auto res = self.calculate_gate_pairs(lc, nl, gates);
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
            py::arg("lc"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
            Calculate gate pairs based on shared connections.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hal_py.Gate] gates: The gates to be paired.
            :returns: A list of gate pairs on success, None otherwise.
            :rtype: list[tuple[hal_py.Gate, hal_py.Gate]] or None
        )");

        py_shared_connection.def(
            "calculate_label",
            [](const machine_learning::gate_pair_label::SharedConnection& self, machine_learning::gate_pair_label::LabelContext& lc, const Gate* g_a, const Gate* g_b)
                -> std::optional<std::vector<u32>> {
                auto res = self.calculate_label(lc, g_a, g_b);
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
            py::arg("lc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
            Calculate labels for a given gate pair based on shared connections.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param hal_py.Gate g_a: The first gate in the pair.
            :param hal_py.Gate g_b: The second gate in the pair.
            :returns: A list of labels on success, None otherwise.
            :rtype: list[int] or None
        )");

        py_shared_connection.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedConnection& self,
               machine_learning::gate_pair_label::LabelContext& lc,
               const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(lc, gate_pairs);
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
            py::arg("lc"),
            py::arg("gate_pairs"),
            R"(
            Calculate labels for multiple gate pairs based on shared connections.

            :param machine_learning.gate_pair_label.LabelContext lc: The labeling context.
            :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
            :returns: A list of label vectors for each pair on success, None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_shared_connection.def(
            "calculate_labels",
            [](const machine_learning::gate_pair_label::SharedConnection& self,
               const Netlist* netlist) -> std::optional<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> {
                auto res = self.calculate_labels(netlist);
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
            py::arg("lc"),
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

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
