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

            :returns: ``True`` if both modules are equal, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def(py::self != py::self, R"(
            Check whether two modules are unequal.
            Does not check for parent module.

            :returns: ``True`` if both modules are unequal, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("__hash__", &Module::get_hash, R"(
            Python requires hash for set and dict container.

            :returns: The hash.
            :rtype: Py_hash_t
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

        py_module.def_property_readonly("submodule_depth", &Module::get_submodule_depth, R"(
            The depth of the module within the module hierarchie (0 = top module, 1 = direct child of top module, ...).

            :type: int
        )");

        py_module.def("get_submodule_depth", &Module::get_submodule_depth, R"(
            Get the depth of the module within the module hierarchie (0 = top module, 1 = direct child of top module, ...).

            :returns: The depth within the module hierarchie.
            :rtype: int
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
            Get all parents of this module.
            If ``recursive`` is set to ``True``, all indirect parents are also included.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :param bool recursive: Set ``True`` to include indirect parents as well, ``False`` otherwise.
            :returns: A list of parent modules.
            :rtype: list[hal_py.Module]
        )");

        py_module.def("set_parent_module", &Module::set_parent_module, py::arg("new_parent"), R"(
            Set a new parent for this module.
            If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.

            :param hal_py.Module new_parent: The new parent module.
            :returns: ``True`` if the parent was changed, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("is_parent_module_of", &Module::is_parent_module_of, py::arg("module"), py::arg("recursive") = false, R"(
            Check if the module is a parent of the specified module.
         
            :param hal_py.Module module: The module.
            :param bool recursive: Set ``True`` to check recursively, ``False`` otherwise.
            :returns: ``True`` if the module is a parent of the specified module, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly(
            "submodules", [](Module* mod) { return mod->get_submodules(); }, R"(
            A list of all direct submodules of this module.

            :type: list[hal_py.Module]
        )");

        py_module.def("get_submodules", &Module::get_submodules, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Get all direct submodules of this module.
            If ``recursive`` is set to ``True``, all indirect submodules are also included.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :param bool recursive: Set ``True`` to include indirect submodules as well, ``False`` otherwise.
            :returns: A list of submodules.
            :rtype: list[hal_py.Module]
        )");

        py_module.def("is_submodule_of", &Module::is_submodule_of, py::arg("module"), py::arg("recursive") = false, R"(
            Check if the module is a submodule of the specified module.

            :param hal_py.Module module: The module.
            :param bool recursive: Set ``True`` to check recursively, ``False`` otherwise.
            :returns: ``True`` if the module is a submodule of the specified module, ``False`` otherwise.
        )");

        py_module.def("contains_module", &Module::contains_module, py::arg("other"), py::arg("recusive") = false, R"(
            Checks whether another module is a submodule of this module.
            If recursive is set to ``True``, all indirect submodules are also included.

            :param hal_py.Module other: Other module to check for.
            :param bool recursive: Set ``True`` to include indirect submodules as well, ``False`` otherwise.
            :returns: ``True`` if the other module is a submodule, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly("top_module", &Module::is_top_module, R"(
            True only if the module is the top module of the netlist.
        
            :type: bool
        )");

        py_module.def("is_top_module", &Module::is_top_module, R"(
            Returns true only if the module is the top module of the netlist.

            :returns: ``True`` if the module is the top module, ``False`` otherwise.
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

        py_module.def("update_nets", &Module::update_nets, R"(
            Iterates over all nets connected to at least one gate of the module to update the nets, internal nets, input nets, and output nets of the module.
            Has no effect on module pins. 

            WARNING: can only be used when automatic net checks have been disabled using hal_py.Netlist.enable_automatic_net_checks.

            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("contains_net", &Module::contains_net, py::arg("net"), py::arg("recursive") = false, R"(
            Check whether a net is contained in the module.
            If ``recursive`` is set to ``True``, nets in submodules are considered as well.
        
            :param hal_py.Net net: The net to check for.
            :param bool recursive: ``True`` to also consider nets in submodules, ``False`` otherwise.
            :returns: ``True`` if the net is contained in the module, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def_property_readonly("nets", py::overload_cast<>(&Module::get_nets, py::const_), R"(
            An unordered set of all nets that have at least one source or one destination within the module.

            :type: set[hal_py.Net]
        )");

        py_module.def("get_nets", py::overload_cast<>(&Module::get_nets, py::const_), R"(
            Get all nets that have at least one source or one destination within the module.

            :returns: An unordered set of nets.
            :rtype: set[hal_py.Net]
        )");

        py_module.def("get_nets", py::overload_cast<const std::function<bool(Net*)>&, bool>(&Module::get_nets, py::const_), py::arg("filter"), py::arg("recursive") = false, R"(
            Get all nets that have at least one source or one destination within the module.
            The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
            If ``recursive`` is ``True``, nets in submodules are considered as well.

            :param lambda filter: Filter function to be evaluated on each net.
            :param bool recursive: ``True`` to also consider nets in submodules, ``False`` otherwise.
            :returns: An unordered set of nets.
            :rtype: set[hal_py.Net]
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
            :returns: ``True`` if the net is an input net, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("is_output_net", &Module::is_output_net, py::arg("net"), R"(
            Check whether the given net is an output of the module, i.e., whether the net is a global output to the netlist or has at least one destination outside of the module.
            
            :param hal_py.Net net: The net.
            :returns: ``True`` if the net is an output net, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("is_internal_net", &Module::is_internal_net, py::arg("net"), R"(
            Check whether the given net is an internal net of the module, i.e. whether the net has at least one source and one destination within the module.
            
            :param hal_py.Net net: The net.
            :returns: ``True`` if the net is an internal net, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("assign_gate", &Module::assign_gate, py::arg("gate"), R"(
            Assign a gate to the module.
            The gate is removed from its previous module in the process.

            :param hal_py.Gate gate: The gate to assign.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("assign_gates", &Module::assign_gates, py::arg("gates"), R"(
            Assign a list of gates to the module.
            The gates are removed from their previous module in the process.

            :param list[hal_py.Gate] gates: The gates to assign.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("remove_gate", &Module::remove_gate, py::arg("gate"), R"(
            Remove a gate from the module.
            Automatically moves the gate to the top module of the netlist.

            :param hal_py.Gate gate: The gate to remove.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("remove_gates", &Module::remove_gates, py::arg("gates"), R"(
            Remove a list of gates from the module.
            Automatically moves the gates to the top module of the netlist.

            :param list[hal_py.Gate] gates: The gates to remove.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("contains_gate", &Module::contains_gate, py::arg("gate"), py::arg("recursive") = false, R"(
            Check whether a gate is contained in the module.
            If ``recursive`` is ``True``, gates in submodules are considered as well.

            :param hal_py.Gate gate: The gate to check for.
            :param bool recursive: ``True`` to also consider gates in submodules, ``False`` otherwise.
            :returns: ``True`` if the gate is contained in the module, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("get_gate_by_id", &Module::get_gate_by_id, py::arg("id"), py::arg("recursive") = false, R"(
            Get a gate specified by the given ID.
            If ``recursive`` is ``True``, gates in submodules are considered as well.

            :param int id: The unique ID of the gate.
            :param bool recursive: ``True`` to also consider gates in submodules, ``False`` otherwise.
            :returns: The gate if found, None otherwise.
            :rtype: hal_py.Gate or None
        )");

        py_module.def_property_readonly("gates", py::overload_cast<>(&Module::get_gates, py::const_), R"(
            The list of all gates contained within the module.

            :type: list[hal_py.Gate]
        )");

        py_module.def("get_gates", py::overload_cast<>(&Module::get_gates, py::const_), R"(
            Get all gates contained within the module.

            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_module.def("get_gates", py::overload_cast<const std::function<bool(Gate*)>&, bool>(&Module::get_gates, py::const_), py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
            Get all gates contained within the module.
            The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
            If ``recursive`` is ``True``, gates in submodules are considered as well.

            :param lambda filter: Filter function to be evaluated on each gate.
            :param bool recursive: ``True`` to also consider gates in submodules, ``False`` otherwise. Defaults to ``False``.
            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_module.def("get_unique_pin_id", &Module::get_unique_pin_id, R"(
            Get a spare pin ID.
            The value of 0 is reserved and represents an invalid ID.

            :returns: The pin ID.
            :rtype: int
        )");

        py_module.def("get_unique_pin_group_id", &Module::get_unique_pin_group_id, R"(
            Get a spare pin group ID.
            The value of 0 is reserved and represents an invalid ID.

            :returns: The pin group ID.
            :rtype: int
        )");

        py_module.def(
            "create_pin",
            [](Module& self, const u32 id, const std::string& name, Net* net, PinType type = PinType::none, bool create_group = true) -> ModulePin* {
                auto res = self.create_pin(id, name, net, type, create_group);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("id"),
            py::arg("name"),
            py::arg("net"),
            py::arg("type")         = PinType::none,
            py::arg("create_group") = true,
            R"(
            Manually assign a module pin to a net.
            Checks whether the given direction matches the actual properties of the net, i.e., checks whether the net actually is an input and/or output to the module.
            Hence, make sure to update the module nets beforehand using ``hal_py.Module.update_net``.
            If ``create_group`` is set to ``False``, the pin will not be added to a pin group.
            
            WARNING: can only be used when automatic net checks have been disabled using ``hal_py.Netlist.enable_automatic_net_checks``.

            :param int id: The ID of the pin.
            :param str name: The name of the pin.
            :param hal_py.Net net: The net that the pin is being assigned to.
            :param hal_py.PinType type: The type of the pin. Defaults to ``hal_py.PinType.none``.
            :param bool create_group: Set ``True`` to automatically create a pin group and assign the pin, ``False`` otherwise. Defaults to ``True``.
            :returns: The module pin on success, ``None`` otherwise.
            :rtype: hal_py.ModulePin or None
        )");

        py_module.def(
            "create_pin",
            [](Module& self, const std::string& name, Net* net, PinType type = PinType::none, bool create_group = true) -> ModulePin* {
                auto res = self.create_pin(name, net, type, create_group);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("name"),
            py::arg("net"),
            py::arg("type")         = PinType::none,
            py::arg("create_group") = true,
            R"(
            Manually assign a module pin to a net.
            The ID of the pin is set automatically.
            Checks whether the given direction matches the actual properties of the net, i.e., checks whether the net actually is an input and/or output to the module.
            Hence, make sure to update the module nets beforehand using ``hal_py.Module.update_net``.
            If ``create_group`` is set to ``False``, the pin will not be added to a pin group.
            
            WARNING: can only be used when automatic net checks have been disabled using ``hal_py.Netlist.enable_automatic_net_checks``.

            :param str name: The name of the pin.
            :param hal_py.Net net: The net that the pin is being assigned to.
            :param hal_py.PinType type: The type of the pin. Defaults to ``hal_py.PinType.none``.
            :param bool create_group: Set ``True`` to automatically create a pin group and assign the pin, ``False`` otherwise. Defaults to ``True``.
            :returns: The module pin on success, ``None`` otherwise.
            :rtype: hal_py.ModulePin or None
        )");

        py_module.def_property_readonly("pins", &Module::get_pins, R"(
            The (ordered) pins of the module.

            :type: list[hal_py.ModulePin]
        )");

        py_module.def("get_pins", &Module::get_pins, py::arg("filter") = nullptr, R"(
            Get the (ordered) pins of the module.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :returns: A list of pins.
            :rtype: list[hal_py.ModulePin]
        )");

        py_module.def_property_readonly(
            "pin_names",
            [](const Module& self) -> std::vector<std::string> { return self.get_pin_names(); },
            R"(
            An ordered list of the names of all pins of the module.

            :type: list[str]
        )");

        py_module.def("get_pin_names", &Module::get_pin_names, py::arg("filter") = nullptr, R"(
            Get an ordered list of the names of all pins of the module.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :returns: A list of input pin names of the module.
            :param lambda filter: An optional filter.
            :returns: An ordered list of pins.
            :rtype: list[str]
        )");

        py_module.def_property_readonly("input_pins", &Module::get_input_pins, R"(
            An ordered list of all input pins of the module (including inout pins).

            :type: list[hal_py.ModulePin]
        )");

        py_module.def("get_input_pins", &Module::get_input_pins, R"(
            Get an ordered list of all input pins of the module (including inout pins).

            :returns: An ordered list of input pins.
            :rtype: list[hal_py.ModulePin]
        )");

        py_module.def_property_readonly("input_pin_names", &Module::get_input_pin_names, R"(
            An ordered list of the names of all input pins of the module (including inout pins).

            :type: list[str]
        )");

        py_module.def("get_input_pin_names", &Module::get_input_pin_names, R"(
            Get an ordered list of the names of all input pins of the module (including inout pins).

            :returns: An ordered list of input pin names.
            :rtype: list[str]
         )");

        py_module.def_property_readonly("output_pins", &Module::get_output_pins, R"(
            An ordered list of all output pins of the module (including inout pins).

            :type: list[hal_py.ModulePin]
        )");

        py_module.def("get_output_pins", &Module::get_output_pins, R"(
            Get an ordered list of all output pins of the module (including inout pins).

            :returns: An ordered list of output pins.
            :rtype: list[hal_py.ModulePin]
        )");

        py_module.def_property_readonly("output_pin_names", &Module::get_output_pin_names, R"(
            An ordered list of the names of all output pins of the module (including inout pins).

            :type: list[str]
        )");

        py_module.def("get_output_pin_names", &Module::get_output_pin_names, R"(
            Get an ordered list of the names of all output pins of the module (including inout pins).

            :returns: An ordered list of output pin names.
            :rtype: list[str]
        )");

        py_module.def_property_readonly("pin_groups", &Module::get_pin_groups, R"(
            All pin_groups of the module.

            :type: list[hal_py.ModulePinGroup]
        )");

        py_module.def("get_pin_groups", &Module::get_pin_groups, py::arg("filter") = nullptr, R"(
            Get all pin groups of the module.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :returns: A list of pin groups.
            :rtype: list[hal_py.ModulePinGroup]
        )");

        py_module.def("get_pin_by_id", &Module::get_pin_by_id, py::arg("id"), R"(
            Get the pin corresponding to the given ID.

            :param int id: The ID of the pin.
            :returns: The pin on success, ``None`` otherwise.
            :rtype: hal_py.ModulePin or None
        )");

        py_module.def("get_pin_by_name", &Module::get_pin_by_name, py::arg("name"), R"(
             Get the pin corresponding to the given name.

             :param str name: The name of the pin.
             :returns: The pin on success, ``None`` otherwise.
             :rtype: hal_py.ModulePin or None
         )");

        py_module.def("get_pin_by_net", &Module::get_pin_by_net, py::arg("net"), R"(
            Get the pin that passes through the specified net.

            :param hal_py.Net net: The net.
            :returns: The pin on success, ``None`` otherwise.
            :rtype: hal_py.ModulePin or None
        )");

        py_module.def("get_pin_group_by_id", &Module::get_pin_group_by_id, py::arg("id"), R"(
            Get the pin group corresponding to the given ID.

            :param int id: The ID of the pin group.
            :returns: The pin group on success, ``None`` otherwise.
            :rtype: hal_py.ModulePinGroup or None
        )");

        py_module.def("get_pin_group_by_name", &Module::get_pin_group_by_name, py::arg("name"), R"(
             Get the pin group corresponding to the given name.

             :param str name: The name of the pin group.
             :returns: The pin group on success, ``None`` otherwise.
             :rtype: hal_py.ModulePinGroup or None
         )");

        py_module.def("set_pin_name", &Module::set_pin_name, py::arg("pin"), py::arg("new_name"), R"(
            Set the name of the given pin.

            :param hal_py.ModulePin pin: The pin.
            :param str new_name: The name to be assigned to the pin.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("set_pin_group_name", &Module::set_pin_group_name, py::arg("pin_group"), py::arg("new_name"), R"(
            Set the name of the given pin group.

            :param hal_py.ModulePinGroup pin_group: The pin group.
            :param str new_name: The name to be assigned to the pin group.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("set_pin_type", &Module::set_pin_type, py::arg("pin"), py::arg("new_type"), R"(
            Set the type of the given pin.

            :param hal_py.ModulePin pin: The pin.
            :param hal_py.PinType new_type: The type to be assigned to the pin.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("set_pin_group_type", &Module::set_pin_group_type, py::arg("pin_group"), py::arg("new_type"), R"(
            Set the type of the given pin group.

            :param hal_py.ModulePinGroup pin_group: The pin group.
            :param hal_py.PinType new_type: The type to be assigned to the pin group.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def("set_pin_group_direction", &Module::set_pin_group_direction, py::arg("pin_group"), py::arg("new_direction"), R"(
            Set the direction of the given pin group.

            :param hal_py.ModulePinGroup pin_group: The pin group.
            :param hal_py.PinDirection new_direction: The direction to be assigned to the pin group.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def(
            "create_pin_group",
            [](Module& self,
               const u32 id,
               const std::string& name,
               const std::vector<ModulePin*> pins = {},
               PinDirection direction             = PinDirection::none,
               PinType type                       = PinType::none,
               bool ascending                     = true,
               u32 start_index                    = 0,
               bool delete_empty_groups           = true) -> PinGroup<ModulePin>* {
                auto res = self.create_pin_group(id, name, pins, direction, type, ascending, start_index, delete_empty_groups);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin group:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("id"),
            py::arg("name"),
            py::arg("pins"),
            py::arg("direction")           = PinDirection::none,
            py::arg("type")                = PinType::none,
            py::arg("ascending")           = true,
            py::arg("start_index")         = 0,
            py::arg("delete_empty_groups") = true,
            R"(
            Create a new pin group with the given name.
            All pins to be added to the pin group must have the same direction and type.

            :param int id: The ID of the pin group.
            :param str name: The name of the pin group.
            :param list[hal_py.ModulePin] pins: The pins to be assigned to the pin group. Defaults to an empty list.
            :param hal_py.PinDirection direction: The direction of the pin group, if any. Defaults to ``hal_py.PinDirection.none``.
            :param hal_py.PinType type: The type of the pin group, if any. Defaults to ``hal_py.PinType.none``.
            :param bool ascending: Set ``True`` for ascending pin order (from 0 to n-1), ``False`` otherwise (from n-1 to 0). Defaults to ``True``.
            :param int start_index: The start index of the pin group. Defaults to ``0``.
            :param bool delete_empty_groups: Set ``True`` to delete groups that are empty after the pins have been assigned to the new group, ``False`` to keep empty groups. Defaults to ``True``.
            :returns: The pin group on success, ``None`` otherwise.
            :rtype: hal_py.ModulePinGroup or None
        )");

        py_module.def(
            "create_pin_group",
            [](Module& self,
               const std::string& name,
               const std::vector<ModulePin*> pins = {},
               PinDirection direction             = PinDirection::none,
               PinType type                       = PinType::none,
               bool ascending                     = true,
               u32 start_index                    = 0,
               bool delete_empty_groups           = true) -> PinGroup<ModulePin>* {
                auto res = self.create_pin_group(name, pins, direction, type, ascending, start_index, delete_empty_groups);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin group:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("name"),
            py::arg("pins"),
            py::arg("direction")           = PinDirection::none,
            py::arg("type")                = PinType::none,
            py::arg("ascending")           = true,
            py::arg("start_index")         = 0,
            py::arg("delete_empty_groups") = true,
            R"(
            Create a new pin group with the given name.
            All pins to be added to the pin group must have the same direction and type.

            :param str name: The name of the pin group.
            :param list[hal_py.ModulePin] pins: The pins to be assigned to the pin group. Defaults to an empty list.
            :param hal_py.PinDirection direction: The direction of the pin group, if any. Defaults to ``hal_py.PinDirection.none``.
            :param hal_py.PinType type: The type of the pin group, if any. Defaults to ``hal_py.PinType.none``.
            :param bool ascending: Set ``True`` for ascending pin order (from 0 to n-1), ``False`` otherwise (from n-1 to 0). Defaults to ``True``.
            :param int start_index: The start index of the pin group. Defaults to ``0``.
            :param bool delete_empty_groups: Set `True`` to delete groups that are empty after the pins have been assigned to the new group, ``False`` to keep empty groups. Defaults to ``True``.
            :returns: The pin group on success, ``None`` otherwise.
            :rtype: hal_py.ModulePinGroup or None
        )");

        py_module.def(
            "delete_pin_group",
            [](Module& self, PinGroup<ModulePin>* pin_group) {
                auto res = self.delete_pin_group(pin_group);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while deleting pin group:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("pin_group"),
            R"(
            Delete the given pin group.

            :param hal_py.ModulePinGroup pin_group: The pin group to be deleted.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def(
            "move_pin_group",
            [](Module& self, PinGroup<ModulePin>* pin_group, u32 new_index) {
                auto res = self.move_pin_group(pin_group, new_index);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while moving pin group:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("pin_group"),
            py::arg("new_index"),
            R"(
            Move a pin group to another index within the module.
            The indices of some other pin groups will be incremented or decremented to make room for the moved pin group to be inserted at the desired position.

            :param hal_py.ModulePinGroup pin_group: The pin group to be moved.
            :param int new_index: The index to which the pin group is moved.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def(
            "assign_pin_to_group",
            [](Module& self, PinGroup<ModulePin>* pin_group, ModulePin* pin, bool delete_empty_groups = true) {
                auto res = self.assign_pin_to_group(pin_group, pin, delete_empty_groups);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while assigning pin to pin group:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("pin_group"),
            py::arg("pin"),
            py::arg("delete_empty_groups") = true,
            R"(
            Assign a pin to a pin group.

            :param hal_py.ModulePinGroup pin_group: The new pin group.
            :param hal_py.ModulePin pin: The pin to be added.
            :param bool delete_empty_groups: Set ``True`` to delete groups that are empty after the pin has been assigned to the new group, ``False`` to keep empty groups. Defaults to ``True``.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def(
            "move_pin_within_group",
            [](Module& self, PinGroup<ModulePin>* pin_group, ModulePin* pin, u32 new_index) {
                auto res = self.move_pin_within_group(pin_group, pin, new_index);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while moving pin within pin group:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("pin_group"),
            py::arg("pin"),
            py::arg("new_index"),
            R"(
            Move a pin to another index within the given pin group.
            The indices of some other pins within the group will be incremented or decremented to make room for the moved pin to be inserted at the desired position.

            :param hal_py.ModulePinGroup pin_group: The pin group.
            :param hal_py.ModulePin pin: The pin to be moved.
            :param int new_index: The index to which the pin is moved.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_module.def(
            "remove_pin_from_group",
            [](Module& self, PinGroup<ModulePin>* pin_group, ModulePin* pin, bool delete_empty_groups = true) {
                auto res = self.remove_pin_from_group(pin_group, pin, delete_empty_groups);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while removing pin from pin group:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("pin_group"),
            py::arg("pin"),
            py::arg("delete_empty_groups") = true,
            R"(
            Remove a pin from a pin group.
            The pin will be moved to a new group that goes by the pin's name.

            :param hal_py.ModulePinGroup pin_group: The old pin group.
            :param hal_py.ModulePin pin: The pin to be removed.
            :param bool delete_empty_groups: Set ``True`` to delete the group of it is empty after the pin has been removed, ``False`` to keep the empty group. Defaults to ``True``.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
