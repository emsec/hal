#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void module_init(py::module& m)
    {
        py::class_<Module, DataContainer, RawPtrWrapper<Module>> py_module(m, "Module", R"(
            A module is a container for gates and their associated nets that enables hierarchization within the netlist.
            Each gate can only be in one module at a time. Nets are only loosely associated with modules.
        )");

        py_module.def(py::self == py::self, R"(
            Check whether two modules are equal.
            Does not check for parent module.

            :returns: True if both modules are equal, False otherwise.
            :rtype: bool
        )");

        py_module.def(py::self != py::self, R"(
            Check whether two modules are unequal.
            Does not check for parent module.

            :returns: True if both modules are unequal, False otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly("id", &Module::get_id, R"(
            The unique ID of the module.

            :type: int
        )");

        py_module.def("get_id", &Module::get_id, R"(
            Get the unique ID of the module.

            :returns: The unique id.
            :rtype: int
        )");

        py_module.def_property("name", &Module::get_name, &Module::set_name, R"(
            The name of the module.

            :type: str
        )");

        py_module.def("get_name", &Module::get_name, R"(
            Get the name of the module.

            :returns: The name.
            :rtype: str
        )");

        py_module.def("set_name", &Module::set_name, py::arg("name"), R"(
            Set the name of the module.

            :param str name: The new name.
        )");

        py_module.def_property("type", &Module::get_type, &Module::set_type, R"(
            The type of the module.

            :type: str
        )");

        py_module.def("get_type", &Module::get_type, R"(
            Get the type of the module.

            :returns: The type.
            :rtype: str
        )");

        py_module.def("set_type", &Module::set_type, py::arg("type"), R"(
            Set the type of the module.

            :param str type: The new type.
        )");

        py_module.def("get_grouping", &Module::get_grouping, R"(
            Get the grouping in which this module is contained.

            :returns: The grouping.
            :rtype: hal_py.Grouping
        )");

        py_module.def_property("parent_module", &Module::get_parent_module, &Module::set_parent_module, R"(
            The parent module of this module.
            Is set to None for the top module, but cannot be set to None by the user.

            :type: hal_py.Module or None
        )");

        py_module.def("get_parent_module", &Module::get_parent_module, R"(
            Get the parent module of this module.
            For the top module, None is returned.

            :returns: The parent module.
            :rtype: hal_py.Module or None
        )");

        py_module.def_property_readonly(
            "parent_modules", [](Module* mod) { return mod->get_parent_modules(); }, R"(
            The parent modules of this module.

            :type: list[hal_py.Module]
        )");

        py_module.def("get_parent_modules", &Module::get_parent_modules, py::arg("filter") = nullptr, py::arg("recursive") = true, R"(
            Get all direct parent of this module.
            If recursive is set to true, all indirect parents are also included.
            A filter can be applied to the result to only get parents matching the specified condition.

            :param lambda filter: Filter to be applied to the modules.
            :param bool recursive: True to include indirect parents as well, False otherwise.
            :returns: A list of parent modules.
            :rtype: list[hal_py.Module]
        )");

        py_module.def("set_parent_module", &Module::set_parent_module, py::arg("new_parent"), R"(
            Set a new parent for this module.
            If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.

            :param hal_py.Module new_parent: The new parent module.
            :returns: True if the parent was changed, false otherwise.
            :rtype: bool
        )");

        py_module.def("is_parent_module_of", &Module::is_parent_module_of, py::arg("module"), py::arg("recursive") = false, R"(
            Check if the module is a parent of the specified module.
         
            :param hal_py.Module module: The module.
            :param bool recursive: True to check recursively, False otherwise.
            :returns: True if the module is a parent of the specified module, False otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly(
            "submodules", [](Module* mod) { return mod->get_submodules(); }, R"(
            A list of all direct submodules of this module.

            :type: list[hal_py.Module]
        )");

        py_module.def("get_submodules", &Module::get_submodules, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Get all direct submodules of this module.
            If recursive is set to true, all indirect submodules are also included.
            A filter can be applied to the result to only get submodules matching the specified condition.

            :param lambda filter: Filter to be applied to the modules.
            :param bool recursive: True to include indirect submodules as well, false otherwise.
            :returns: A list of submodules.
            :rtype: list[hal_py.Module]
        )");

        py_module.def("is_submodule_of", &Module::is_submodule_of, py::arg("module"), py::arg("recursive") = false, R"(
            Check if the module is a submodule of the specified module.

            :param hal_py.Module module: The module.
            :param bool recursive: True to check recursively, False otherwise.
            :returns: True if the module is a submodule of the specified module, False otherwise.
        )");

        py_module.def("contains_module", &Module::contains_module, py::arg("other"), py::arg("recusive") = false, R"(
            Checks whether another module is a submodule of this module.
            If recursive is set to true, all indirect submodules are also included.

            :param other: Other module to check for.
            :param recursive: True to include indirect submodules as well.
            :type other: hal_py.Module
            :type recursive: bool
            :returns: True if the other module is a submodule, false otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly("top_module", &Module::is_top_module, R"(
            True only if the module is the top module of the netlist.
        
            :type: bool
        )");

        py_module.def("is_top_module", &Module::is_top_module, R"(
            Returns true only if the module is the top module of the netlist.

            :returns: True if the module is the top module, False otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly(
            "netlist", [](Module* module) { return RawPtrWrapper<Netlist>(module->get_netlist()); }, R"(
            The netlist this module is associated with.

            :type: hal_py.Netlist
        )");

        py_module.def(
            "get_netlist", [](Module* module) { return RawPtrWrapper<Netlist>(module->get_netlist()); }, R"(
            Get the netlist this module is associated with.

            :returns: The netlist.
            :rtype: hal_py.Netlist
        )");

        py_module.def("contains_net", &Module::contains_net, py::arg("net"), py::arg("recursive") = false, R"(
            Check whether a net is contained in the module.
            If recursive is set to true, nets in submodules are considered as well.
        
            :param hal_py.Net net: The net to check for.
            :param bool recursive: True to also consider nets in submodules, false otherwise.
            :returns: True if the net is contained in the module, False otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly(
            "nets", [](Module* mod) { return mod->get_nets(); }, R"(
            A list of all nets that have at least one source or one destination within the module.

            :type: list[hal_py.Net]
        )");

        py_module.def("get_nets", &Module::get_nets, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Get all nets that have at least one source or one destination within the module.
            A filter can be applied to the result to only get nets matching the specified condition.
            If recursive is True, nets in submodules are considered as well.

            :param lambda filter: Filter to be applied to the nets.
            :param bool recursive: True to also consider nets in submodules, False otherwise.
            :returns: A list of nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def_property_readonly("input_nets", &Module::get_input_nets, R"(
            A list of all nets that are either a global input to the netlist or have at least one source outside of the module.

            :type: list[hal_py.Net]
        )");

        py_module.def("get_input_nets", &Module::get_input_nets, R"(
            Get all nets that are either a global input to the netlist or have at least one source outside of the module.

            :returns: A list of input nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def_property_readonly("output_nets", &Module::get_output_nets, R"(
            A list of all nets that are either a global output to the netlist or have at least one destination outside of the module.

            :type: set[hal_py.Net]
        )");

        py_module.def("get_output_nets", &Module::get_output_nets, R"(
            Get all nets that are either a global output to the netlist or have at least one destination outside of the module.

            :returns: A list of output nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def_property_readonly("internal_nets", &Module::get_internal_nets, R"(
            A list of all nets that have at least one source and one destination within the module, including its submodules. The result may contain nets that are also regarded as input or output nets.

            :type: list[hal_py.Net]
        )");

        py_module.def("get_internal_nets", &Module::get_internal_nets, R"(
            Get all nets that have at least one source and one destination within the module, including its submodules. The result may contain nets that are also regarded as input or output nets.

            :returns: A list of internal nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def("is_input_net", &Module::is_input_net, py::arg("net"), R"(
            Check whether the given net is an input of the module, i.e., whether the net is a global input to the netlist or has at least one source outside of the module.
            
            :param hal_py.Net net: The net.
            :returns: True if the net is an input net, False otherwise.
            :rtype: bool
        )");

        py_module.def("is_output_net", &Module::is_output_net, py::arg("net"), R"(
            Check whether the given net is an output of the module, i.e., whether the net is a global output to the netlist or has at least one destination outside of the module.
            
            :param hal_py.Net net: The net.
            :returns: True if the net is an output net, False otherwise.
            :rtype: bool
        )");

        py_module.def("is_internal_net", &Module::is_internal_net, py::arg("net"), R"(
            Check whether the given net is an internal net of the module, i.e. whether the net has at least one source and one destination within the module.
            
            :param hal_py.Net net: The net.
            :returns: True if the net is an internal net, False otherwise.
            :rtype: bool
        )");

        py_module.def("assign_gate", &Module::assign_gate, py::arg("gate"), R"(
            Assign a gate to the module.
            The gate is removed from its previous module in the process.

            :param hal_py.Gate gate: The gate to assign.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_module.def("assign_gates", &Module::assign_gates, py::arg("gates"), R"(
            Assign a list of gates to the module.
            The gates are removed from their previous module in the process.

            :param list[hal_py.Gate] gates: The gates to assign.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("remove_gate", &Module::remove_gate, py::arg("gate"), R"(
            Remove a gate from the module.
            Automatically moves the gate to the top module of the netlist.

            :param hal_py.Gate gate: The gate to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_module.def("remove_gates", &Module::remove_gates, py::arg("gates"), R"(
            Remove a list of gates from the module.
            Automatically moves the gates to the top module of the netlist.

            :param list[hal_py.Gate] gates: The gates to remove.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("contains_gate", &Module::contains_gate, py::arg("gate"), py::arg("recusive") = false, R"(
            Check whether a gate is contained in the module.
            If recursive is True, gates in submodules are considered as well.

            :param hal_py.Gate gate: The gate to check for.
            :param bool recursive: True to also consider gates in submodules, false otherwise.
            :returns: True if the gate is contained in the module, false otherwise.
            :rtype: bool
        )");

        py_module.def("get_gate_by_id", &Module::get_gate_by_id, py::arg("id"), py::arg("recursive") = false, R"(
            Get a gate specified by the given ID.
            If recursive is True, gates in submodules are considered as well.

            :param int id: The unique ID of the gate.
            :param bool recursive: True to also consider gates in submodules, false otherwise.
            :returns: The gate if found, None otherwise.
            :rtype: hal_py.Gate or None
        )");

        py_module.def_property_readonly(
            "gates", [](Module* mod) { return mod->get_gates(); }, R"(
            The list of all gates contained within the module.

            :type: list[hal_py.Gate]
        )");

        py_module.def("get_gates", &Module::get_gates, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Get all gates contained within the module.<br>
            A filter can be applied to the result to only get gates matching the specified condition.<br>
            If recursive is True, gates in submodules are considered as well.

            :param lambda filter: Filter to be applied to the gates.
            :param bool recursive: True to also consider gates in submodules, false otherwise.
            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_module.def_property_readonly("pins", &Module::get_pins, R"(
            All pins of the module.

            :type: list[hal_py.ModulePin]
        )");

        py_module.def("get_pins", &Module::get_pins, py::arg("filter") = nullptr, R"(
            Get all pins of the module.
            The optional filter is evaluated on every pin such that the result only contains pins matching the specified condition.

            :param lambda filter: Filter function to be evaluated on each pin.
            :returns: A list of pins.
            :rtype: list[hal_py.ModulePin]
        )");

        py_module.def_property_readonly("pin_groups", &Module::get_pin_groups, R"(
            All pin_groups of the module.

            :type: list[hal_py.ModulePinGroup]
        )");

        py_module.def("get_pin_groups", &Module::get_pin_groups, py::arg("filter") = nullptr, R"(
            Get all pin groups of the module.
            The optional filter is evaluated on every pin group such that the result only contains pin groups matching the specified condition.

            :param lambda filter: Filter function to be evaluated on each pin group.
            :returns: A list of pin groups.
            :rtype: list[hal_py.ModulePinGroup]
        )");

        py_module.def("get_pin", py::overload_cast<const std::string&>(&Module::get_pin, py::const_), py::arg("name"), R"(
            Get the pin specified by the given name.

            :param str name: The name of the pin.
            :returns: The pin on success, None otherwise.
            :rtype: hal_py.ModulePin or None
        )");

        py_module.def("get_pin", py::overload_cast<Net*>(&Module::get_pin, py::const_), py::arg("net"), R"(
            Get the pin that passes through the specified net.

            :param hal_py.Net net: The net.
            :returns: The pin on success, None otherwise.
            :rtype: hal_py.ModulePin or None
        )");

        py_module.def("get_pin_group", &Module::get_pin_group, py::arg("name"), R"(
            Get the pin group specified by the given name.

            :param str name: The name of the pin group.
            :returns: The pin group on success, None otherwise.
            :rtype: hal_py.ModulePinGroup or None
        )");

        py_module.def("set_pin_name", &Module::set_pin_name, py::arg("pin"), py::arg("new_name"), R"(
            Set the name of the given pin.

            :param hal_py.ModulePin pin: The pin.
            :param str new_name: The name to be assigned to the pin.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("set_pin_group_name", &Module::set_pin_group_name, py::arg("pin_group"), py::arg("new_name"), R"(
            Set the name of the given pin group.

            :param hal_py.ModulePinGroup pin_group: The pin group.
            :param str new_name: The name to be assigned to the pin group.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("set_pin_type", &Module::set_pin_type, py::arg("pin"), py::arg("new_type"), R"(
            Set the type of the given port.

            :param hal_py.ModulePin pin: The pin.
            :param hal_py.PinType new_type: The type to be assigned to the pin.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("create_pin_group", &Module::create_pin_group, py::arg("name"), py::arg("pins"), py::arg("ascending") = false, py::arg("start_index") = 0, R"(
            Create a new pin group with the given name.
            All pins to be added to the pin group must have the same direction and type.

            :param str name: The name of the pin group.
            :param list[hal_py.ModulePin] pins: The pins to be assigned to the pin group.
            :returns: The pin group on success, a nullptr otherwise.
            :rtype: hal_py.ModulePinGroup
        )");

        py_module.def("delete_pin_group", &Module::delete_pin_group, py::arg("pin_group"), R"(
            Delete the given pin group.

            :param hal_py.ModulePinGroup pin_group: The pin group to be deleted.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("assign_pin_to_group", &Module::assign_pin_to_group, py::arg("pin_group"), py::arg("pin"), R"(
            Assign a pin to a pin group.
            Only pins with matching direction and type can be assigned to an existing pin group.

            :param hal_py.ModulePinGroup pin_group: The pin group.
            :param hal_py.ModulePin pin: The pin to be added.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
