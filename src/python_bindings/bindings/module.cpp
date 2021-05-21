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

            :returns: True if both modules are equal, false otherwise.
            :rtype: bool
        )");

        py_module.def(py::self != py::self, R"(
            Check whether two modules are unequal.
            Does not check for parent module.

            :returns: True if both modules are unequal, false otherwise.
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
            The parent module of this module. Set to None for the top module.

            :type: hal_py.Module or None
        )");

        py_module.def("get_parent_module", &Module::get_parent_module, R"(
            Get the parent module of this module.
            For the top module, None is returned.

            :returns: The parent module.
            :rtype: hal_py.Module or None
        )");

        py_module.def("set_parent_module", &Module::set_parent_module, py::arg("new_parent"), R"(
            Set a new parent for this module.
            If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.

            :param hal_py.Module new_parent: The new parent module.
            :returns: True if the parent was changed, false otherwise.
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
            :param bool recursive: True to include indirect submodules as well.
            :returns: The vector of submodules.
            :rtype: list[hal_py.Module]
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

        py_module.def_property_readonly("nets", &Module::get_nets, R"(
            A sorted list of all nets that have at least one source or one destination within the module. Includes nets that are input and/or output to any of the submodules.

            :type: list[hal_py.Net]
        )");

        py_module.def("get_nets", &Module::get_nets, R"(
            Get all nets that have at least one source or one destination within the module. Includes nets that are input and/or output to any of the submodules.

            :returns: A sorted list of nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def_property_readonly("input_nets", &Module::get_input_nets, R"(
            A sorted list of all nets that are either a global input to the netlist or have at least one source outside of the module.

            :type: list[hal_py.Net]
        )");

        py_module.def("get_input_nets", &Module::get_input_nets, R"(
            Get all nets that are either a global input to the netlist or have at least one source outside of the module.

            :returns: A sorted list of input nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def_property_readonly("output_nets", &Module::get_output_nets, R"(
            A sorted list of all nets that are either a global output to the netlist or have at least one destination outside of the module.

            :type: set[hal_py.Net]
        )");

        py_module.def("get_output_nets", &Module::get_output_nets, R"(
            Get all nets that are either a global output to the netlist or have at least one destination outside of the module.

            :returns: A sorted list of output nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def_property_readonly("internal_nets", &Module::get_internal_nets, R"(
            A sorted list of all nets that have at least one source and one destination within the module, including its submodules. The result may contain nets that are also regarded as input or output nets.

            :type: list[hal_py.Net]
        )");

        py_module.def("get_internal_nets", &Module::get_internal_nets, R"(
            Get all nets that have at least one source and one destination within the module. The result may contain nets that are also regarded as input or output nets.

            :returns: A sorted list of internal nets.
            :rtype: list[hal_py.Net]
        )");

        py_module.def("set_input_port_name", &Module::set_input_port_name, py::arg("input_net"), py::arg("port_name"), R"(
            Set the name of the port corresponding to the specified input net.

            :param hal_py.Net input_net: The input net.
            :param str port_name: The input port name.
        )");

        py_module.def("get_input_port_name", &Module::get_input_port_name, py::arg("input_net"), R"(
            Get the name of the port corresponding to the specified input net.

            :param hal_py.Net input_net: The input net.
            :returns: The input port name.
            :rtype: str
        )");

        py_module.def("get_input_port_net", &Module::get_input_port_net, py::arg("port_name"), R"(
            Get the input net of the port corresponding to the specified port name.

            :param str port_name: The input port name.
            :returns: The input net.
            :rtype: hal_py.Net or None
        )");

        py_module.def_property_readonly("input_port_names", &Module::get_input_port_names, R"(
            The dictionary mapping all input nets to their corresponding port names.

            :type: dict[hal_py.Net,str]
        )");

        py_module.def("get_input_port_names", &Module::get_input_port_names, R"(
            Get the mapping of all input nets to their corresponding port names.

            :returns: The dictionary from input net to port name.
            :rtype: dict[hal_py.Net,str]
        )");

        py_module.def("set_output_port_name", &Module::set_output_port_name, py::arg("output_net"), py::arg("port_name"), R"(
            Set the name of the port corresponding to the specified output net.

            :param hal_py.Net output_net: The output net.
            :param str port_name: The output port name.
        )");

        py_module.def("get_output_port_name", &Module::get_output_port_name, py::arg("output_net"), R"(
            Get the name of the port corresponding to the specified output net.

            :param hal_py.Net output_net: The output net.
            :returns: The output port name.
            :rtype: str
        )");

        py_module.def("get_output_port_net", &Module::get_output_port_net, py::arg("port_name"), R"(
            Get the output net of the port corresponding to the specified port name.

            :param str port_name: The output port name.
            :returns: The output net.
            :rtype: hal_py.Net or None
        )");

        py_module.def_property_readonly("output_port_names", &Module::get_output_port_names, R"(
            The dictionary mapping all output nets to their corresponding port names.

            :type: dict[hal_py.Net,str]
        )");

        py_module.def("get_output_port_names", &Module::get_output_port_names, R"(
            Get the mapping of all output nets to their corresponding port names.

            :returns: The dictionary from output net to port name.
            :rtype: dict[hal_py.Net,str]
        )");

        py_module.def_property("next_input_port_id", &Module::get_next_input_port_id, &Module::set_next_input_port_id, R"(
            The next free input port ID.
        )");

        py_module.def("get_next_input_port_id", &Module::get_next_input_port_id, R"(
            Get the next free input port ID.

            :returns: The next input port ID.
            :rtype: int
        )");

        py_module.def("set_next_input_port_id", &Module::set_next_input_port_id, py::arg("id"), R"(
            Set the next free input port ID to the given value.

            :param int id: The next input port ID.
        )");

        py_module.def_property("next_output_port_id", &Module::get_next_output_port_id, &Module::set_next_output_port_id, R"(
            The next free output port ID.
        )");

        py_module.def("get_next_output_port_id", &Module::get_next_output_port_id, R"(
            Get the next free output port ID.

            :returns: The next output port ID.
            :rtype: int
        )");

        py_module.def("set_next_output_port_id", &Module::set_next_output_port_id, py::arg("id"), R"(
            Set the next free output port ID to the given value.

            :param int id: The next output port ID.
        )");

        py_module.def("set_cache_dirty", &Module::set_cache_dirty, py::arg("is_dirty") = true, R"(
            Mark all internal caches as dirty. Caches are primarily used for the nets connected to the gates of a module.

            :param bool is_dirty: True to mark caches as dirty, False otherwise.
        )");

        py_module.def("assign_gate", &Module::assign_gate, py::arg("gate"), R"(
            Assign a gate to the module.
            The gate is removed from its previous module in the process.

            :param hal_py.Gate gate: The gate to assign.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_module.def("remove_gate", &Module::remove_gate, py::arg("gate"), R"(
            Remove a gate from the module.
            Automatically moves the gate to the top module of the netlist.

            :param hal_py.Gate gate: The gate to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_module.def("contains_gate", &Module::contains_gate, py::arg("gate"), py::arg("recusive") = false, R"(
            Check whether a gate is in the module.
            If recursive is set to true, all submodules are searched as well.

            :param hal_py.Gate gate: The gate to check for.
            :param bool recursive: True to also search in submodules.
            :returns: True if the gate is in the module, false otherwise.
            :rtype: bool
        )");

        py_module.def("get_gate_by_id", &Module::get_gate_by_id, py::arg("id"), py::arg("recursive") = false, R"(
            Get a gate specified by the given ID.
            If recursive is true, all submodules are searched as well.

            :param int id: The unique ID of the gate.
            :param bool recursive: True to also search in submodules.
            :returns: The gate if found, None otherwise.
            :rtype: hal_py.Gate or None
        )");

        py_module.def_property_readonly(
            "gates", [](Module* mod) { return mod->get_gates(); }, R"(
            The list of all gates contained within the module.

            :type: list[hal_py.Gate]
        )");

        py_module.def("get_gates", &Module::get_gates, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Get all modules contained within the module.
            A filter can be applied to the result to only get gates matching the specified condition.
            If recursive is true, all submodules are searched as well.

            :param lambda filter: Filter to be applied to the gates.
            :param bool recursive: True to also search in submodules.
            :returns: The list of all gates.
            :rtype: list[hal_py.Gate]
        )");
    }
}    // namespace hal
