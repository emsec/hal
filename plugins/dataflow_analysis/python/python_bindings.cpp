#include "hal_core/python_bindings/python_bindings.h"

#include "dataflow_analysis/common/result.h"
#include "dataflow_analysis/dataflow.h"
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
                 py::arg("bad_group_sizes")               = 7,
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
        py_dataflow.def(
            "analyze",
            [](Netlist* nl, const std::vector<u32>& sizes = {}, bool register_stage_identification = false, const std::vector<std::vector<u32>>& known_groups = {}, const u32 bad_group_size = 7)
                -> std::optional<dataflow::Result> {
                auto res = dataflow::analyze(nl, sizes, register_stage_identification, known_groups, bad_group_size);
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
            py::arg("sizes")                         = std::vector<u32>(),
            py::arg("register_stage_identification") = false,
            py::arg("known_groups")                  = std::vector<std::vector<u32>>(),
            py::arg("bad_group_size")                = 7,
            R"(
                Analyze the datapath to identify word-level registers in the given netlist.

                :param hal_py.Netlist nl: The netlist.
                :param pathlib.Path out_path: The output path to which the results are written.
                :param list[int] sizes: Register sizes that are expected to be found in the netlist. These sizes will be prioritized over others during analysis. Defaults to an empty list.
                :param bool register_stage_identification: Set ``True``to enable register stage identification during analysis, ``False`` otherwise. Defaults to ``False``.
                :param list[list[int]] known_groups: Registers that have been identified prior to dataflow analysis. Must be provided as a list of registers with each register being represented as a list of gate IDs. Defaults to an empty list.
                :param int bad_group_size: Minimum expected register size. Smaller registers will not be considered during analysis. Defults to ``7``.
                :returns: The dataflow analysis result on success, ``None`` otherwise.
                :rtype: hal_py.Dataflow.Result or None
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
            "get_control_nets_of_group",
            [](const dataflow::Result& self, const u32 group_id, const PinType type) -> std::optional<std::unordered_set<Net*>> {
                auto res = self.get_control_nets_of_group(group_id, type);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting control nets of group:\n{}", res.get_error().get());
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
                :rtype: bool
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

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
