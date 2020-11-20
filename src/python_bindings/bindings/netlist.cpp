#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_init(py::module& m)
    {
        py::class_<Netlist, std::shared_ptr<Netlist>> py_netlist(m, "Netlist", R"(
            Netlist class containing information about the netlist including its gates, modules, nets, and groupings as well as the underlying gate library.
        )");

        py_netlist.def(py::init<GateLibrary*>(), py::arg("gate_library"), R"(
            Construct a new netlist for the specified gate library.

            Warning: Use the netlist_factory to create instances!

            :param hal_py.GateLibrary gate_library: The gate library.
        )");

        py_netlist.def_property("id", &Netlist::get_id, &Netlist::set_id, R"(
            The ID of the netlist.
            If not explicitly set, the ID defaults to 0.

            :type: int
        )");

        py_netlist.def("get_id", &Netlist::get_id, R"(
            Get the ID of the netlist.
            If not explicitly set, the ID defaults to 0.

            :returns: The ID of the netlist.
            :rtype: int
        )");

        py_netlist.def("set_id", &Netlist::set_id, py::arg("id"), R"(
            Set the ID of the netlist to the specified value.

            :param int id: The new ID of the netlist.
        )");

        py_netlist.def_property("input_filename", &Netlist::get_input_filename, &Netlist::set_input_filename, R"(
            The path to the input file.

            :type: str
        )");

        py_netlist.def("get_input_filename", &Netlist::get_input_filename, R"(
            Get the path to the input file.

            :returns: The path to the input file.
            :rtype: hal_py.hal_path
        )");

        py_netlist.def("set_input_filename", &Netlist::set_input_filename, py::arg("path"), R"(
            Set the path to the input file.

            :param hal_py.hal_path filename: The path to the input file.
        )");

        py_netlist.def_property("design_name", &Netlist::get_design_name, &Netlist::set_design_name, R"(
            The name of the design.

            :type: str
        )");

        py_netlist.def("get_design_name", &Netlist::get_design_name, R"(
            Get the name of the design.

            :returns: The name of the design.
            :rtype: str
        )");

        py_netlist.def("set_design_name", &Netlist::set_design_name, py::arg("design_name"), R"(
            Set the name of the design.

            :param str design_name: The new name of the design.
        )");

        py_netlist.def_property("device_name", &Netlist::get_device_name, &Netlist::set_device_name, R"(
            The name of the target device.

            :type: str
        )");

        py_netlist.def("get_device_name", &Netlist::get_device_name, R"(
            Get the name of the target device.

            :returns: The name of the target device.
            :rtype: str
        )");

        py_netlist.def("set_device_name", &Netlist::set_device_name, py::arg("device_name"), R"(
            Set the name of the target device.

            :param str divice_name: The name of the target device.
        )");

        py_netlist.def_property_readonly(
            "GateLibrary", [](Netlist* nl) { return RawPtrWrapper<const GateLibrary>(nl->get_gate_library()); }, R"(
            The gate library associated with the netlist.

            :type: hal_py.GateLibrary
        )");

        py_netlist.def(
            "get_gate_library", [](Netlist* nl) { return RawPtrWrapper<const GateLibrary>(nl->get_gate_library()); }, R"(
            Get the gate library associated with the netlist.

            :returns: The gate library.
            :rtype: hal_py.GateLibrary
        )");

        py_netlist.def("clear_caches", &Netlist::clear_caches, R"(
            Clear all internal caches of the netlist.
            In a typical application, calling this function is not required.
        )");

        py_netlist.def("get_unique_gate_id", &Netlist::get_unique_gate_id, R"(
            Get a spare gate ID.
            The value of 0 is reserved and represents an invalid ID.

            :returns: The gate ID.
            :rtype: int
        )");

        py_netlist.def("create_gate",
                       py::overload_cast<u32, const GateType*, const std::string&, float, float>(&Netlist::create_gate),
                       py::arg("gate_id"),
                       py::arg("gate_type"),
                       py::arg("name"),
                       py::arg("x") = -1,
                       py::arg("y") = -1,
                       R"(
            Create a new gate and add it to the netlist.

            :param int gate_id: The unique ID of the gate.
            :param hal_py.Gate_type gate_type: The gate type.
            :param str name: The name of the gate.
            :param float x: The x-coordinate of the gate.
            :param float y: The y-coordinate of the gate.
            :returns: The new gate on success, None otherwise.
            :rtype: hal_py.Gate or None
        )");

        py_netlist.def("create_gate",
                       py::overload_cast<const GateType*, const std::string&, float, float>(&Netlist::create_gate),
                       py::arg("gate_type"),
                       py::arg("name"),
                       py::arg("x") = -1,
                       py::arg("y") = -1,
                       R"(
            Create a new gate and add it to the netlist.
            The ID of the gate is set automatically.

            :param hal_py.Gate_type gate_type: The gate type.
            :param str name: The name of the gate.
            :param float x: The x-coordinate of the gate.
            :param float y: The y-coordinate of the gate.
            :returns: The new gate on success, None otherwise.
            :rtype: hal_py.Gate or None
        )");

        py_netlist.def("delete_gate", &Netlist::delete_gate, py::arg("gate"), R"(
            Remove a gate from the netlist.

            :param gate: The gate.
            :type gate: hal_py.Gate
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_gate_in_netlist", &Netlist::is_gate_in_netlist, py::arg("gate"), R"(
            Check whether the gate is registered in the netlist.

            :param gate: The gate to check.
            :type gate: hal_py.Gate
            :returns: True if the gate is in the netlist, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("get_gate_by_id", &Netlist::get_gate_by_id, py::arg("gate_id"), R"(
            Get the gate specified by the given ID.

            :param int gate_id: The unique ID of the gate.
            :returns: The gate on success, None otherwise.
            :rtype: hal_py.Gate or None
        )");

        py_netlist.def_property_readonly(
            "gates", [](Netlist* n) { return n->get_gates(); }, R"(
            All gates contained within the netlist.

            :type: list[hal_py.Gate]
        )");

        py_netlist.def("get_gates", &Netlist::get_gates, py::arg("filter") = nullptr, R"(
            Get all gates contained within the netlist.
            A filter can be applied to the result to only get gates matching the specified condition.

            :param lambda filter: Filter to be applied to the gates.
            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist.def("mark_vcc_gate", &Netlist::mark_vcc_gate, py::arg("gate"), R"(
            Mark a gate as global VCC gate.

            :param hal_py.Gate gate: The gate.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("mark_gnd_gate", &Netlist::mark_gnd_gate, py::arg("gate"), R"(
            Mark a gate as global GND gate.

            :param hal_py.Gate gate: The gate.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("unmark_vcc_gate", &Netlist::unmark_vcc_gate, py::arg("gate"), R"(
            Unmark a global VCC gate.

            :param hal_py.Gate gate: The gate.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("unmark_gnd_gate", &Netlist::unmark_gnd_gate, py::arg("gate"), R"(
            Unmark a global GND gate.

            :param hal_py.Gate gate: The gate.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_vcc_gate", &Netlist::is_vcc_gate, py::arg("gate"), R"(
            Check whether a gate is a global VCC gate.

            :param gate: The gate to check.
            :type gate: hal_py.Gate
            :returns: True if the gate is a global VCC gate, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_gnd_gate", &Netlist::is_gnd_gate, py::arg("gate"), R"(
            Check whether a gate is a global GND gate.

            :param gate: The gate to check.
            :type gate: hal_py.Gate
            :returns: True if the gate is a global GND gate, false otherwise.
            :rtype: bool
        )");

        py_netlist.def_property_readonly("vcc_gates", &Netlist::get_vcc_gates, R"(
            All global VCC gates.

            :type: list[hal_py.Gate]
        )");

        py_netlist.def("get_vcc_gates", &Netlist::get_vcc_gates, R"(
            Get all global VCC gates.

            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist.def_property_readonly("gnd_gates", &Netlist::get_gnd_gates, R"(
            All global GND gates.

            :type: list[hal_py.Gate]
        )");

        py_netlist.def("get_gnd_gates", &Netlist::get_gnd_gates, R"(
            Get all global GND gates.

            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist.def("get_unique_net_id", &Netlist::get_unique_net_id, R"(
            Get a spare net ID.
            The value of 0 is reserved and represents an invalid ID.

            :returns: The net ID.
            :rtype: int
        )");

        py_netlist.def("create_net", py::overload_cast<const u32, const std::string&>(&Netlist::create_net), py::arg("net_id"), py::arg("name"), R"(
            Create a new net and add it to the netlist.

            :param int net_id: The unique ID of the net.
            :param str name: The name of the net.
            :returns: The new net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");

        py_netlist.def("create_net", py::overload_cast<const std::string&>(&Netlist::create_net), py::arg("name"), R"(
            Create a new net and add it to the netlist.
            The ID of the net is set automatically.

            :param str name: The name of the net.
            :returns: The new net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");

        py_netlist.def("delete_net", &Netlist::delete_net, py::arg("net"), R"(
            Removes a net from the netlist.

            :param hal_py.Net net: The net.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_net_in_netlist", &Netlist::is_net_in_netlist, py::arg("net"), R"(
            Check whether a net is registered in the netlist.

            :param hal_py.Net net: The net to check.
            :returns: True if the net is in the netlist, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("get_net_by_id", &Netlist::get_net_by_id, py::arg("net_id"), R"(
            Get the net specified by the given ID.

            :param int net_id: The unique ID of the net.
            :returns: The net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");

        py_netlist.def_property_readonly(
            "nets", [](Netlist* n) { return n->get_nets(); }, R"(
            All nets contained within the netlist.

            :type: list[hal_py.Net]
        )");

        py_netlist.def("get_nets", &Netlist::get_nets, py::arg("filter") = nullptr, R"(
            Get all nets contained within the netlist.
            A filter can be applied to the result to only get nets matching the specified condition.

            :param lambda filter: Filter to be applied to the nets.
            :returns: A list of nets.
            :rtype: list[hal_py.Net]
        )");

        py_netlist.def("mark_global_input_net", &Netlist::mark_global_input_net, py::arg("net"), R"(
            Mark a net as a global input net.

            :param hal_py.Net net: The net.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("mark_global_output_net", &Netlist::mark_global_output_net, py::arg("net"), R"(
            Mark a net as a global output net.

            :param hal_py.Net net: The net.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("unmark_global_input_net", &Netlist::unmark_global_input_net, py::arg("net"), R"(
            Unmark a global input net.

            :param hal_py.Net net: The net.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("unmark_global_output_net", &Netlist::unmark_global_output_net, py::arg("net"), R"(
            Unmark a global output net.

            :param hal_py.Net net: The net.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_global_input_net", &Netlist::is_global_input_net, py::arg("net"), R"(
            Check whether a net is a global input net.

            :param hal_py.Net net: The net to check.
            :returns: True if the net is a global input net, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_global_output_net", &Netlist::is_global_output_net, py::arg("net"), R"(
            Check whether a net is a global output net.

            :param hal_py.Net net: The net to check.
            :returns: True if the net is a global output net, false otherwise.
            :rtype: bool
        )");

        py_netlist.def_property_readonly("global_input_nets", &Netlist::get_global_input_nets, R"(
            All global input nets.

            :type: list[hal_py.Net]
        )");

        py_netlist.def("get_global_input_nets", &Netlist::get_global_input_nets, R"(
            Get all global input nets.

            :returns: A list of nets.
            :rtype: list[hal_py.Net]
        )");

        py_netlist.def_property_readonly("global_output_nets", &Netlist::get_global_output_nets, R"(
            All global output nets.

            :type: list[hal_py.Net]
        )");

        py_netlist.def("get_global_output_nets", &Netlist::get_global_output_nets, R"(
            Get all global output nets.

            :returns: A list of nets.
            :rtype: list[hal_py.Net]
        )");

        py_netlist.def("get_unique_module_id", &Netlist::get_unique_module_id, R"(
            Get a spare module ID.
            The value of 0 is reserved and represents an invalid ID.

            :returns: The module ID.
            :rtype: int
        )");

        py_netlist.def("create_module",
                       py::overload_cast<const u32, const std::string&, Module*, const std::vector<Gate*>&>(&Netlist::create_module),
                       py::arg("module_id"),
                       py::arg("name"),
                       py::arg("parent"),
                       py::arg("gates") = std::vector<Gate*>(),
                       R"(
            Create a new module and add it to the netlist.

            :param int module_id: The unique ID of the module.
            :param str name: The name of the module.
            :param hal_py.Module parent: The parent module.
            :param list gates: Gates to assign to the new module.
            :returns: The new module on succes, None on error.
            :rtype: hal_py.Module or None
        )");

        py_netlist.def("create_module",
                       py::overload_cast<const std::string&, Module*, const std::vector<Gate*>&>(&Netlist::create_module),
                       py::arg("name"),
                       py::arg("parent"),
                       py::arg("gates") = std::vector<Gate*>(),
                       R"(
            Create a new module and add it to the netlist.
            The ID of the module is set automatically.

            :param str name: The name of the module.
            :param hal_py.Module parent: The parent module.
            :param list gates: Gates to assign to the new module.
            :returns: The new module on succes, None on error.
            :rtype: hal_py.Module or None
        )");

        py_netlist.def("delete_module", &Netlist::delete_module, py::arg("module"), R"(
            Remove a module from the netlist.

            :param module: The module.
            :type module: hal_py.Module
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_module_in_netlist", &Netlist::is_module_in_netlist, py::arg("module"), R"(
            Check whether a module is registered in the netlist.

            :param hal_py.Module module: The module to check.
            :returns: True if the module is in the netlist, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("get_module_by_id", &Netlist::get_module_by_id, py::arg("module_id"), R"(
            Get the module specified by the given ID.

            :param int module_id: The unique ID of the module.
            :returns: The module on success, None otherwise.
            :rtype: hal_py.Module
        )");

        py_netlist.def_property_readonly("modules", &Netlist::get_modules, R"(
            All modules contained within the netlist including the top module.

            :rtype: list[hal_py.Module]
        )");

        py_netlist.def("get_modules", &Netlist::get_modules, R"(
            Get all modules contained within the netlist including the top module.

            :returns: A list of modules.
            :rtype: list[hal_py.Module]
        )");

        py_netlist.def_property_readonly("top_module", &Netlist::get_top_module, R"(
            The top module of the netlist.

            :type: hal_py.Module
        )");

        py_netlist.def("get_top_module", &Netlist::get_top_module, R"(
            Get the top module of the netlist.

            :returns: The top module.
            :rtype: hal_py.Module
        )");

        py_netlist.def("get_unique_grouping_id", &Netlist::get_unique_grouping_id, R"(
            Get a spare and unique grouping ID.
            The value of 0 is reserved and represents an invalid ID.

            :returns: The grouping ID.
            :rtype: int
        )");

        py_netlist.def("create_grouping",
                       py::overload_cast<const u32, const std::string&>(&Netlist::create_grouping),
                       py::arg("grouping_id"),
                       py::arg("name"),
                       R"(
            Create a new grouping and add it to the netlist.

            :param int grouping_id: The unique ID of the grouping.
            :param str name: The name of the grouping.
            :returns: The new grouping on success, None otherwise.
            :rtype: hal_py.Grouping or None
        )");

        py_netlist.def("create_grouping",
                       py::overload_cast<const std::string&>(&Netlist::create_grouping),
                       py::arg("name"),
                       R"(
            Create a new grouping and add it to the netlist.
            The ID of the grouping is set automatically.

            :param str name: The name of the grouping.
            :returns: The new grouping on success, None otherwise.
            :rtype: hal_py.Grouping or None
        )");

        py_netlist.def("delete_grouping", &Netlist::delete_grouping, py::arg("grouping"), R"(
            Remove a grouping from the netlist.

            :param hal_py.Grouping grouping: The grouping.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("is_grouping_in_netlist", &Netlist::is_grouping_in_netlist, py::arg("grouping"), R"(
            Check whether the grouping is registered in the netlist.

            :param hal_py.Module grouping: The grouping to check.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist.def("get_grouping_by_id", &Netlist::get_grouping_by_id, py::arg("grouping_id"), R"(
            Get the grouping specified by the given ID.

            :param int grouping_id: The unique ID of the grouping.
            :returns: The grouping on success, nullptr otherwise.
            :rtype: hal_py.Grouping
        )");

        py_netlist.def_property_readonly(
            "groupings", [](Netlist* n) { return n->get_groupings(); }, R"(
            All groupings contained within the netlist.

            :rtype: list[hal_py.Grouping]
        )");

        py_netlist.def("get_groupings", &Netlist::get_groupings, py::arg("filter") = nullptr, R"(
            Get all groupings contained within the netlist.
            A filter can be applied to the result to only get groupings matching the specified condition.

            :param lambda filter: Filter to be applied to the groupings.
            :returns: A list of groupings.
            :rtype: list[hal_py.Grouping]
        )");

        py_netlist.def("get_next_gate_id", &Netlist::get_next_gate_id, R"(
            Get the gate ID following the highest currently used ID.

            :returns: The next gate ID.
            :rtype: int
        )");

        py_netlist.def("set_next_gate_id", &Netlist::set_next_gate_id, py::arg("id"), R"(
            Set the gate ID following the highest currently used ID.

            :param int id: The next gate ID.
        )");

        py_netlist.def("get_used_gate_ids", &Netlist::get_used_gate_ids, R"(
            Get a set of all currently used gate IDs.

            :returns: All used gate IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_used_gate_ids", &Netlist::set_used_gate_ids, py::arg("ids"), R"(
            Set a set of all currently used gate IDs.

            :param set[int] ids: All used gate IDs.
        )");

        py_netlist.def("get_free_gate_ids", &Netlist::get_free_gate_ids, R"(
            Get a set of all gate IDs that have previously been used but been freed ever since.

            :returns: All freed gate IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_free_gate_ids", &Netlist::set_free_gate_ids, py::arg("ids"), R"(
            Set a set of all gate IDs that have previously been used but been freed ever since.

            :param set[int] ids: All freed gate IDs.
        )");

        py_netlist.def("get_next_net_id", &Netlist::get_next_net_id, R"(
            Get the net ID following the highest currently used ID.

            :returns: The next net ID.
            :rtype: int
        )");

        py_netlist.def("set_next_net_id", &Netlist::set_next_net_id, py::arg("id"), R"(
            Set the net ID following the highest currently used ID.

            :param int id: The next net ID.
        )");

        py_netlist.def("get_used_net_ids", &Netlist::get_used_net_ids, R"(
            Get a set of all currently used net IDs.

            :returns: All used net IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_used_net_ids", &Netlist::set_used_net_ids, py::arg("ids"), R"(
            Set a set of all currently used net IDs.

            :param set[int] ids: All used net IDs.
        )");

        py_netlist.def("get_free_net_ids", &Netlist::get_free_net_ids, R"(
            Get a set of all net IDs that have previously been used but been freed ever since.

            :returns: All freed net IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_free_net_ids", &Netlist::set_free_net_ids, py::arg("ids"), R"(
            Set a set of all net IDs that have previously been used but been freed ever since.

            :param set[int] ids: All freed net IDs.
        )");

        py_netlist.def("get_next_module_id", &Netlist::get_next_module_id, R"(
            Get the module ID following the highest currently used ID.

            :returns: The next module ID.
            :rtype: int
        )");

        py_netlist.def("set_next_module_id", &Netlist::set_next_module_id, py::arg("id"), R"(
            Set the module ID following the highest currently used ID.

            :param int id: The next module ID.
        )");

        py_netlist.def("get_used_module_ids", &Netlist::get_used_module_ids, R"(
            Get a set of all currently used module IDs.

            :returns: All used module IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_used_module_ids", &Netlist::set_used_module_ids, py::arg("ids"), R"(
            Set a set of all currently used module IDs.

            :param set[int] ids: All used module IDs.
        )");

        py_netlist.def("get_free_module_ids", &Netlist::get_free_module_ids, R"(
            Get a set of all module IDs that have previously been used but been freed ever since.

            :returns: All freed module IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_free_module_ids", &Netlist::set_free_module_ids, py::arg("ids"), R"(
            Set a set of all module IDs that have previously been used but been freed ever since.

            :param set[int] ids: All freed module IDs.
        )");

        py_netlist.def("get_next_grouping_id", &Netlist::get_next_grouping_id, R"(
            Get the grouping ID following the highest currently used ID.

            :returns: The next grouping ID.
            :rtype: int
        )");

        py_netlist.def("set_next_grouping_id", &Netlist::set_next_grouping_id, py::arg("id"), R"(
            Set the grouping ID following the highest currently used ID.

            :param int id: The next grouping ID.
        )");

        py_netlist.def("get_used_grouping_ids", &Netlist::get_used_grouping_ids, R"(
            Get a set of all currently used grouping IDs.

            :returns: All used grouping IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_used_grouping_ids", &Netlist::set_used_grouping_ids, py::arg("ids"), R"(
            Set a set of all currently used grouping IDs.

            :param set[int] ids: All used grouping IDs.
        )");

        py_netlist.def("get_free_grouping_ids", &Netlist::get_free_grouping_ids, R"(
            Get a set of all grouping IDs that have previously been used but been freed ever since.

            :returns: All freed grouping IDs.
            :rtype: set[int]
        )");

        py_netlist.def("set_free_grouping_ids", &Netlist::set_free_grouping_ids, py::arg("ids"), R"(
            Set a set of all grouping IDs that have previously been used but been freed ever since.

            :param set[int] ids: All freed grouping IDs.
        )");
    }
}    // namespace hal
