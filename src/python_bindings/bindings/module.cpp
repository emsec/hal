#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void module_init(py::module& m)
    {
        py::class_<Module, DataContainer, RawPtrWrapper<Module>> py_module(m, "Module", R"(
            Module class containing information about a module including its gates, submodules, and parent module.
        )");

        py_module.def_property_readonly("id", &Module::get_id, R"(
            The unique ID of the module object.

            :type: int
        )");

        py_module.def("get_id", &Module::get_id, R"(
            Returns the unique ID of the module object.

            :returns: The unique id.
            :rtype: int
        )");

        py_module.def_property("name", &Module::get_name, &Module::set_name, R"(
            The name of the module.

            :type: str
        )");

        py_module.def("get_name", &Module::get_name, R"(
            Gets the module's name.

            :returns: The name.
            :rtype: str
        )");

        py_module.def("set_name", &Module::set_name, py::arg("name"), R"(
            Sets the module's name.

            :param str name: The new name.
        )");

        py_module.def_property("type", &Module::get_type, &Module::set_type, R"(
            The type of the module.

            :type: str
        )");

        py_module.def("get_type", &Module::get_type, R"(
            Gets the module's type.

            :returns: The type.
            :rtype: str
        )");

        py_module.def("set_type", &Module::set_type, py::arg("type"), R"(
            Sets the module's type.

            :param str name: The new type.
        )");

        py_module.def_property("parent_module", &Module::get_parent_module, &Module::set_parent_module, R"(
            The parent module of this module. Set to None for the top module.

            :type: hal_py.Module or None
        )");

        py_module.def("get_parent_module", &Module::get_parent_module, R"(
            Get the parent of this module.
            This returns None for the top module.

            :returns: The parent module.
            :rtype: hal_py.Module or None
        )");

        py_module.def("set_parent_module", &Module::set_parent_module, py::arg("new_parent"), R"(
            Set the parent of this module.
            If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.

            :returns: True if the parent was changed
            :rtype: bool
        )");

        py_module.def_property_readonly("submodules", [](Module* mod) { return mod->get_submodules(); }, R"(
            A set of all direct submodules of this module.

            :type: set[hal_py.Module]
        )");

        py_module.def("get_submodules", &Module::get_submodules, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Get all direct submodules of this module.
            If recursive parameter is true, all indirect submodules are also included.

            :param lambda filter: Filter for the modules.
            :param bool recursive: Look into submodules aswell.
            :returns: The set of submodules:
            :rtype: set[hal_py.Module]
        )");

        py_module.def("contains_module", &Module::contains_module, py::arg("other"), py::arg("recusive") = false, R"(
            Checks whether another module is a submodule of this module. If \p recursive is true, all indirect submodules are also included.

            :param other: Other module to check
            :param recursive: Look into submodules too
            :type other: hal_py.Module
            :type recursive: bool
            :returns: True if the module is a submodule
            :rtype: bool
        )");

        py_module.def_property_readonly("netlist", [](Module* module){return RawPtrWrapper(module->get_netlist());}, R"(
            The netlist this module is associated with.

            :type: hal_py.Netlist
        )");

        py_module.def("get_netlist", [](Module* module){return RawPtrWrapper(module->get_netlist());}, R"(
            Get the netlist this module is associated with.

            :returns: The netlist.
            :rtype: hal_py.Netlist
        )");

        py_module.def_property_readonly("input_nets", &Module::get_input_nets, R"(
            The input nets to this module.

            :type: set[hal_py.Net]
        )");

        py_module.def("get_input_nets", &Module::get_input_nets, R"(
            Get the input nets to this module.
            A module input net is either a global input to the netlist or has a source outside of the module.

            :returns: A set of module input nets.
            :rtype: set[hal_py.Net]
        )");

        py_module.def_property_readonly("output_nets", &Module::get_output_nets, R"(
            The output nets to this module.

            :type: set[hal_py.Net]
        )");

        py_module.def("get_output_nets", &Module::get_output_nets, R"(
            Get the output nets to this module.     
            A module output net is either a global output of the netlist or has a destination outside of the module.

            :returns: The set of module output nets.
            :rtype: set[hal_py.Net]
        )");

        py_module.def_property_readonly("internal_nets", &Module::get_internal_nets, R"(
            The internal nets to this module.

            :type: set[hal_py.Net]
        )");

        py_module.def("get_internal_nets", &Module::get_internal_nets, R"(
            Get the internal nets to this module. A net is internal if its source and at least one output are inside the module.
            Therefore it may contain some nets that are also regarded as output nets.

            :returns: The set of internal nets.
            :rtype: set[hal_py.Net]
        )");

        py_module.def_property_readonly("gates", [](Module* mod) { return mod->get_gates(); }, R"(
            The set of all gates belonging to the module.

            :type: set[hal_py.Gate]
        )");

        py_module.def("get_gates", &Module::get_gates, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Returns all associated gates. You can filter with the optional parameters. If the parameter recursive is true, all submodules are searched aswell.

            :param lambda filter: Filter for the gates.
            :param bool recursive: Look into submodules too.
            :returns: A set of gates.
            :rtype: set[hal_py.Gate]
        )");

        py_module.def("get_gate_by_id", &Module::get_gate_by_id, py::arg("id"), py::arg("recursive") = false, R"(
            Get a gate specified by id. If recursive parameter is true, all submodule are searched aswell.

            :param int id: The gate's id.
            :param bool recursive: Look into submodules too.
            :returns: The gate or None.
            :rtype: hal_py.Gate or None
        )");

        py_module.def("assign_gate", &Module::assign_gate, py::arg("gate"), R"(
            Moves a gate into this module. The gate is removed from its previous module in the process.

            :param hal_py.Gate gate: The gate to add.
            :returns: True on success.
            :rtype: bool
        )");

        py_module.def("remove_gate", &Module::remove_gate, py::arg("gate"), R"(
            Removes a gate from the module object.

            :param hal_py.Gate gate: The gate to remove.
            :returns: True on success.
            :rtype: bool
        )");

        py_module.def("contains_gate", &Module::contains_gate, py::arg("gate"), py::arg("recusive") = false, R"(
            Checks whether a gate is in the module. If \p recursive is true, all submodules are searched as well.

            :param hal_py.Gate gate: The gate to search for.
            :param bool recursive: Look into submodules too
            :returns: True if the gate is in the object.
            :rtype: bool
        )");

        py_module.def("set_input_port_name", &Module::set_input_port_name, py::arg("input_net"), py::arg("port_name"), R"(
            Set the name of the port corresponding to the specified input net to the given string.

            :param hal_py.Net input_net: The input net.
            :param str port_name: The port name.
        )");

        py_module.def("get_input_port_name", &Module::get_input_port_name, py::arg("input_net"), R"(
            Get the name of the port corresponding to the specified input net.

            :param hal_py.Net input_net: The input net.
            :returns: The port name.
            :rtype: str
        )");

        py_module.def_property_readonly("input_port_names", &Module::get_input_port_names, R"(
            Get the mapping of all input nets to their corresponding port names.

            :type: dict[hal_py.Net,str]
        )");

        py_module.def("get_input_port_names", &Module::get_input_port_names, R"(
            Get the mapping of all input nets to their corresponding port names.

            :returns: The map from input net to port name.
            :rtype: dict[hal_py.Net,str]
        )");

        py_module.def("set_output_port_name", &Module::set_output_port_name, py::arg("output_net"), py::arg("port_name"), R"(
            Set the name of the port corresponding to the specified output net to the given string.

            :param hal_py.Net output_net: The output net.
            :param str port_name: The port name.
        )");

        py_module.def("get_output_port_name", &Module::get_output_port_name, py::arg("output_net"), R"(
            Get the name of the port corresponding to the specified output net.

            :param hal_py.Net output_net: The output net.
            :returns: The port name.
            :rtype: str
        )");

        py_module.def_property_readonly("output_port_names", &Module::get_output_port_names, R"(
            Get the mapping of all output nets to their corresponding port names.

            :type: dict[hal_py.Net,str]
        )");

        py_module.def("get_output_port_names", &Module::get_output_port_names, R"(
            Get the mapping of all output nets to their corresponding port names.

            :returns: The map from output net to port name.
            :rtype: dict[hal_py.Net,str]
        )");
    }
}    // namespace hal
