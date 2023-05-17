#include "hal_core/python_bindings/python_bindings.h"

#include "dataflow_analysis/plugin_dataflow.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;
namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(dataflow, m)
    {
        m.doc() = "hal DataflowPlugin python bindings";
#else
    PYBIND11_PLUGIN(dataflow)
    {
        py::module m("dataflow", "hal DataflowPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE
        py::class_<plugin_dataflow, RawPtrWrapper<plugin_dataflow>, BasePluginInterface>(m, "DataflowPlugin")
            .def_property_readonly("name", &plugin_dataflow::get_name, R"(
                The name of the plugin.

                :type: str
                )")
            .def("get_name", &plugin_dataflow::get_name, R"(
                Get the name of the plugin.

                :returns: Plugin name.
                :rtype: str
                )")
            .def_property_readonly("version", &plugin_dataflow::get_version, R"(
                The version of the plugin.

                :type: str
                )")
            .def("get_version", &plugin_dataflow::get_version, R"(
                Get the version of the plugin.

                :returns: Plugin version.
                :rtype: str
                )")
            .def("execute",
                 &plugin_dataflow::execute,
                 py::arg("netlist"),
                 py::arg("output_path"),
                 py::arg("sizes"),
                 py::arg("draw_graph")                    = false,
                 py::arg("create_modules")                = false,
                 py::arg("register_stage_identification") = false,
                 py::arg("known_groups")                  = std::vector<std::vector<u32>>(),
                 py::arg("min_group_size")                = 8,
                 R"(
                Executes the dataflow analysis plugin (DANA). Starting from the netlist DANA tries to identify high-level registers.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :param str output_path: Path where the dataflow graph should be written to
                :param list[int] sizes: Prioritized sizes.
                :param bool draw_graph: Switch to turn on/off the generation of the graph.
                :param bool create_modules: Switch to turn on/off the creation of HAL modules for the registers.
                :param bool register_stage_identification: Switch to turn on/off the register stage rule. Note that this rule can be too restrictive and is turned off by default.
                :param list[list[int]] known_groups: Previously known groups that stay untouched.
                :returns: Register groups created by DANA
                :rtype: list[list[hal_py.Gate]]
                )");

        auto py_dataflow = m.def_submodule("Dataflow");

        py::class_<dataflow::Configuration, RawPtrWrapper<dataflow::Configuration>> py_dataflow_configuration(py_dataflow, "Configuration", R"(
            Holds the configuration of a dataflow analysis run.
        )");

        py_dataflow_configuration.def(py::init<>(), R"(
            Constructs a new dataflow analysis configuration.
        )");

        py_dataflow_configuration.def_readwrite("min_group_size", &dataflow::Configuration::min_group_size, R"(
            Minimum size of a group. Smaller groups will be penalized during analysis. Defaults to 8.

            :type: int
        )");

        py_dataflow_configuration.def_readwrite("expected_sizes", &dataflow::Configuration::expected_sizes, R"(
            Expected group sizes. Groups of these sizes will be prioritized. Defaults to an empty list.

            :type: list[int]
        )");

        py_dataflow_configuration.def_readwrite("known_groups", &dataflow::Configuration::known_groups, R"(
            Already identified groups of sequential gates as a list of groups with each group being a list of gate IDs. Defaults to an empty list.

            :type: list[list[int]]
        )");

        py_dataflow_configuration.def_readwrite("enable_register_stages", &dataflow::Configuration::enable_register_stages, R"(
            Enable register stage identification as part of dataflow analysis. Defaults to ``False``.

            :type: bool
        )");

        py_dataflow_configuration.def("with_min_group_size", &dataflow::Configuration::with_min_group_size, py::arg("size"), R"(
            Set the minimum size of a group. Smaller groups will be penalized during analysis.

            :param int size: The minimum group size.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_expected_sizes", &dataflow::Configuration::with_expected_sizes, py::arg("sizes"), R"(
            Set the expected group sizes. Groups of these sizes will be prioritized.

            :param list[int] sizes: The expected group sizes.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_groups", py::overload_cast<const std::vector<Module*>&>(&dataflow::Configuration::with_known_groups), py::arg("groups"), R"(
            Set already identified groups of sequential gates as a list of groups with each group being a module.

            :param list[hal_py.Module] groups: A list of groups.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_groups", py::overload_cast<const std::vector<Grouping*>&>(&dataflow::Configuration::with_known_groups), py::arg("groups"), R"(
            Set already identified groups of sequential gates as a vector of groups with each group being a grouping.

            :param list[hal_py.Grouping] groups: A list of groups.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_groups", py::overload_cast<const std::vector<std::vector<Gate*>>&>(&dataflow::Configuration::with_known_groups), py::arg("groups"), R"(
            Set already identified groups of sequential gates as a list of groups with each group being a list of gates.

            :param list[list[hal_py.Gate]] groups: A list of groups.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_groups", py::overload_cast<const std::vector<std::vector<u32>>&>(&dataflow::Configuration::with_known_groups), py::arg("groups"), R"(
            Set already identified groups of sequential gates as a list of groups with each group being a list of gate IDs.

            :param list[list[int]] groups: A list of groups.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_register_stage_identification", &dataflow::Configuration::with_register_stage_identification, py::arg("enable") = true, R"(
            Enable register stage identification as part of dataflow analysis.

            :param bool enable: Set ``True`` to enable register stage identification, ``False`` otherwise. Defaults to ``True``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_type_consistency", &dataflow::Configuration::with_type_consistency, py::arg("enable") = true, R"(
            Enable type consistency as part of dataflow analysis when deciding whether two gates are allowed to merge into the same group.

            :param bool enable: Set ``True`` to enable type consistency inside of a group, ``False`` otherwise. Defaults to ``True``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow.def(
            "analyze",
            [](Netlist* nl, const dataflow::Configuration& config = dataflow::Configuration()) -> std::optional<dataflow::Result> {
                auto res = dataflow::analyze(nl, config);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while analyzing dataflow:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("config") = dataflow::Configuration(),
            R"(
                Analyze the datapath to identify word-level registers in the given netlist.

                :param hal_py.Netlist nl: The netlist.
                :param dataflow.Dataflow.Configuration config: The dataflow analysis configuration.
                :returns: The dataflow analysis result on success, ``None`` otherwise.
                :rtype: dataflow.Dataflow.Result or None
            )");

        py::class_<dataflow::Result, RawPtrWrapper<dataflow::Result>> py_dataflow_result(py_dataflow, "Result", R"(
            The result of a dataflow analysis run containing the identified groups of sequential gates and their interconnections.
        )");

        py_dataflow_result.def("get_netlist", &dataflow::Result::get_netlist, R"(
            Get the netlist on which dataflow analysis has been performed.

            :returns: The netlist.
            :rtype: hal_py.Netlist
        )");

        py_dataflow_result.def("get_groups", &dataflow::Result::get_groups, R"(
            Get the groups of sequential gates resulting from dataflow analysis. 
        
            :returns: A dict from group ID to a set of gates belonging to the respective group.
            :rtype: dict[int,set[hal_py.Gate]]
        )");

        py_dataflow_result.def("get_gates", &dataflow::Result::get_gates, R"(
            Get all gates contained in any of the groups groups.
        
            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_dataflow_result.def(
            "get_gates_of_group",
            [](const dataflow::Result& self, const u32 group_id) -> std::optional<std::unordered_set<Gate*>> {
                auto res = self.get_gates_of_group(group_id);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting gates of group:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("group_id"),
            R"(
            Get the gates of the specified group of sequential gates.

            :param int group_id: The ID of the group.
            :returns: The gates of the group as a set on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_dataflow_result.def(
            "get_group_id_of_gate",
            [](const dataflow::Result& self, const Gate* gate) -> std::optional<u32> {
                auto res = self.get_group_id_of_gate(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting group ID of gate:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Get the group ID of the group that contains the given gate. 

            :param hal_py.Gate gate: The gate.
            :returns: The group ID on success, ``None`` otherwise.
            :rtype: int or None
        )");

        py_dataflow_result.def(
            "get_group_control_nets",
            [](const dataflow::Result& self, const u32 group_id, const PinType type) -> std::optional<std::unordered_set<Net*>> {
                auto res = self.get_group_control_nets(group_id, type);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting group control nets:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("group_id"),
            py::arg("type"),
            R"(
            Get the control nets of the group with the given group ID that are connected to a pin of the specified type.

            :param int group_id: The group ID.
            :param hal_py.PinType type: The pin type.
            :returns: A set of control nets of the group on success, ``None`` otherwise.
            :rtype: set[hal_py.Net] or None
        )");

        py_dataflow_result.def(
            "get_gate_control_nets",
            [](const dataflow::Result& self, const Gate* gate, const PinType type) -> std::optional<std::unordered_set<Net*>> {
                auto res = self.get_gate_control_nets(gate, type);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting gate control nets:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("type"),
            R"(
            Get the control nets of the given gate that are connected to a pin of the specified type.

            :param hal_py.Gate gate: The gate.
            :param hal_py.PinType type: The pin type.
            :returns: A set of control nets of the gate on success, ``None`` otherwise.
            :rtype: set[hal_py.Net] or None
        )");

        py_dataflow_result.def(
            "get_group_successors",
            [](const dataflow::Result& self, const u32 group_id) -> std::optional<std::unordered_set<u32>> {
                auto res = self.get_group_successors(group_id);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting successor groups:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("group_id"),
            R"(
            Get the successor groups of the group with the given ID.

            :param int group_id: The group ID.
            :returns: The successors of the group as a set of group IDs on success, ``None`` otherwise.
            :rtype: set[int] or None
        )");

        py_dataflow_result.def(
            "get_gate_successors",
            [](const dataflow::Result& self, const Gate* gate) -> std::optional<std::unordered_set<Gate*>> {
                auto res = self.get_gate_successors(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting successor gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Get the sequential successor gates of the given sequential gate.

            :param hal_py.Gate gate: The gate.
            :returns: The successors of the gate as a set of gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_dataflow_result.def(
            "get_group_predecessors",
            [](const dataflow::Result& self, const u32 group_id) -> std::optional<std::unordered_set<u32>> {
                auto res = self.get_group_predecessors(group_id);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting predecessor groups:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("group_id"),
            R"(
            Get the predecessor groups of the group with the given ID.

            :param int group_id: The ID of the group.
            :returns: The predecessors of the group as a set of group IDs on success, ``None`` otherwise.
            :rtype: set[int] or None
        )");

        py_dataflow_result.def(
            "get_gate_predecessors",
            [](const dataflow::Result& self, const Gate* gate) -> std::optional<std::unordered_set<Gate*>> {
                auto res = self.get_gate_predecessors(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting predecessor gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Get the sequential predecessor gates of the given sequential gate.

            :param hal_py.Gate gate: The gate.
            :returns: The predecessors of the gate as a set of gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_dataflow_result.def(
            "write_dot",
            [](const dataflow::Result& self, const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids = {}) -> bool {
                auto res = self.write_dot(out_path, group_ids);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while writing DOT graph:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("out_path"),
            py::arg("group_ids") = std::unordered_set<u32>(),
            R"(
                Write the dataflow graph as a DOT graph to the specified location.

                :param pathlib.Path out_path: The output path.
                :param set[int] group_ids: The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        py_dataflow_result.def(
            "write_txt",
            [](const dataflow::Result& self, const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids = {}) -> bool {
                auto res = self.write_txt(out_path, group_ids);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while writing DOT graph:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("out_path"),
            py::arg("group_ids") = std::unordered_set<u32>(),
            R"(
                Write the groups resulting from dataflow analysis to a `.txt` file.

                :param pathlib.Path out_path: The output path.
                :param set[int] group_ids: The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        py_dataflow_result.def(
            "create_modules",
            [](const dataflow::Result& self, const std::unordered_set<u32>& group_ids = {}) -> std::optional<std::unordered_map<u32, Module*>> {
                auto res = self.create_modules(group_ids);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating modules:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("group_ids") = std::unordered_set<u32>(),
            R"(
                Create modules for the dataflow analysis result.

                :param set[int] group_ids: The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
                :returns: A map from group IDs to Modules on success, ``None`` otherwise.
                :rtype: dict[int,hal_py.Module] or None
            )");

        py_dataflow_result.def("get_group_list",
                               &dataflow::Result::get_groups_as_list,
                               py::arg("group_ids") = std::unordered_set<u32>(),
                               R"(
                            Get the groups of the dataflow analysis result as a list.

                            :param set[int] group_ids: The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
                            :returns: A list of groups with each group being a list of gates.
                            :rtype: list[list[hal_py.Gate]]
                        )");

        py_dataflow_result.def(
            "merge_groups",
            [](dataflow::Result& self, const std::vector<u32>& group_ids) -> std::optional<u32> {
                auto res = self.merge_groups(group_ids);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while merging groups:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("group_ids"),
            R"(
                Merge multiple groups specified by ID. 
                All specified groups are merged into the first group of the provided vector and are subsequently deleted.

                :param set[int] group_ids: The group IDs of the groups to merge.
                :returns: The ID of the group that all over groups have been merged into on success, ``None`` otherwise.
                :rtype: int or None
            )");

        py_dataflow_result.def(
            "split_group",
            [](dataflow::Result& self, u32 group_id, const std::vector<std::unordered_set<Gate*>>& new_groups) -> std::optional<std::vector<u32>> {
                auto res = self.split_group(group_id, new_groups);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while splitting group:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("group_id"),
            py::arg("new_groups"),
            R"(
                Split a group into multiple smaller groups specified by sets of gates.
                All gates of the group to split must be contained in the sets exactly once and all gates in the sets must be contained in the group to split.
                The group that is being split is deleted in the process.

                :param int group_id: The group ID of the group to split.
                :param list[set[hal_py.Gate]] new_groups: A list of groups specified as unordered sets of gates.
                :returns: The group IDs of the newly created groups in the order of the provided sets.
                :rtype: list[int]
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
