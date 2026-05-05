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

        py_gate_feature_class.def("get_legend",
                                  &machine_learning::gate_feature::GateFeature::get_legend,
                                  py::arg("feature_context"),
                                  R"(
                Get one concise label per index of the feature vector produced by this gate feature.

                The length of the returned list equals the width of the feature vector produced by
                :meth:`calculate_feature` for the same context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :returns: One concise label per index of the feature vector.
                :rtype: list[str]
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

        // PinCount class
        py::class_<machine_learning::gate_feature::PinCount, machine_learning::gate_feature::GateFeature> py_pin_count(py_gate_feature, "PinCount", R"(
            Gate feature counting the pins on a gate's type that match the configured direction
            and pin-type filters. Purely a gate-type property; does not depend on net connectivity.
        )");

        py_pin_count.def(py::init<const std::vector<PinDirection>&, const std::vector<PinType>&>(),
                         py::arg("directions") = std::vector<PinDirection>(),
                         py::arg("pin_types")  = std::vector<PinType>(),
                         R"(
            Construct a PinCount gate feature.

            :param list[hal_py.PinDirection] directions: Accepted pin directions. Empty means accept any direction. Defaults to empty.
            :param list[hal_py.PinType] pin_types: Accepted pin types. Empty means accept any type. Defaults to empty.
        )");

        py_pin_count.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::PinCount& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                :returns: A single-element list with the matching pin count on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_pin_count.def("to_string",
                         &machine_learning::gate_feature::PinCount::to_string,
                         R"(
                Get the string representation of the PinCount gate feature.

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

        // NeighborCount class
        py::class_<machine_learning::gate_feature::NeighborCount, machine_learning::gate_feature::GateFeature> py_neighbor_count(py_gate_feature, "NeighborCount", R"(
            Gate feature counting the gates reachable from the source gate within a bounded
            traversal on the original netlist abstraction.
        )");

        py_neighbor_count.def(py::init<const u32, const PinDirection, const bool, const std::vector<PinType>&, const std::vector<PinType>&>(),
                              py::arg("depth"),
                              py::arg("direction"),
                              py::arg("directed")            = true,
                              py::arg("starting_pin_types")  = std::vector<PinType>(),
                              py::arg("forbidden_pin_types") = std::vector<PinType>(),
                              R"(
            Construct a NeighborCount gate feature.

            :param int depth: Maximum traversal depth (in gate hops).
            :param hal_py.PinDirection direction: Direction of traversal.
            :param bool directed: Whether to respect signal direction. Defaults to True.
            :param list[hal_py.PinType] starting_pin_types: If non-empty, only pins of these types on the source gate start the traversal. Defaults to empty.
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are never crossed. Defaults to empty.
        )");

        py_neighbor_count.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::NeighborCount& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                :returns: A single-element list with the neighborhood size on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_neighbor_count.def("to_string",
                              &machine_learning::gate_feature::NeighborCount::to_string,
                              R"(
                Get the string representation of the NeighborCount gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        // SequentialNeighborCount class
        py::class_<machine_learning::gate_feature::SequentialNeighborCount, machine_learning::gate_feature::GateFeature> py_sequential_neighbor_count(
            py_gate_feature, "SequentialNeighborCount", R"(
            Gate feature counting the sequential gates reachable from the source gate within a
            bounded traversal on the sequential netlist abstraction.
        )");

        py_sequential_neighbor_count.def(py::init<const u32, const PinDirection, const bool, const std::vector<PinType>&, const std::vector<PinType>&>(),
                                         py::arg("depth"),
                                         py::arg("direction"),
                                         py::arg("directed")            = true,
                                         py::arg("starting_pin_types")  = std::vector<PinType>(),
                                         py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                         R"(
            Construct a SequentialNeighborCount gate feature.

            :param int depth: Maximum traversal depth (in sequential-gate hops).
            :param hal_py.PinDirection direction: Direction of traversal.
            :param bool directed: Whether to respect signal direction. Defaults to True.
            :param list[hal_py.PinType] starting_pin_types: If non-empty, only pins of these types on the source gate start the traversal. Defaults to empty.
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are never crossed. Defaults to empty.
        )");

        py_sequential_neighbor_count.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::SequentialNeighborCount& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                :returns: A single-element list with the sequential neighborhood size on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_sequential_neighbor_count.def("to_string",
                                         &machine_learning::gate_feature::SequentialNeighborCount::to_string,
                                         R"(
                Get the string representation of the SequentialNeighborCount gate feature.

                :returns: The string representation.
                :rtype: str
            )");

        py::enum_<machine_learning::NormalizationType>(m, "NormalizationType", R"(
            Selects how a vector of feature values is normalized.
        )")
            .value("None_", machine_learning::NormalizationType::None, R"(
                Leave values untouched.
            )")
            .value("MinMax", machine_learning::NormalizationType::MinMax, R"(
                Min-max normalize across the batch into [0, 1].
            )")
            .value("GateCount", machine_learning::NormalizationType::GateCount, R"(
                Divide every value by the number of gates in the netlist.
            )")
            .export_values();

        py::class_<machine_learning::gate_feature::BetweennessCentrality, machine_learning::gate_feature::GateFeature> py_betweenness_centrality(py_gate_feature, "BetweennessCentrality", R"(
            A class representing the betweenness centrality feature for gates.
            )");

        py_betweenness_centrality.def(py::init<bool, i32, machine_learning::NormalizationType>(),
                                      py::arg("directed")      = true,
                                      py::arg("cutoff")        = -1,
                                      py::arg("normalization") = machine_learning::NormalizationType::MinMax,
                                      R"(
            Construct a BetweennessCentrality object.

            :param bool directed: Whether the graph is directed. Defaults to True.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            :param hal_py.machine_learning.NormalizationType normalization: Normalization to apply to the centrality values. Defaults to MinMax.
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

        py_harmonic_centrality.def(py::init<PinDirection, i32, machine_learning::NormalizationType>(),
                                   py::arg("direction"),
                                   py::arg("cutoff")        = -1,
                                   py::arg("normalization") = machine_learning::NormalizationType::MinMax,
                                   R"(
            Construct a HarmonicCentrality object.

            :param hal_py.PinDirection direction: The pin direction to consider.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            :param hal_py.machine_learning.NormalizationType normalization: Normalization to apply to the centrality values. Defaults to MinMax.
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

        py_sequential_betweenness_centrality.def(py::init<bool, i32, machine_learning::NormalizationType>(),
                                                 py::arg("directed")      = true,
                                                 py::arg("cutoff")        = -1,
                                                 py::arg("normalization") = machine_learning::NormalizationType::MinMax,
                                                 R"(
            Construct a SequentialBetweennessCentrality object.

            :param bool directed: Whether the graph is directed. Defaults to True.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            :param hal_py.machine_learning.NormalizationType normalization: Normalization to apply to the centrality values. Defaults to MinMax.
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

        py_sequential_harmonic_centrality.def(py::init<PinDirection, i32, machine_learning::NormalizationType>(),
                                              py::arg("direction"),
                                              py::arg("cutoff")        = -1,
                                              py::arg("normalization") = machine_learning::NormalizationType::MinMax,
                                              R"(
            Construct a SequentialHarmonicCentrality object.

            :param hal_py.PinDirection direction: The pin direction to consider.
            :param int cutoff: The maximum distance to consider. Defaults to -1.
            :param hal_py.machine_learning.NormalizationType normalization: Normalization to apply to the centrality values. Defaults to MinMax.
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

        // InCycle class
        py::class_<machine_learning::gate_feature::InCycle, machine_learning::gate_feature::GateFeature> py_in_cycle(py_gate_feature, "InCycle", R"(
            Gate feature: binary indicator that the gate is part of a directed cycle on the
            original netlist graph.
        )");

        py_in_cycle.def(py::init<const i32, const std::vector<PinType>&>(),
                        py::arg("cutoff")              = -1,
                        py::arg("forbidden_pin_types") = std::vector<PinType>(),
                        R"(
            Construct an InCycle gate feature.

            :param int cutoff: Maximum cycle length to consider. -1 means unbounded. Defaults to -1.
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are not crossed during traversal. Defaults to empty.
        )");

        py_in_cycle.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::InCycle& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                Calculate the InCycle feature for the given gate in the given feature context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A single-element list with the binary indicator on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_in_cycle.def("to_string", &machine_learning::gate_feature::InCycle::to_string, R"(
            Get the string representation of the InCycle gate feature.

            :returns: The string representation.
            :rtype: str
        )");

        // ShortestCycleLength class
        py::class_<machine_learning::gate_feature::ShortestCycleLength, machine_learning::gate_feature::GateFeature> py_shortest_cycle_length(py_gate_feature, "ShortestCycleLength", R"(
            Gate feature: length of the shortest directed cycle through the gate on the original
            netlist graph, clamped to 255.
        )");

        py_shortest_cycle_length.def(py::init<const i32, const std::vector<PinType>&>(),
                                     py::arg("cutoff")              = -1,
                                     py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                     R"(
            Construct a ShortestCycleLength gate feature.

            :param int cutoff: Maximum cycle length to consider. -1 means unbounded. Defaults to -1.
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are not crossed during traversal. Defaults to empty.
        )");

        py_shortest_cycle_length.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::ShortestCycleLength& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                Calculate the ShortestCycleLength feature for the given gate in the given feature context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A single-element list with the cycle length on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_shortest_cycle_length.def("to_string", &machine_learning::gate_feature::ShortestCycleLength::to_string, R"(
            Get the string representation of the ShortestCycleLength gate feature.

            :returns: The string representation.
            :rtype: str
        )");

        // InCycleOfLength class
        py::class_<machine_learning::gate_feature::InCycleOfLength, machine_learning::gate_feature::GateFeature> py_in_cycle_of_length(py_gate_feature, "InCycleOfLength", R"(
            Gate feature: binary indicator that there is a closed directed walk of length exactly
            `length` through the gate on the original netlist graph.
        )");

        py_in_cycle_of_length.def(py::init<const u32, const std::vector<PinType>&>(),
                                  py::arg("length"),
                                  py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                  R"(
            Construct an InCycleOfLength gate feature.

            :param int length: The walk length to test for (number of edges).
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are not crossed during traversal. Defaults to empty.
        )");

        py_in_cycle_of_length.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::InCycleOfLength& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                Calculate the InCycleOfLength feature for the given gate in the given feature context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A single-element list with the binary indicator on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_in_cycle_of_length.def("to_string", &machine_learning::gate_feature::InCycleOfLength::to_string, R"(
            Get the string representation of the InCycleOfLength gate feature.

            :returns: The string representation.
            :rtype: str
        )");

        // SequentialInCycle class
        py::class_<machine_learning::gate_feature::SequentialInCycle, machine_learning::gate_feature::GateFeature> py_sequential_in_cycle(py_gate_feature, "SequentialInCycle", R"(
            Gate feature: binary indicator that the gate is part of a directed cycle on the
            sequential netlist abstraction. Always 0 for non-sequential gates.
        )");

        py_sequential_in_cycle.def(py::init<const i32, const std::vector<PinType>&>(),
                                   py::arg("cutoff")              = -1,
                                   py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                   R"(
            Construct a SequentialInCycle gate feature.

            :param int cutoff: Maximum cycle length to consider. -1 means unbounded. Defaults to -1.
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are not crossed during traversal. Defaults to empty.
        )");

        py_sequential_in_cycle.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::SequentialInCycle& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                Calculate the SequentialInCycle feature for the given gate in the given feature context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A single-element list with the binary indicator on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_sequential_in_cycle.def("to_string", &machine_learning::gate_feature::SequentialInCycle::to_string, R"(
            Get the string representation of the SequentialInCycle gate feature.

            :returns: The string representation.
            :rtype: str
        )");

        // SequentialShortestCycleLength class
        py::class_<machine_learning::gate_feature::SequentialShortestCycleLength, machine_learning::gate_feature::GateFeature> py_sequential_shortest_cycle_length(
            py_gate_feature, "SequentialShortestCycleLength", R"(
            Gate feature: length of the shortest directed cycle through the gate on the
            sequential netlist abstraction, clamped to 255. Non-sequential gates receive the clamp value.
        )");

        py_sequential_shortest_cycle_length.def(py::init<const i32, const std::vector<PinType>&>(),
                                                py::arg("cutoff")              = -1,
                                                py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                                R"(
            Construct a SequentialShortestCycleLength gate feature.

            :param int cutoff: Maximum cycle length to consider. -1 means unbounded. Defaults to -1.
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are not crossed during traversal. Defaults to empty.
        )");

        py_sequential_shortest_cycle_length.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::SequentialShortestCycleLength& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                Calculate the SequentialShortestCycleLength feature for the given gate in the given feature context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A single-element list with the cycle length on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_sequential_shortest_cycle_length.def("to_string", &machine_learning::gate_feature::SequentialShortestCycleLength::to_string, R"(
            Get the string representation of the SequentialShortestCycleLength gate feature.

            :returns: The string representation.
            :rtype: str
        )");

        // SequentialInCycleOfLength class
        py::class_<machine_learning::gate_feature::SequentialInCycleOfLength, machine_learning::gate_feature::GateFeature> py_sequential_in_cycle_of_length(
            py_gate_feature, "SequentialInCycleOfLength", R"(
            Gate feature: binary indicator that there is a closed directed walk of length exactly
            `length` through the gate on the sequential netlist abstraction. Always 0 for
            non-sequential gates.
        )");

        py_sequential_in_cycle_of_length.def(py::init<const u32, const std::vector<PinType>&>(),
                                             py::arg("length"),
                                             py::arg("forbidden_pin_types") = std::vector<PinType>(),
                                             R"(
            Construct a SequentialInCycleOfLength gate feature.

            :param int length: The walk length to test for (number of edges).
            :param list[hal_py.PinType] forbidden_pin_types: Pins of these types are not crossed during traversal. Defaults to empty.
        )");

        py_sequential_in_cycle_of_length.def(
            "calculate_feature",
            [](const machine_learning::gate_feature::SequentialInCycleOfLength& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<FEATURE_TYPE>> {
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
                Calculate the SequentialInCycleOfLength feature for the given gate in the given feature context.

                :param hal_py.machine_learning.Context feature_context: The feature context.
                :param hal_py.Gate gate: The gate.
                :returns: A single-element list with the binary indicator on success, None otherwise.
                :rtype: list[FEATURE_TYPE] or None
            )");

        py_sequential_in_cycle_of_length.def("to_string", &machine_learning::gate_feature::SequentialInCycleOfLength::to_string, R"(
            Get the string representation of the SequentialInCycleOfLength gate feature.

            :returns: The string representation.
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
