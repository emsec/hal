#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_gate_labels(py::module& m, py::module& py_gate_label)
            {
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

        py::class_<machine_learning::gate_label::ModuleNameKeyWords, machine_learning::gate_label::GateLabel, std::shared_ptr<machine_learning::gate_label::ModuleNameKeyWords>>
            py_module_name_key_words(py_gate_label, "ModuleNameKeyWords", R"(
            Labels gates based on whether their name includes a keyword of a selection or not.
        )");

        py_module_name_key_words
            .def(py::init<const std::vector<std::string>&, const std::vector<GateTypeProperty>&, bool, bool>(),
                 py::arg("key_words"),
                 py::arg("applicable_to")  = std::vector<GateTypeProperty>{},
                 py::arg("recursive")      = false,
                 py::arg("allow_multiple") = false,
                 R"(
Construct a ModuleNameKeyWords labeler.

:param list[str] key_words: Keywords to check for in gate names.
:param list[hal_py.machine_learning.GateTypeProperty] applicable_to:
    Gate type properties this label applies to. If an empty list is provided the label is applied to all gate types. Defaults to an empty list.
:param bool recursive: Search parent modules recursively. Defaults to False.
:param bool allow_multiple: Allow multiple labels. Defaults to False.
)")

            .def(py::init<const std::vector<std::vector<std::string>>&, const std::vector<GateTypeProperty>&, bool, bool>(),
                 py::arg("key_words"),
                 py::arg("applicable_to")  = std::vector<GateTypeProperty>{},
                 py::arg("recursive")      = false,
                 py::arg("allow_multiple") = false,
                 R"(
Construct a ModuleNameKeyWords labeler.

:param list[list[str]] key_words: Groups of keywords; each inner list is a group.
:param list[hal_py.machine_learning.GateTypeProperty] applicable_to:
    Gate type properties this label applies to. If an empty list is provided the label is applied to all gate types. Defaults to an empty list.
:param bool recursive: Search parent module recursively. Defaults to False.
:param bool allow_multiple: Allow multiple labels. Defaults to False.
)");

        // py_module_name_key_words.def_readonly("MATCH", &machine_learning::gate_label::ModuleNameKeyWords::MATCH, R"(
        //     A label vector indicating a match.

        //     :type: list[int]
        // )");

        // py_module_name_key_words.def_readonly("NO_MATCH", &machine_learning::gate_label::ModuleNameKeyWords::NO_MATCH, R"(
        //     A label vector indicating no match was found.

        //     :type: list[int]
        // )");

        // py_module_name_key_words.def_readonly("NA", &machine_learning::gate_label::ModuleNameKeyWords::NA, R"(
        //     A label vector indicating not applicable.

        //     :type: list[int]
        // )");

        py_module_name_key_words.def(
            "calculate_label",
            [](const machine_learning::gate_label::ModuleNameKeyWords& self, machine_learning::Context& ctx, const Gate* g) -> std::optional<std::vector<u32>> {
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

        py_module_name_key_words.def(
            "calculate_labels",
            [](const machine_learning::gate_label::ModuleNameKeyWords& self, machine_learning::Context& ctx, const std::vector<Gate*>& gates) -> std::optional<std::vector<std::vector<u32>>> {
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

        py_module_name_key_words.def(
            "calculate_labels",
            [](const machine_learning::gate_label::ModuleNameKeyWords& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<u32>>> {
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

        py_module_name_key_words.def("to_string",
                                     &machine_learning::gate_label::ModuleNameKeyWords::to_string,
                                     R"(
            Get a string representation of the gate labeler.

            :returns: The string representation.
            :rtype: str
        )");

            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
