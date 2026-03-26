#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_gate_pair_features(py::module& m, py::module& py_gate_pair_feature)
            {
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
            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
