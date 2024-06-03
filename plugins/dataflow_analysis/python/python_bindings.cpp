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
        m.doc() = "Dataflow analysis tool DANA to recover word-level structures such as registers from gate-level netlists.";
#else
    PYBIND11_PLUGIN(dataflow)
    {
        py::module m("dataflow", "Dataflow analysis tool DANA to recover word-level structures such as registers from gate-level netlists.");
#endif    // ifdef PYBIND11_MODULE
        py::class_<DataflowPlugin, RawPtrWrapper<DataflowPlugin>, BasePluginInterface> py_dataflow_plugin(
            m, "DataflowPlugin", R"(This class provides an interface to integrate the DANA tool as a plugin within the HAL framework.)");

        py_dataflow_plugin.def_property_readonly("name", &DataflowPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_dataflow_plugin.def("get_name", &DataflowPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_dataflow_plugin.def_property_readonly("description", &DataflowPlugin::get_description, R"(
            The short description of the plugin.

            :type: str
        )");

        py_dataflow_plugin.def("get_description", &DataflowPlugin::get_description, R"(
            Get the short description of the plugin.

            :returns: The short description of the plugin.
            :rtype: str
        )");

        py_dataflow_plugin.def_property_readonly("version", &DataflowPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_dataflow_plugin.def("get_version", &DataflowPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py::class_<dataflow::Configuration, RawPtrWrapper<dataflow::Configuration>> py_dataflow_configuration(m, "Configuration", R"(
            This class holds all information relevant for the configuration of a dataflow analysis run, including the netlist to analyze.
        )");

        py_dataflow_configuration.def(py::init<Netlist*>(), py::arg("nl"), R"(
            Construct a new dataflow analysis configuration for the given netlist.
            
            :param hal_py.Netlist nl: The netlist.
        )");

        py_dataflow_configuration.def_readwrite("min_group_size", &dataflow::Configuration::min_group_size, R"(
            Minimum size of a group. Smaller groups will be penalized during analysis. Defaults to 8.

            :type: int
        )");

        py_dataflow_configuration.def_readwrite("expected_sizes", &dataflow::Configuration::expected_sizes, R"(
            Expected group sizes. Groups of these sizes will be prioritized. Defaults to an empty list.

            :type: list[int]
        )");

        py_dataflow_configuration.def_readwrite("known_gate_groups", &dataflow::Configuration::known_gate_groups, R"(
            Groups of gates that have already been identified as word-level groups beforehand. All gates of a group must be of one of the target gate types. Defaults to an empty list.

            :type: list[list[hal_py.Gate]]
        )");

        py_dataflow_configuration.def_readwrite("known_net_groups", &dataflow::Configuration::known_net_groups, R"(
            Groups of nets that have been identified as word-level datapathes beforehand. Defaults to an empty list.
            :type: list[list[hal_py.Net]]
        )");

        py_dataflow_configuration.def_readwrite("gate_types", &dataflow::Configuration::gate_types, R"(
            The gate types to be grouped by dataflow analysis. Defaults to an empty set.
            :type: set[hal_py.GateType]
        )");

        py_dataflow_configuration.def_readwrite("control_pin_types", &dataflow::Configuration::control_pin_types, R"(
            The pin types of the pins to be considered control pins. Defaults to an empty set.
            :type: set[hal_py.PinType]
        )");

        py_dataflow_configuration.def_readwrite("enable_stages", &dataflow::Configuration::enable_stages, R"(
            Enable stage identification as part of dataflow analysis. Defaults to ``False``.

            :type: bool
        )");

        py_dataflow_configuration.def_readwrite("enforce_type_consistency", &dataflow::Configuration::enforce_type_consistency, R"(
            Enforce gate type consistency inside of a group. Defaults to ``False``.
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

        py_dataflow_configuration.def(
            "with_known_structures", py::overload_cast<const std::vector<Module*>&, bool>(&dataflow::Configuration::with_known_structures), py::arg("structures"), py::arg("overwrite") = false, R"(
            Add modules to the set of previously identified word-level structures.
            The gates contained in the modules do not have to be of the target gate types.
            The input and output pin groups of these modules will be used to guide datapath analysis.
            Only pin groups larger than ``min_group_size`` will be considered.
            :param list[hal_py.Module] structures: A list of modules.
            :param bool overwrite: Set ``True`` to overwrite the existing known word-level structures, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_structures",
                                      py::overload_cast<const std::vector<std::pair<Module*, std::vector<PinGroup<ModulePin>*>>>&, bool>(&dataflow::Configuration::with_known_structures),
                                      py::arg("structures"),
                                      py::arg("overwrite") = false,
                                      R"(
            Add modules to the set of previously identified word-level structures.
            The gates contained in the modules do not have to be of the target gate types.
            The input and output pin groups of these modules will be used to guide datapath analysis.
            For each module, the input and output pin groups to be considered for analysis must be specified. 
            An empty pin group vector results in all pin groups of the module being considered.
            Only pin groups larger than ``min_group_size`` will be considered.
            :param list[tuple(hal_py.Module,list[hal_py.ModulePinGroup])] structures: A list of modules, each of them with a list of module pin groups.
            :param bool overwrite: Set ``True`` to overwrite the existing known word-level structures, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def(
            "with_known_structures", py::overload_cast<const std::vector<Gate*>&, bool>(&dataflow::Configuration::with_known_structures), py::arg("structures"), py::arg("overwrite") = false, R"(
            Add (typically large) gates to the set of previously identified word-level structures.
            The gates do not have to be of the target gate types.
            The input and output pin groups of these gates will be used to guide datapath analysis.
            Only pin groups larger than ``min_group_size`` will be considered.
            :param list[hal_py.Gate] structures: A list of gates.
            :param bool overwrite: Set ``True`` to overwrite the existing known word-level structures, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_structures",
                                      py::overload_cast<const std::vector<std::pair<Gate*, std::vector<PinGroup<GatePin>*>>>&, bool>(&dataflow::Configuration::with_known_structures),
                                      py::arg("structures"),
                                      py::arg("overwrite") = false,
                                      R"(
            Add (typically large) gates to the set of previously identified word-level structures.
            The gates do not have to be of the target gate types.
            The input and output pin groups of these gates will be used to guide datapath analysis.
            For each gate, the input and output pin groups to be considered for analysis must be specified. 
            An empty pin group vector results in all pin groups of the gate being considered.
            Only pin groups larger than ``min_group_size`` will be considered.
            :param list[tuple(hal_py.Gate,list[hal_py.GatePinGroup])] structures: A list of gates, each of them with a list of gate pin groups.
            :param bool overwrite: Set ``True`` to overwrite the existing known word-level structures, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_structures",
                                      py::overload_cast<const std::unordered_set<const GateType*>&, bool>(&dataflow::Configuration::with_known_structures),
                                      py::arg("structures"),
                                      py::arg("overwrite") = false,
                                      R"(
            Add all gates of a (typically large) gate type to the set of previously identified word-level structures.
            The gate types do not have to be part of the target gate types.
            The input and output pin groups of the gates of these types will be used to guide datapath analysis.
            Only pin groups larger than ``min_group_size`` will be considered.
            :param set[hal_py.GateType] structures: A set of gates.
            :param bool overwrite: Set ``True`` to overwrite the existing known word-level structures, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_structures",
                                      py::overload_cast<const std::unordered_map<const GateType*, std::vector<PinGroup<GatePin>*>>&, bool>(&dataflow::Configuration::with_known_structures),
                                      py::arg("structures"),
                                      py::arg("overwrite") = false,
                                      R"(
            Add all gates of a (typically large) gate type to the set of previously identified word-level structures.
            The gate types do not have to be part of the target gate types.
            The input and output pin groups of the gates of these types will be used to guide datapath analysis.
            For each gate type, the input and output pin groups to be considered for analysis must be specified. 
            An empty pin group vector results in all pin groups of the gate type being considered.
            Only pin groups larger than ``min_group_size`` will be considered.
            :param dict[hal_py.GateType,list[hal_py.GatePinGroup]] structures: A dict from gates to a vector of a subset of their pin groups.
            :param bool overwrite: Set ``True`` to overwrite the existing known word-level structures, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def(
            "with_known_groups", py::overload_cast<const std::vector<Module*>&, bool>(&dataflow::Configuration::with_known_groups), py::arg("groups"), py::arg("overwrite") = false, R"(
            Add modules to the set of previously identified word-level groups.
            These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
            The groups will be used to guide dataflow analysis, but will remain unaltered in the process.

            :param list[hal_py.Module] groups: A list of modules.
            :param bool overwrite: Set ``True`` to overwrite the existing previously identified word-level groups, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_groups",
                                      py::overload_cast<const std::vector<std::vector<Gate*>>&, bool>(&dataflow::Configuration::with_known_groups),
                                      py::arg("groups"),
                                      py::arg("overwrite") = false,
                                      R"(
            Add lists of gates to the set of previously identified word-level groups.
            These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
            The groups will be used to guide dataflow analysis, but will remain unaltered in the process.
            :param list[list[hal_py.Gate]] groups: A list of groups, each of them given as a list of gates.
            :param bool overwrite: Set ``True`` to overwrite the existing previously identified word-level groups, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def(
            "with_known_groups", py::overload_cast<const std::vector<std::vector<u32>>&, bool>(&dataflow::Configuration::with_known_groups), py::arg("groups"), py::arg("overwrite") = false, R"(
            Add lists of gate IDs to the set of previously identified word-level groups.
            These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
            The groups will be used to guide dataflow analysis, but will remain unaltered in the process.

            :param list[list[int]] groups: A list of groups, each of them given as a list of gate IDs.
            :param bool overwrite: Set ``True`` to overwrite the existing previously identified word-level groups, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_known_groups",
                                      py::overload_cast<const std::unordered_map<u32, std::unordered_set<Gate*>>&, bool>(&dataflow::Configuration::with_known_groups),
                                      py::arg("groups"),
                                      py::arg("overwrite") = false,
                                      R"(
            Add groups from a previous dataflow analysis run to the set of previously identified word-level groups.
            These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
            The groups will be used to guide dataflow analysis, but will remain unaltered in the process.
            The group IDs will be ignored during analysis and the same group may be assigned a new ID.
            :param dict[int,set[hal_py.Gate]] groups: A dict from group IDs to groups, each of them given as a set of gates.
            :param bool overwrite: Set ``True`` to overwrite the existing previously identified word-level groups, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def(
            "with_gate_types", py::overload_cast<const std::set<const GateType*>&, bool>(&dataflow::Configuration::with_gate_types), py::arg("types"), py::arg("overwrite") = false, R"(
            Add the gate types to the set of gate types to be grouped by dataflow analysis.
            Overwrite the existing set of gate types by setting the optional ``overwrite`` flag to ``True``.
            :param set[hal_py.GateType] types: A set of gate types.
            :param bool overwrite: Set ``True`` to overwrite existing set of gate types, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def(
            "with_gate_types", py::overload_cast<const std::set<GateTypeProperty>&, bool>(&dataflow::Configuration::with_gate_types), py::arg("type_properties"), py::arg("overwrite") = false, R"(
            Add the gate types featuring the specified properties to the set of gate types to be grouped by dataflow analysis.
            Overwrite the existing set of gate types by setting the optional ``overwrite`` flag to ``True``.

            :param set[hal_py.GateTypeProperty] type_properties: A set of gate type properties.
            :param bool overwrite: Set ``True`` to overwrite existing set of gate types, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_control_pin_types", &dataflow::Configuration::with_control_pin_types, py::arg("types"), py::arg("overwrite") = false, R"(
            Set the pin types of the pins to be considered control pins by dataflow analysis.
            Overwrite the existing set of pin types by setting the optional ``overwrite`` flag to ``True``.

            :param set[hal_py.PinType] types: A set of pin types.
            :param bool enable: Set ``True`` to overwrite existing set of pin types, ``False`` otherwise. Defaults to ``False``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_stage_identification", &dataflow::Configuration::with_stage_identification, py::arg("enable") = true, R"(
            Enable stage identification as part of dataflow analysis.

            :param bool enable: Set ``True`` to enable stage identification, ``False`` otherwise. Defaults to ``True``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        py_dataflow_configuration.def("with_type_consistency", &dataflow::Configuration::with_type_consistency, py::arg("enable") = true, R"(
            Enable type consistency as part of dataflow analysis when deciding whether two gates are allowed to merge into the same group.

            :param bool enable: Set ``True`` to enable type consistency inside of a group, ``False`` otherwise. Defaults to ``True``.
            :returns: The updated dataflow analysis configuration.
            :rtype: dataflow.Dataflow.Configuration
        )");

        m.def(
            "analyze",
            [](const dataflow::Configuration& config) -> std::optional<dataflow::Result> {
                auto res = dataflow::analyze(config);
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
            py::arg("config"),
            R"(
                Analyze the gate-level netlist to identify word-level registers.
                Reconstructs registers based on properties such as the control inputs of their flip-flops and common successors/predecessors.
                Operates on an abstraction of the netlist that contains only flip-flops and connections between two flip-flops only if they are connected through combinational logic.

                :param dataflow.Dataflow.Configuration config: The dataflow analysis configuration.
                :returns: The dataflow analysis result on success, ``None`` otherwise.
                :rtype: dataflow.Dataflow.Result or None
            )");

        py::class_<dataflow::Result, RawPtrWrapper<dataflow::Result>> py_dataflow_result(m, "Result", R"(
            This class holds result of a dataflow analysis run, which contains the identified groups of sequential gates and their interconnections.
            Each such group is assigned a unique ID by which it can be addressed in many of the member functions of this class.
            Please note that this ID is not related to any other HAL ID.
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
            [](const dataflow::Result& self,
               const std::map<const GateType*, std::string>& module_suffixes                   = {},
               const std::map<std::pair<PinDirection, std::string>, std::string>& pin_prefixes = {},
               const std::unordered_set<u32>& group_ids                                        = {}) -> std::optional<std::unordered_map<u32, Module*>> {
                auto res = self.create_modules(module_suffixes, pin_prefixes, group_ids);
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
            py::arg("module_suffixes") = std::map<const GateType*, std::string>(),
            py::arg("pin_prefixes")    = std::map<std::pair<PinDirection, std::string>, std::string>(),
            py::arg("group_ids")       = std::unordered_set<u32>(),
            R"(
                Create modules for the dataflow analysis result.
                :param dict[hal_py.GateType,str] module_suffixes: The suffixes to use for modules containing only gates of a specific gate type. Defaults to ``"module"`` for mixed and unspecified gate types.
                :param dict[tuple(hal_py.PinDirection,str),str] pin_prefixes: The prefixes to use for the module pins that (within the module) only connect to gate pins of a specific name. Defaults to the gate pin name.
                :param set[int] group_ids: The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
                :returns: A map from group IDs to Modules on success, ``None`` otherwise.
                :rtype: dict[int,hal_py.Module] or None
            )");

        py_dataflow_result.def(
            "create_modules",
            [](const dataflow::Result& self,
               const std::map<GateTypeProperty, std::string>& module_suffixes,
               const std::map<std::pair<PinDirection, std::string>, std::string>& pin_prefixes = {},
               const std::unordered_set<u32>& group_ids                                        = {}) -> std::optional<std::unordered_map<u32, Module*>> {
                auto res = self.create_modules(module_suffixes, pin_prefixes, group_ids);
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
            py::arg("module_suffixes"),
            py::arg("pin_prefixes") = std::map<std::pair<PinDirection, std::string>, std::string>(),
            py::arg("group_ids")    = std::unordered_set<u32>(),
            R"(
                Create modules for the dataflow analysis result.
                :param dict[hal_py.GateTypeProperty,str] module_suffixes: The suffixes to use for modules containing only gates of a specific gate type. All gate types featuring the specified gate type property are considered, but the module must still be pure (i.e., all gates must be of the same type) for the suffix to be used. Defaults to ``"module"`` for mixed and unspecified gate types.
                :param dict[tuple(hal_py.PinDirection,str),str] pin_prefixes: The prefixes to use for the module pins that (within the module) only connect to gate pins of a specific name. Defaults to the gate pin name.
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
                :returns: The ID of the group that all other groups have been merged into on success, ``None`` otherwise.
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
