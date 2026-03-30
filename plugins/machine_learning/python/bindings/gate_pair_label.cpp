#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_gate_pair_labels(py::module& m, py::module& py_gate_pair_label)
            {
        UNUSED(m);

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

            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
