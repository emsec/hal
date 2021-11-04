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

        py_module.def("get_parent_modules", &Module::get_parent_modules, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
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

        py_module.def("is_parent_module_of", &Module::is_parent_module_of, py::arg("module"), py::arg("recursive") = true, R"(
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

        py_module.def("is_submodule_of", &Module::is_submodule_of, py::arg("module"), py::arg("recursive") = true, R"(
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

        py::class_<Module::Port, RawPtrWrapper<Module::Port>> py_module_port(py_module, "Port", R"(
            A module port is a named entry or exit point of a module.
            It comprises one or more pins, each of them being connected to a net.
            A port always has a direction and may additionally feature a type.
        )");

        py_module_port.def(py::self == py::self, R"(
            Check whether two ports are equal.

            :returns: True if both ports are equal, False otherwise.
            :rtype: bool
        )");

        py_module_port.def(py::self != py::self, R"(
            Check whether two ports are unequal.

            :returns: True if both ports are unequal, False otherwise.
            :rtype: bool
        )");

        py_module_port.def_property_readonly("module", &Module::Port::get_module, R"(
            The module the port is assigned to.
            
            :type: hal_py.Module
        )");

        py_module_port.def("get_module", &Module::Port::get_module, R"(
            Get the module the port is assigned to.

            :returns: The module.
            :rtype: hal_py.Module
        )");

        py_module_port.def_property_readonly("name", &Module::Port::get_name, R"(
            The name of the port.
            
            :type: str
        )");

        py_module_port.def("get_name", &Module::Port::get_name, R"(
            Get the name of the port.

            :returns: The name of the port.
            :rtype: str
        )");

        py_module_port.def_property_readonly("direction", &Module::Port::get_direction, R"(
            The direction of the port.
            
            :type: hal_py.PinDirection
        )");

        py_module_port.def("get_direction", &Module::Port::get_direction, R"(
            Get the direction of the port.

            :returns: The direction of the port.
            :rtype: hal_py.PinDirection
        )");

        py_module_port.def_property_readonly("type", &Module::Port::get_type, R"(
            The type of the port.
            
            :type: hal_py.PinType
        )");

        py_module_port.def("get_type", &Module::Port::get_type, R"(
            Get the type of the port.

            :returns: The type of the port.
            :rtype: hal_py.PinType
        )");

        py_module_port.def("get_pins", &Module::Port::get_pins, R"(
            Get the ordered pins of the port.

            :returns: An ordered list of pins.
            :rtype: list[str]
        )");

        py_module_port.def("get_nets", &Module::Port::get_nets, R"(
            Get the ordered nets of the port.

            :returns: An ordered list of nets.
            :rtype: list[hal_py.Net]
        )");

        py_module_port.def("get_pin", &Module::Port::get_pin, py::arg("net"), R"(
            Get the pin through which the specified net runs.

            :param hal_py.Net net: The net.
            :returns: The pin through which the net runs.
            :rtype: str
        )");

        py_module_port.def("get_net", &Module::Port::get_net, py::arg("pin_name"), R"(
            Get the net that runs through the specified pin.

            :param str pin_name: The name of the pin.
            :returns: The net that runs through the pin.
            :rtype: hal_py.Net
        )");

        py_module_port.def("get_pins_and_nets", &Module::Port::get_pins_and_nets, R"(
            Get the ordered pins and the nets that pass through them.

            :returns: An ordered list of pairs of pins and nets.
            :rtype: list[tuple(str,hal_py.Net)]
        )");

        py_module_port.def("is_multi_bit", &Module::Port::is_multi_bit, R"(
            Check whether the port is a multi-bit port, i.e., contains more than one pin.
            
            ;returns: True if the port is a multi-bit port, False otherwise.
            :rtype: bool
        )");

        py_module_port.def("contains_pin", &Module::Port::contains_pin, py::arg("pin_name"), R"(
            Check whether the port contains the specified pin.

            :param str pin_name: The name of the pin.
            :returns: True if the port contains the pin, False otherwise.
            :rtype: bool
        )");

        py_module_port.def("contains_net", &Module::Port::contains_net, py::arg("net"), R"(
            Check whether the port contains the specified net.

            :param str net: The net.
            :returns: True if the port contains the net, False otherwise.
            :rtype: bool
        )");

        py_module_port.def("move_pin", py::overload_cast<const std::string&, u32>(&Module::Port::move_pin), py::arg("pin_name"), py::arg("new_index"), R"(
            Remove a pin from the port by its name.

            :param str pin_name: The name of the pin.
            :param int new_index: The new position that the pin should be assigned to.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module_port.def("move_pin", py::overload_cast<Net*, u32>(&Module::Port::move_pin), py::arg("net"), py::arg("new_index"), R"(
            Remove a pin from the port by its name.

            :param hal_py.Net net: The net.
            :param int new_index: The new position that the pin should be assigned to.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly("ports", &Module::get_ports, R"(
            All ports of the module.

            :type: list[hal_py.Module.Port]
        )");

        py_module.def("get_ports", &Module::get_ports, py::arg("filter") = nullptr, R"(
            Get all ports of the module.
            The optional filter is evaluated on every port such that the result only contains ports matching the specified condition.

            :param lambda filter: Filter function to be evaluated on each port.
            :returns: A list of ports.
            :rtype: list[hal_py.Module.Port]
        )");

        py_module.def("get_port", py::overload_cast<const std::string&>(&Module::get_port, py::const_), py::arg("port_name"), R"(
            Get the port specified by the given name.
        
            :param str port_name: The name of the port.
            :returns: The port on success, None otherwise.
            :rtype: hal_py.Module.Port or None
        )");

        py_module.def("get_port", py::overload_cast<Net*>(&Module::get_port, py::const_), py::arg("net"), R"(
            Get the port that contains the specified net.
        
            :param hal_py.Net net: The net.
            :returns: The port on success, None otherwise.
            :rtype: hal_py.Module.Port or None
        )");

        py_module.def("get_port_by_pin_name", &Module::get_port_by_pin_name, py::arg("pin_name"), R"(
            Get the port that contains the specified pin.

            :param str pin_name: The name of the pin.
            :returns: The port on success, None otherwise.
            :rtype: hal_py.Module.Port or None
        )");

        py_module.def("set_port_name", &Module::set_port_name, py::arg("port"), py::arg("new_name"), R"(
            Set the name of the given port.
            For single-bit ports, the pin name is updated as well.

            :param hal_py.Module.Port port: The port.
            :param str new_name: The name to be assigned to the port.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("set_port_type", &Module::set_port_type, py::arg("port"), py::arg("new_type"), R"(
            Set the type of the given port.

            :param hal_py.Module.Port port: The port.
            :param hal_py.PinType new_type: The type to be assigned to the port.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def(
            "set_port_pin_name", py::overload_cast<Module::Port*, const std::string&, const std::string&>(&Module::set_port_pin_name), py::arg("port"), py::arg("old_name"), py::arg("new_name"), R"(
            Set the name of a pin within a port.

            :param hal_py.Module.Port port: The port that contains the pin.
            :param str old_name: The old name of the pin.
            :param str new_name: The new name of the pin.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("set_port_pin_name", py::overload_cast<Module::Port*, Net*, const std::string&>(&Module::set_port_pin_name), py::arg("port"), py::arg("net"), py::arg("new_name"), R"(
            Set the name of a pin within a port.

            :param hal_py.Module.Port port: The port that contains the pin.
            :param str net: The net that passes through the pin.
            :param str new_name: The new name of the pin.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_module.def("create_multi_bit_port", &Module::create_multi_bit_port, py::arg("name"), py::arg("ports_to_merge"), R"(
            Merge multiple existing ports into a single multi-bit port.
         
            :param str name: The name of the new port.
            :param list[hal_py.Module.Port] ports_to_merge: The ports to be merged in the order in which they should be assigned to the new port.
            :returns: The port on success and None otherwise.
            :rtype: hal_py.Module.Port or None
        )");

        py_module.def("delete_multi_bit_port", &Module::delete_multi_bit_port, py::arg("port"), R"(
            Split a multi-bit port into multiple single-bit ports.
         
            :param hal_py.Module.Port port: The port to be split.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
