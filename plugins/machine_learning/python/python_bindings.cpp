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
            .value("directed_forward", machine_learning::GraphDirection::directed_forward, R"(
            Directed forward traversal.

            :type: int
        )")
            .value("directed_backward", machine_learning::GraphDirection::directed_backward, R"(
            Directed backward traversal.

            :type: int
        )")
            .value("bidirectional", machine_learning::GraphDirection::bidirectional, R"(
            Bidirectional traversal.

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
        py::class_<hal::machine_learning::gate_feature::FeatureContext> py_gate_feature_context(py_gate_feature, "FeatureContext", R"(
                This class holds context information for feature extraction in a netlist analysis.
                Provides methods for obtaining specific feature-related data.
            )");

        py_gate_feature_context.def(py::init<const hal::Netlist*>(), py::arg("netlist"), R"(
            Initialize the FeatureContext with the given netlist.

            :param hal_py.Netlist netlist: The netlist to analyze.
        )");

        // py_gate_feature_context.def("get_sequential_abstraction", &hal::machine_learning::gate_feature::FeatureContext::get_sequential_abstraction, R"(
        //     Get the sequential abstraction of the netlist.

        //     :returns: The sequential abstraction of the netlist.
        //     :rtype: hal_py.NetlistAbstraction
        // )");

        // py_gate_feature_context.def("get_possible_gate_type_properties", &hal::machine_learning::gate_feature::FeatureContext::get_possible_gate_type_properties, R"(
        //     Get possible gate type properties for feature extraction.

        //     :returns: A list of possible gate type properties.
        //     :rtype: list[hal_py.GateTypeProperty]
        // )");

        py_gate_feature_context.def_readonly("nl", &hal::machine_learning::gate_feature::FeatureContext::nl, R"(
            The netlist associated with this context.
            :type: hal_py.Netlist
        )");

        py::class_<hal::machine_learning::gate_feature::GateFeature> py_gate_feature_class(py_gate_feature, "GateFeature", R"(
                Base class for gate feature extraction in machine learning analysis.
                Provides an interface for calculating features and obtaining feature names.
            )");

        py_gate_feature_class.def("calculate_feature", &hal::machine_learning::gate_feature::GateFeature::calculate_feature, py::arg("fc"), py::arg("g"), R"(
            Calculate the feature vector for a specific gate in the given feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_gate_feature_class.def("get_name", &hal::machine_learning::gate_feature::GateFeature::get_name, R"(
            Get the name of the feature.

            :returns: The feature name.
            :rtype: str
        )");

        py::class_<hal::machine_learning::gate_feature::ConnectedGlobalIOs, hal::machine_learning::gate_feature::GateFeature> py_connected_global_ios(py_gate_feature, "ConnectedGlobalIOs", R"(
                Feature class for extracting features based on globally connected IOs.
            )");

        py_connected_global_ios.def(py::init<>(), R"(
            Construct a ConnectedGlobalIOs feature extractor.
        )");

        py_connected_global_ios.def("calculate_feature", &hal::machine_learning::gate_feature::ConnectedGlobalIOs::calculate_feature, py::arg("fc"), py::arg("g"), R"(
            Calculate the ConnectedGlobalIOs feature for a specific gate in the given feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_connected_global_ios.def("get_name", &hal::machine_learning::gate_feature::ConnectedGlobalIOs::get_name, R"(
            Get the name of the ConnectedGlobalIOs feature.

            :returns: The feature name.
            :rtype: str
        )");

        py::class_<hal::machine_learning::gate_feature::DistanceGlobalIO, hal::machine_learning::gate_feature::GateFeature> py_distance_global_io(py_gate_feature, "DistanceGlobalIO", R"(
                Feature class for calculating distance to global IO based on pin direction.
            )");

        py_distance_global_io.def(py::init<const hal::PinDirection&>(), py::arg("direction"), R"(
            Construct a DistanceGlobalIO feature extractor with a specified pin direction.

            :param hal_py.PinDirection direction: The pin direction.
        )");

        py_distance_global_io.def("calculate_feature", &hal::machine_learning::gate_feature::DistanceGlobalIO::calculate_feature, py::arg("fc"), py::arg("g"), R"(
            Calculate the DistanceGlobalIO feature for a specific gate in the given feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_distance_global_io.def("get_name", &hal::machine_learning::gate_feature::DistanceGlobalIO::get_name, R"(
            Get the name of the DistanceGlobalIO feature.

            :returns: The feature name.
            :rtype: str
        )");

        py::class_<hal::machine_learning::gate_feature::SequentialDistanceGlobalIO, hal::machine_learning::gate_feature::GateFeature> py_sequential_distance_global_io(
            py_gate_feature, "SequentialDistanceGlobalIO", R"(
                Feature class for calculating distance to global IO based on pin direction.
            )");

        py_sequential_distance_global_io.def(py::init<const hal::PinDirection&>(), py::arg("direction"), R"(
            Construct a SequentialDistanceGlobalIO feature extractor with a specified pin direction.

            :param hal_py.PinDirection direction: The pin direction.
        )");

        py_sequential_distance_global_io.def("calculate_feature", &hal::machine_learning::gate_feature::SequentialDistanceGlobalIO::calculate_feature, py::arg("fc"), py::arg("g"), R"(
            Calculate the SequentialDistanceGlobalIO feature for a specific gate in the given feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_sequential_distance_global_io.def("get_name", &hal::machine_learning::gate_feature::SequentialDistanceGlobalIO::get_name, R"(
            Get the name of the SequentialDistanceGlobalIO feature.

            :returns: The feature name.
            :rtype: str
        )");

        py::class_<hal::machine_learning::gate_feature::IODegrees, hal::machine_learning::gate_feature::GateFeature> py_io_degrees(py_gate_feature, "IODegrees", R"(
                Feature class for calculating distance to global IO based on pin direction.
            )");

        py_io_degrees.def(py::init<>(), R"(
            Construct a IODegrees feature extractor with a specified pin direction.
        )");

        py_io_degrees.def("calculate_feature", &hal::machine_learning::gate_feature::IODegrees::calculate_feature, py::arg("fc"), py::arg("g"), R"(
            Calculate the IODegrees feature for a specific gate in the given feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_io_degrees.def("get_name", &hal::machine_learning::gate_feature::IODegrees::get_name, R"(
            Get the name of the IODegrees feature.

            :returns: The feature name.
            :rtype: str
        )");

        py::class_<hal::machine_learning::gate_feature::GateTypeOneHot, hal::machine_learning::gate_feature::GateFeature> py_gate_type_one_hot(py_gate_feature, "GateTypeOneHot", R"(
                Feature class for calculating distance to global IO based on pin direction.
            )");

        py_gate_type_one_hot.def(py::init<>(), R"(
            Construct a GateTypeOneHot feature extractor with a specified pin direction.
        )");

        py_gate_type_one_hot.def("calculate_feature", &hal::machine_learning::gate_feature::GateTypeOneHot::calculate_feature, py::arg("fc"), py::arg("g"), R"(
            Calculate the GateTypeOneHot feature for a specific gate in the given feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_gate_type_one_hot.def("get_name", &hal::machine_learning::gate_feature::GateTypeOneHot::get_name, R"(
            Get the name of the GateTypeOneHot feature.

            :returns: The feature name.
            :rtype: str
        )");

        py::class_<hal::machine_learning::gate_feature::NeighboringGateTypes, hal::machine_learning::gate_feature::GateFeature> py_neighboring_gate_types(py_gate_feature, "NeighboringGateTypes", R"(
                Feature class for calculating distance to global IO based on pin direction.
            )");

        py_neighboring_gate_types.def(py::init<const u32, const hal::PinDirection&>(), py::arg("depth"), py::arg("direction"), R"(
            Construct a NeighboringGateTypes feature extractor with a specified pin direction.

            :param hal_py.PinDirection direction: The pin direction.
        )");

        py_neighboring_gate_types.def("calculate_feature", &hal::machine_learning::gate_feature::NeighboringGateTypes::calculate_feature, py::arg("fc"), py::arg("g"), R"(
            Calculate the NeighboringGateTypes feature for a specific gate in the given feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_neighboring_gate_types.def("get_name", &hal::machine_learning::gate_feature::NeighboringGateTypes::get_name, R"(
            Get the name of the NeighboringGateTypes feature.

            :returns: The feature name.
            :rtype: str
        )");

        // Define Python bindings for build_feature_vec and build_feature_vecs functions
        py_gate_feature.def("build_feature_vec",
                            py::overload_cast<const std::vector<const hal::machine_learning::gate_feature::GateFeature*>&, const hal::Gate*>(&hal::machine_learning::gate_feature::build_feature_vec),
                            py::arg("features"),
                            py::arg("g"),
                            R"(
            Build the feature vector for a specific gate using a list of gate features.

            :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of gate features.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_gate_feature.def("build_feature_vec",
                            py::overload_cast<hal::machine_learning::gate_feature::FeatureContext&, const std::vector<const hal::machine_learning::gate_feature::GateFeature*>&, const hal::Gate*>(
                                &hal::machine_learning::gate_feature::build_feature_vec),
                            py::arg("fc"),
                            py::arg("features"),
                            py::arg("g"),
                            R"(
            Build the feature vector for a specific gate using a list of gate features and a feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of gate features.
            :param hal_py.Gate g: The gate to calculate features for.
            :returns: A vector of feature values.
            :rtype: list[int]
        )");

        py_gate_feature.def(
            "build_feature_vecs",
            py::overload_cast<const std::vector<const hal::machine_learning::gate_feature::GateFeature*>&, const std::vector<hal::Gate*>&>(&hal::machine_learning::gate_feature::build_feature_vecs),
            py::arg("features"),
            py::arg("gates"),
            R"(
            Build feature vectors for a list of gates using a list of gate features.

            :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of gate features.
            :param list[hal_py.Gate] gates: The list of gates to calculate features for.
            :returns: A list of feature vectors for each gate.
            :rtype: list[list[int]]
        )");

        py_gate_feature.def(
            "build_feature_vecs",
            py::overload_cast<hal::machine_learning::gate_feature::FeatureContext&, const std::vector<const hal::machine_learning::gate_feature::GateFeature*>&, const std::vector<hal::Gate*>&>(
                &hal::machine_learning::gate_feature::build_feature_vecs),
            py::arg("fc"),
            py::arg("features"),
            py::arg("gates"),
            R"(
            Build feature vectors for a list of gates using a list of gate features and a feature context.

            :param hal_py.machine_learning.gate_feature.FeatureContext fc: The feature context.
            :param list[hal_py.machine_learning.gate_feature.GateFeature] features: The list of gate features.
            :param list[hal_py.Gate] gates: The list of gates to calculate features for.
            :returns: A list of feature vectors for each gate.
            :rtype: list[list[int]]
        )");

        // machine_learning::features::gate_pair_feature
        // Define FeatureContext
        py::class_<hal::machine_learning::gate_pair_feature::FeatureContext> py_gate_pair_feature_context(py_gate_pair_feature, "FeatureContext", R"(
                    This class provides the feature context which includes information from a netlist to analyze gate pairs.
                )");

        py_gate_pair_feature_context.def(py::init<const Netlist*>(), py::arg("netlist"), R"(
                    Initialize the feature context with a given netlist.

                    :param hal_py.Netlist netlist: The netlist to analyze.
                )");

        // py_gate_pair_feature_context.def("get_sequential_abstraction", &hal::machine_learning::gate_pair_feature::FeatureContext::get_sequential_abstraction, R"(
        //             Retrieve the sequential abstraction of the netlist.

        //             :returns: The sequential abstraction of the netlist.
        //             :rtype: hal_py.NetlistAbstraction
        //         )");

        // Define GatePairFeature
        py::class_<hal::machine_learning::gate_pair_feature::GatePairFeature> py_gate_pair_feature_class(py_gate_pair_feature, "GatePairFeature", R"(
                    Base class for features that operate on pairs of gates.
                )");

        py_gate_pair_feature_class.def("calculate_feature", &hal::machine_learning::gate_pair_feature::GatePairFeature::calculate_feature, py::arg("fc"), py::arg("g_a"), py::arg("g_b"), R"(
                    Calculate feature vector for a pair of gates.

                    :param hal_py.FeatureContext fc: The feature context.
                    :param hal_py.Gate g_a: The first gate.
                    :param hal_py.Gate g_b: The second gate.
                    :returns: The feature vector for the gate pair.
                    :rtype: list[int]
                )");

        py_gate_pair_feature_class.def("get_name", &hal::machine_learning::gate_pair_feature::GatePairFeature::get_name, R"(
                    Get the name of the feature.

                    :returns: The name of the feature.
                    :rtype: str
                )");

        // Define LogicalDistance
        py::class_<hal::machine_learning::gate_pair_feature::LogicalDistance, hal::machine_learning::gate_pair_feature::GatePairFeature> py_logical_distance(
            py_gate_pair_feature, "LogicalDistance", R"(
                    Feature representing logical distance between gate pairs in a specific direction.
                )");

        py_logical_distance.def(py::init<const PinDirection>(), py::arg("direction"), R"(
                    Initialize the LogicalDistance feature with a specified pin direction.

                    :param hal_py.PinDirection direction: The direction of the pin.
                )");

        // Define SequentialDistance
        py::class_<hal::machine_learning::gate_pair_feature::SequentialDistance, hal::machine_learning::gate_pair_feature::GatePairFeature> py_sequential_distance(
            py_gate_pair_feature, "SequentialDistance", R"(
                    Feature representing sequential distance between gate pairs in a specific direction.
                )");

        py_sequential_distance.def(py::init<const PinDirection>(), py::arg("direction"), R"(
                    Initialize the SequentialDistance feature with a specified pin direction.

                    :param hal_py.PinDirection direction: The direction of the pin.
                )");

        // Define PhysicalDistance
        py::class_<hal::machine_learning::gate_pair_feature::PhysicalDistance, hal::machine_learning::gate_pair_feature::GatePairFeature> py_physical_distance(
            py_gate_pair_feature, "PhysicalDistance", R"(
                    Feature representing physical distance between gate pairs.
                )");

        py_physical_distance.def(py::init<>(), R"(
                    Initialize the PhysicalDistance feature.
                )");

        // Define SharedControlSignals
        py::class_<hal::machine_learning::gate_pair_feature::SharedControlSignals, hal::machine_learning::gate_pair_feature::GatePairFeature> py_shared_control_signals(
            py_gate_pair_feature, "SharedControlSignals", R"(
                    Feature indicating shared control signals between gate pairs.
                )");

        py_shared_control_signals.def(py::init<>(), R"(
                    Initialize the SharedControlSignals feature.
                )");

        // Define SharedSequentialNeighbors
        py::class_<hal::machine_learning::gate_pair_feature::SharedSequentialNeighbors, hal::machine_learning::gate_pair_feature::GatePairFeature> py_shared_sequential_neighbors(
            py_gate_pair_feature, "SharedSequentialNeighbors", R"(
                    Feature indicating shared sequential neighbors for gate pairs with a specified depth and direction.
                )");

        py_shared_sequential_neighbors.def(py::init<const u32, const PinDirection>(), py::arg("depth"), py::arg("direction"), R"(
                    Initialize the SharedSequentialNeighbors feature with a specified depth and direction.

                    :param int depth: The depth for the analysis.
                    :param hal_py.PinDirection direction: The direction of the pin.
                )");

        // Define SharedNeighbors
        py::class_<hal::machine_learning::gate_pair_feature::SharedNeighbors, hal::machine_learning::gate_pair_feature::GatePairFeature> py_shared_neighbors(
            py_gate_pair_feature, "SharedNeighbors", R"(
                    Feature indicating shared neighbors for gate pairs with a specified depth and direction.
                )");

        py_shared_neighbors.def(py::init<const u32, const PinDirection>(), py::arg("depth"), py::arg("direction"), R"(
                    Initialize the SharedNeighbors feature with a specified depth and direction.

                    :param int depth: The depth for the analysis.
                    :param hal_py.PinDirection direction: The direction of the pin.
                )");

        // Free functions in gate_pair_feature
        py_gate_pair_feature.def("build_feature_vec",
                                 py::overload_cast<const std::vector<const hal::machine_learning::gate_pair_feature::GatePairFeature*>&, const Gate*, const Gate*>(
                                     &hal::machine_learning::gate_pair_feature::build_feature_vec),
                                 py::arg("features"),
                                 py::arg("g_a"),
                                 py::arg("g_b"),
                                 R"(
                    Build a feature vector for a pair of gates using specified features.

                    :param list[hal_py.GatePairFeature] features: The features to calculate.
                    :param hal_py.Gate g_a: The first gate.
                    :param hal_py.Gate g_b: The second gate.
                    :returns: A feature vector.
                    :rtype: list[int]
                )");

        py_gate_pair_feature.def("build_feature_vec",
                                 py::overload_cast<hal::machine_learning::gate_pair_feature::FeatureContext&,
                                                   const std::vector<const hal::machine_learning::gate_pair_feature::GatePairFeature*>&,
                                                   const Gate*,
                                                   const Gate*>(&hal::machine_learning::gate_pair_feature::build_feature_vec),
                                 py::arg("fc"),
                                 py::arg("features"),
                                 py::arg("g_a"),
                                 py::arg("g_b"),
                                 R"(
                    Build a feature vector for a pair of gates using specified features and a feature context.

                    :param hal_py.FeatureContext fc: The feature context.
                    :param list[hal_py.GatePairFeature] features: The features to calculate.
                    :param hal_py.Gate g_a: The first gate.
                    :param hal_py.Gate g_b: The second gate.
                    :returns: A feature vector.
                    :rtype: list[int]
                )");

        py_gate_pair_feature.def("build_feature_vec",
                                 py::overload_cast<const std::vector<const hal::machine_learning::gate_pair_feature::GatePairFeature*>&, const std::pair<const Gate*, const Gate*>&>(
                                     &hal::machine_learning::gate_pair_feature::build_feature_vec),
                                 py::arg("features"),
                                 py::arg("gate_pair"),
                                 R"(
                    Build a feature vector for a pair of gates from a gate pair using specified features.

                    :param list[hal_py.GatePairFeature] features: The features to calculate.
                    :param tuple(hal_py.Gate, hal_py.Gate) gate_pair: The gate pair.
                    :returns: A feature vector.
                    :rtype: list[int]
                )");

        py_gate_pair_feature.def("build_feature_vec",
                                 py::overload_cast<hal::machine_learning::gate_pair_feature::FeatureContext&,
                                                   const std::vector<const hal::machine_learning::gate_pair_feature::GatePairFeature*>&,
                                                   const std::pair<const Gate*, const Gate*>&>(&hal::machine_learning::gate_pair_feature::build_feature_vec),
                                 py::arg("fc"),
                                 py::arg("features"),
                                 py::arg("gate_pair"),
                                 R"(
                    Build a feature vector for a pair of gates from a gate pair using specified features and a feature context.

                    :param hal_py.FeatureContext fc: The feature context.
                    :param list[hal_py.GatePairFeature] features: The features to calculate.
                    :param tuple(hal_py.Gate, hal_py.Gate) gate_pair: The gate pair.
                    :returns: A feature vector.
                    :rtype: list[int]
                )");

        py_gate_pair_feature.def("build_feature_vecs",
                                 py::overload_cast<const std::vector<const hal::machine_learning::gate_pair_feature::GatePairFeature*>&, const std::vector<std::pair<const Gate*, const Gate*>>&>(
                                     &hal::machine_learning::gate_pair_feature::build_feature_vecs),
                                 py::arg("features"),
                                 py::arg("gate_pairs"),
                                 R"(
                    Build feature vectors for multiple gate pairs using specified features.

                    :param list[hal_py.GatePairFeature] features: The features to calculate.
                    :param list[tuple(hal_py.Gate, hal_py.Gate)] gate_pairs: The gate pairs.
                    :returns: A list of feature vectors for each gate pair.
                    :rtype: list[list[int]]
                )");

        py_gate_pair_feature.def("build_feature_vecs",
                                 py::overload_cast<hal::machine_learning::gate_pair_feature::FeatureContext&,
                                                   const std::vector<const hal::machine_learning::gate_pair_feature::GatePairFeature*>&,
                                                   const std::vector<std::pair<const Gate*, const Gate*>>&>(&hal::machine_learning::gate_pair_feature::build_feature_vecs),
                                 py::arg("fc"),
                                 py::arg("features"),
                                 py::arg("gate_pairs"),
                                 R"(
                    Build feature vectors for multiple gate pairs using specified features and a feature context.

                    :param hal_py.FeatureContext fc: The feature context.
                    :param list[hal_py.GatePairFeature] features: The features to calculate.
                    :param list[tuple(hal_py.Gate, hal_py.Gate)] gate_pairs: The gate pairs.
                    :returns: A list of feature vectors for each gate pair.
                    :rtype: list[list[int]]
                )");

        // machine_learning::labels::gate_pair_label
        // MultiBitInformation
        py::class_<hal::machine_learning::gate_pair_label::MultiBitInformation> py_multi_bit_information(
            py_gate_pair_label, "MultiBitInformation", R"(
                Holds mappings between word labels and gates, and gates and word labels.

                This struct provides a bi-directional mapping between specific word pairs and their corresponding gates,
                as well as between gates and associated word pairs.
            )");

        py_multi_bit_information.def_readwrite("word_to_gates", &hal::machine_learning::gate_pair_label::MultiBitInformation::word_to_gates, R"(
            Maps word pairs to corresponding gates.

            :type: dict[tuple[str, str], list[hal_py.Gate]]
        )");

        py_multi_bit_information.def_readwrite("gate_to_words", &hal::machine_learning::gate_pair_label::MultiBitInformation::gate_to_words, R"(
            Maps gates to associated word pairs.

            :type: dict[hal_py.Gate, list[tuple[str, str]]]
        )");

        // LabelContext
        py::class_<hal::machine_learning::gate_pair_label::LabelContext> py_label_context(
            py_gate_pair_label, "LabelContext", R"(
                Provides context for gate-pair labeling within a netlist.

                This struct is initialized with a reference to the netlist and the gates involved in the labeling.
                It also provides access to multi-bit information for use in labeling calculations.
            )");

        py_label_context.def(
            py::init<const Netlist*, const std::vector<Gate*>&>(),
            py::arg("netlist"),
            py::arg("gates"),
            R"(
                Constructs a `LabelContext` with the specified netlist and gates.

                :param hal_py.Netlist netlist: The netlist to which the gates belong.
                :param list[hal_py.Gate] gates: The gates to be labeled.
            )");

        py_label_context.def(
            "get_multi_bit_information",
            &hal::machine_learning::gate_pair_label::LabelContext::get_multi_bit_information,
            R"(
                Retrieves the multi-bit information, initializing it if not already done.

                :returns: The MultiBitInformation object.
                :rtype: hal_py.machine_learning.gate_pair_label.MultiBitInformation
            )");

        py_label_context.def_readonly("nl", &hal::machine_learning::gate_pair_label::LabelContext::nl, R"(
            The netlist to which the gates belong.

            :type: hal_py.Netlist
        )");

        py_label_context.def_readonly("gates", &hal::machine_learning::gate_pair_label::LabelContext::gates, R"(
            The gates that are part of this labeling context.

            :type: list[hal_py.Gate]
        )");

        py_label_context.def_readwrite("mbi", &hal::machine_learning::gate_pair_label::LabelContext::mbi, R"(
            Optional storage for multi-bit information, initialized on demand.

            :type: Optional[hal_py.machine_learning.gate_pair_label.MultiBitInformation]
        )");

        // GatePairLabel
        py::class_<hal::machine_learning::gate_pair_label::GatePairLabel> py_gatepairlabel(
            py_gate_pair_label, "GatePairLabel", R"(
                Base class for calculating gate pairs and labels for machine learning models.

                This abstract class provides methods for calculating gate pairs and labels based on various criteria.
            )");

        py_gatepairlabel.def(
            "calculate_gate_pairs",
            &hal::machine_learning::gate_pair_label::GatePairLabel::calculate_gate_pairs,
            py::arg("lc"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
                Calculate gate pairs based on the provided labeling context and netlist.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param hal_py.Netlist nl: The netlist to operate on.
                :param list[hal_py.Gate] gates: The gates to be paired.
                :returns: A list of gate pairs.
                :rtype: list[tuple[hal_py.Gate, hal_py.Gate]]
            )");

        py_gatepairlabel.def(
            "calculate_label",
            &hal::machine_learning::gate_pair_label::GatePairLabel::calculate_label,
            py::arg("lc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate labels for a given gate pair.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param hal_py.Gate g_a: The first gate in the pair.
                :param hal_py.Gate g_b: The second gate in the pair.
                :returns: A list of labels.
                :rtype: list[int]
            )");

        py_gatepairlabel.def(
            "calculate_labels",
            py::overload_cast<hal::machine_learning::gate_pair_label::LabelContext&, const std::vector<std::pair<Gate*, Gate*>>&>(&hal::machine_learning::gate_pair_label::GatePairLabel::calculate_labels, py::const_),
            py::arg("lc"),
            py::arg("gate_pairs"),
            R"(
                Calculate labels for multiple gate pairs.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
                :returns: A list of label vectors for each pair.
                :rtype: list[list[int]]
            )");

        py_gatepairlabel.def(
            "calculate_labels",
            py::overload_cast<hal::machine_learning::gate_pair_label::LabelContext&>(&hal::machine_learning::gate_pair_label::GatePairLabel::calculate_labels, py::const_),
            py::arg("lc"),
            R"(
                Calculate both gate pairs and their labels within the labeling context.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :returns: A pair containing gate pairs and corresponding labels.
                :rtype: tuple[list[tuple[hal_py.Gate, hal_py.Gate]], list[list[int]]]
            )");

        // SharedSignalGroup
        py::class_<hal::machine_learning::gate_pair_label::SharedSignalGroup, hal::machine_learning::gate_pair_label::GatePairLabel> py_sharedsignalgroup(
            py_gate_pair_label, "SharedSignalGroup", R"(
                Labels gate pairs based on shared signal groups.
            )");

        py_sharedsignalgroup.def(py::init<>(), R"(
            Default constructor.
        )");

        py_sharedsignalgroup.def(
            "calculate_gate_pairs",
            &hal::machine_learning::gate_pair_label::SharedSignalGroup::calculate_gate_pairs,
            py::arg("lc"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
                Calculate gate pairs based on the provided labeling context and netlist.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param hal_py.Netlist nl: The netlist to operate on.
                :param list[hal_py.Gate] gates: The gates to be paired.
                :returns: A list of gate pairs.
                :rtype: list[tuple[hal_py.Gate, hal_py.Gate]]
            )");

        py_sharedsignalgroup.def(
            "calculate_label",
            &hal::machine_learning::gate_pair_label::SharedSignalGroup::calculate_label,
            py::arg("lc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate labels for a given gate pair.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param hal_py.Gate g_a: The first gate in the pair.
                :param hal_py.Gate g_b: The second gate in the pair.
                :returns: A vector of labels.
                :rtype: list[int]
            )");

        py_sharedsignalgroup.def(
            "calculate_labels",
            py::overload_cast<hal::machine_learning::gate_pair_label::LabelContext&, const std::vector<std::pair<Gate*, Gate*>>&>(&hal::machine_learning::gate_pair_label::SharedSignalGroup::calculate_labels, py::const_),
            py::arg("lc"),
            py::arg("gate_pairs"),
            R"(
                Calculate labels for multiple gate pairs.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
                :returns: A vector of label vectors for each pair.
                :rtype: list[list[int]]
            )");

        py_sharedsignalgroup.def(
            "calculate_labels",
            py::overload_cast<hal::machine_learning::gate_pair_label::LabelContext&>(&hal::machine_learning::gate_pair_label::SharedSignalGroup::calculate_labels, py::const_),
            py::arg("lc"),
            R"(
                Calculate both gate pairs and their labels within the labeling context.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :returns: A pair containing gate pairs and corresponding labels.
                :rtype: tuple[list[tuple[hal_py.Gate, hal_py.Gate]], list[list[int]]]
            )");

        // SharedConnection
        py::class_<hal::machine_learning::gate_pair_label::SharedConnection, hal::machine_learning::gate_pair_label::GatePairLabel> py_sharedconnection(
            py_gate_pair_label, "SharedConnection", R"(
                Labels gate pairs based on shared connections.
            )");

        py_sharedconnection.def(py::init<>(), R"(
            Default constructor.
        )");

        py_sharedconnection.def(
            "calculate_gate_pairs",
            &hal::machine_learning::gate_pair_label::SharedConnection::calculate_gate_pairs,
            py::arg("lc"),
            py::arg("nl"),
            py::arg("gates"),
            R"(
                Calculate gate pairs based on the provided labeling context and netlist.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param hal_py.Netlist nl: The netlist to operate on.
                :param list[hal_py.Gate] gates: The gates to be paired.
                :returns: A list of gate pairs.
                :rtype: list[tuple[hal_py.Gate, hal_py.Gate]]
            )");

        py_sharedconnection.def(
            "calculate_label",
            &hal::machine_learning::gate_pair_label::SharedConnection::calculate_label,
            py::arg("lc"),
            py::arg("g_a"),
            py::arg("g_b"),
            R"(
                Calculate labels for a given gate pair.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param hal_py.Gate g_a: The first gate in the pair.
                :param hal_py.Gate g_b: The second gate in the pair.
                :returns: A vector of labels.
                :rtype: list[int]
            )");

        py_sharedconnection.def(
            "calculate_labels",
            py::overload_cast<hal::machine_learning::gate_pair_label::LabelContext&, const std::vector<std::pair<Gate*, Gate*>>&>(&hal::machine_learning::gate_pair_label::SharedConnection::calculate_labels, py::const_),
            py::arg("lc"),
            py::arg("gate_pairs"),
            R"(
                Calculate labels for multiple gate pairs.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :param list[tuple[hal_py.Gate, hal_py.Gate]] gate_pairs: The gate pairs to label.
                :returns: A vector of label vectors for each pair.
                :rtype: list[list[int]]
            )");

        py_sharedconnection.def(
            "calculate_labels",
            py::overload_cast<hal::machine_learning::gate_pair_label::LabelContext&>(&hal::machine_learning::gate_pair_label::SharedConnection::calculate_labels, py::const_),
            py::arg("lc"),
            R"(
                Calculate both gate pairs and their labels within the labeling context.

                :param hal_py.machine_learning.gate_pair_label.LabelContext lc: The labeling context.
                :returns: A pair containing gate pairs and corresponding labels.
                :rtype: tuple[list[tuple[hal_py.Gate, hal_py.Gate]], list[list[int]]]
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
