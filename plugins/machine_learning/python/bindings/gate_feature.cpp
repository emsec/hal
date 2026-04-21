#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_gate_features(py::module& m, py::module& py_gate_feature)
            {
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

        // BooleanInfluence class
        py::class_<machine_learning::gate_feature::BooleanInfluence, machine_learning::gate_feature::GateFeature> py_boolean_influence(py_gate_feature, "BooleanInfluence", R"(
            Gate feature representing the Boolean influence of a gate's output nets on downstream sequential gate inputs.
            Only meaningful for sequential gates; returns zero for all other gate types.
        )");

        py_boolean_influence.def(py::init<const std::vector<machine_learning::StatisticalMoment>&>(),
                                 py::arg("moments") = std::vector<machine_learning::StatisticalMoment>(),
                                 R"(
            Construct a BooleanInfluence gate feature.

            :param list[hal_py.machine_learning.StatisticalMoment] moments: The statistical moments to compute over the collected influences. Defaults to [average].
        )");

        py_boolean_influence.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::BooleanInfluence& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                Calculate the Boolean influence feature for the given gate in the given feature context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A list of floats representing the requested statistical moments of the Boolean influence on success, None otherwise.
                :rtype: list[float] or None
            )");

        py_boolean_influence.def("to_string", &machine_learning::gate_feature::BooleanInfluence::to_string, R"(
            Get the string representation of the BooleanInfluence gate feature.

            :returns: The string representation.
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
            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
