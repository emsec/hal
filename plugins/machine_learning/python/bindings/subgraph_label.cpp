#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_subgraph_labels(py::module& m, py::module& py_subgraph_label)
            {
        py::class_<machine_learning::subgraph_label::SubgraphLabel, std::shared_ptr<machine_learning::subgraph_label::SubgraphLabel>> py_subgraph_label_class(py_subgraph_label, "SubgraphLabel", R"(
            Base class for calculating labels for machine learning models.

            This abstract class provides methods for calculating labels based on various criteria.
        )");

        py_subgraph_label_class.def(
            "calculate_subgraphs",
            [](const machine_learning::subgraph_label::SubgraphLabel& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<Gate*>>> {
                auto res = self.calculate_subgraphs(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating subgraphs: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate subgraphs for a given machine learning context.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :returns: A list of subgraphs, consisting of a list of gates
            :rtype: list[list[hal_py.Gate]] or None
        )");

        py_subgraph_label_class.def(
            "calculate_labels",
            [](const machine_learning::subgraph_label::SubgraphLabel& self, machine_learning::Context& ctx, const std::vector<Gate*>& subgraphs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, subgraphs);
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
            py::arg("subgraphs"),
            R"(
            Calculate labels for multiple subgraphs.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param list[list[hal_py.Gate]] subgraphs: The subgraphs to label.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_subgraph_label_class.def("to_string",
                                    &machine_learning::subgraph_label::SubgraphLabel::to_string,
                                    R"(
            Get a string representation of the gate label.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::subgraph_label::ContainedComponents, machine_learning::subgraph_label::SubgraphLabel, std::shared_ptr<machine_learning::subgraph_label::ContainedComponents>>
            py_contained_components(py_subgraph_label, "ContainedComponents", R"(
            Labels gates based on whether their name includes a keyword or not.
        )");

        py_contained_components.def(py::init<const std::vector<std::vector<std::string>>&, bool>(),
                                    py::arg("key_words"),
                                    py::arg("binary_flags"),
                                    R"(
Construct a ContainedComponents labeler.

:param list[list[str]] key_words: Groups of keywords; each inner list is a group.
:param bool binary_flags: Outputs only a flag indicating any amount of a component group if true, otherwise it outputs a count.
)");

        py_contained_components.def(
            "calculate_subgraphs",
            [](const machine_learning::subgraph_label::SubgraphLabel& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<Gate*>>> {
                auto res = self.calculate_subgraphs(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating subgraphs: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate subgraphs for a given machine learning context.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :returns: A list of subgraphs, consisting of a list of gates
            :rtype: list[list[hal_py.Gate]] or None
        )");

        py_contained_components.def(
            "calculate_labels",
            [](const machine_learning::subgraph_label::SubgraphLabel& self, machine_learning::Context& ctx, const std::vector<Gate*>& subgraphs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, subgraphs);
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
            py::arg("subgraphs"),
            R"(
            Calculate labels for multiple subgraphs.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param list[list[hal_py.Gate]] subgraphs: The subgraphs to label.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_contained_components.def(
            "annotate_from_twin_netlist",
            [](const machine_learning::subgraph_label::ContainedComponents& self, machine_learning::Context& ctx, Netlist* nl, const Netlist* twin_nl) -> std::optional<u32> {
                auto res = self.annotate_from_twin_netlist(ctx, nl, twin_nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error annotating from twin netlist: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("nl"),
            py::arg("twin_nl"),
            R"(
            Annotate contained components to subgraphs of a flattened netlist based on an identical unflattened netlist.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param hal_py.netlist nl:  The netlist to annotate.
            :param hal_py.netlist twin_nl: The twin netlist to extract the annotations from.
            :returns: The amount of annotated subgraphs, on success, otherwise None
            :rtype: int or None
        )");

        py_contained_components.def("to_string",
                                    &machine_learning::subgraph_label::ContainedComponents::to_string,
                                    R"(
            Get a string representation of the gate labeler.

            :returns: The string representation.
            :rtype: str
        )");

        py::class_<machine_learning::subgraph_label::ContainedComponentsNetlist,
                   machine_learning::subgraph_label::SubgraphLabel,
                   std::shared_ptr<machine_learning::subgraph_label::ContainedComponentsNetlist>>
            py_contained_components_netlist(py_subgraph_label, "ContainedComponentsNetlist", R"(
            Labels gates based on whether their name includes a keyword or not.
        )");

        py_contained_components_netlist.def(py::init<const std::vector<std::vector<std::string>>&, bool>(),
                                            py::arg("key_words"),
                                            py::arg("binary_flags"),
                                            R"(
Construct a ContainedComponentsNetlist labeler.

:param list[list[str]] key_words: Groups of keywords; each inner list is a group.
:param bool binary_flags: Outputs only a flag indicating any amount of a component group if true, otherwise it outputs a count.
)");

        py_contained_components_netlist.def(
            "calculate_subgraphs",
            [](const machine_learning::subgraph_label::SubgraphLabel& self, machine_learning::Context& ctx) -> std::optional<std::vector<std::vector<Gate*>>> {
                auto res = self.calculate_subgraphs(ctx);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error calculating subgraphs: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            R"(
            Calculate subgraphs for a given machine learning context.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :returns: A list of subgraphs, consisting of a list of gates
            :rtype: list[list[hal_py.Gate]] or None
        )");

        py_contained_components_netlist.def(
            "calculate_labels",
            [](const machine_learning::subgraph_label::SubgraphLabel& self, machine_learning::Context& ctx, const std::vector<Gate*>& subgraphs) -> std::optional<std::vector<std::vector<u32>>> {
                auto res = self.calculate_labels(ctx, subgraphs);
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
            py::arg("subgraphs"),
            R"(
            Calculate labels for multiple subgraphs.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param list[list[hal_py.Gate]] subgraphs: The subgraphs to label.
            :returns: A list of label vectors on success, or None otherwise.
            :rtype: list[list[int]] or None
        )");

        py_contained_components_netlist.def(
            "annotate_from_netlist_metadata",
            [](const machine_learning::subgraph_label::ContainedComponentsNetlist& self, machine_learning::Context& ctx, Netlist* nl, const std::string& metadata_path) -> std::optional<u32> {
                auto res = self.annotate_from_netlist_metadata(ctx, nl, metadata_path);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error annotating from netlist metadata: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("nl"),
            py::arg("metadata_path"),
            R"(
            Annotate contained components to subgraphs of a netlist based on metadata stored in the netlist.

            :param hal_py.machine_learning.Context ctx: The machine learning context.
            :param hal_py.netlist nl:  The netlist to annotate.
            :param str metadata_path: The metadata path to extract the annotations from.
            :returns: The amount of annotated subgraphs, on success, otherwise None
            :rtype: int or None
        )"),

            py_contained_components_netlist.def("to_string",
                                                &machine_learning::subgraph_label::ContainedComponentsNetlist::to_string,
                                                R"(
            Get a string representation of the gate labeler.

            :returns: The string representation.
            :rtype: str
        )");

            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
