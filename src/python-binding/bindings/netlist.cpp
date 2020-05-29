#include "bindings.h"

void netlist_init(py::module& m)
{
    py::class_<netlist, std::shared_ptr<netlist>> py_netlist(
        m, "netlist", R"(Netlist class containing information about the netlist including its gates, modules, and nets, as well as the underlying gate library.)");

    py_netlist.def(py::init<std::shared_ptr<gate_library>>(), R"(
        Construct a new netlist for a specified gate library.

        :param hal_py.gate_library library: The gate library.
)");

    py_netlist.def_property("id", &netlist::get_id, &netlist::set_id, R"(
        The netlist's id. If not changed via set_id() the id is zero.

        :type: int
)");

    py_netlist.def("get_id", &netlist::get_id, R"(
        Get the netlist's id. If not changed via set_id() the id is zero.

        :returns: The netlist's id.
        :rtype: int
)");

    py_netlist.def("set_id", &netlist::set_id, py::arg("id"), R"(
        Sets the netlist id to a new value.

        :param int id: The new netlist id.
)");

    py_netlist.def_property("input_filename", &netlist::get_input_filename, &netlist::set_input_filename, R"(
        The file name of the input design.

        :type: str
)");

    py_netlist.def("get_input_filename", &netlist::get_input_filename, R"(
        Get the file name of the input design.

        :returns: The input file's name.
        :rtype: hal_py.hal_path
)");

    py_netlist.def("set_input_filename", &netlist::set_input_filename, py::arg("input_filename"), R"(
        Set the file name of the input design.

        :param hal_py.hal_path input_filename: The path to the input file.
)");

    py_netlist.def_property("design_name", &netlist::get_design_name, &netlist::set_design_name, R"(
        The design's name.

        :type: str
)");

    py_netlist.def("get_design_name", &netlist::get_design_name, R"(
        Get the design name.

        :returns: The design name.
        :rtype: str
)");

    py_netlist.def("set_design_name", &netlist::set_design_name, py::arg("design_name"), R"(
        Sets the design name.

        :param str design_name: New design name.
)");

    py_netlist.def_property("device_name", &netlist::get_device_name, &netlist::set_device_name, R"(
        Hardware device's name.

        :type: str
)");

    py_netlist.def("get_device_name", &netlist::get_device_name, R"(
        Get the name of the hardware device if specified.

        :returns: The target device name.
        :rtype: str
)");

    py_netlist.def("set_device_name", &netlist::set_device_name, py::arg("device_name"), R"(
        Set the name of the target hardware device.

        :param str divice_name: Name of hardware device.
)");

    py_netlist.def_property_readonly("gate_library", &netlist::get_gate_library, R"(
        Get the gate library associated with the netlist.

        :type: hal_py.gate_library
)");

    py_netlist.def("get_gate_library", &netlist::get_gate_library, R"(
        Get the gate library associated with the netlist.

        :returns: The gate library.
        :rtype: hal_py.gate_library
)");

    py_netlist.def("get_unique_module_id", &netlist::get_unique_module_id, R"(
        Gets an unoccupied module id. The value of 0 is reserved and represents an invalid id.

        :returns: An unoccupied id.
        :rtype: int
)");

    py_netlist.def("create_module",
                   py::overload_cast<const u32, const std::string&, std::shared_ptr<module>, const std::vector<std::shared_ptr<gate>>&>(&netlist::create_module),
                   py::arg("id"),
                   py::arg("name"),
                   py::arg("parent"),
                   py::arg("gates") = std::vector<std::shared_ptr<gate>>(),
                   R"(
        Creates and adds a new module to the netlist. It is identifiable via its unique id.

        :param int id: The unique id != 0 for the new module.
        :param str name: A name for the module.
        :param hal_py.module parent: The parent module.
        :param list gates: Gates to add to the module.
        :returns: The new module on succes, None on error.
        :rtype: hal_py.module or None
)");

    py_netlist.def("create_module",
                   py::overload_cast<const std::string&, std::shared_ptr<module>, const std::vector<std::shared_ptr<gate>>&>(&netlist::create_module),
                   py::arg("name"),
                   py::arg("parent"),
                   py::arg("gates") = std::vector<std::shared_ptr<gate>>(),
                   R"(
        Creates and adds a new module to the netlist. It is identifiable via its unique ID which is automatically set to the next free ID.

        :param str name: A name for the module.
        :param hal_py.module parent: The parent module.
        :param list gates: Gates to add to the module.
        :returns: The new module on succes, None on error.
        :rtype: hal_py.module or None
)");

    py_netlist.def("delete_module", &netlist::delete_module, py::arg("module"), R"(
        Removes a module from the netlist.

        :param module: The module to be removed.
        :type module: hal_py.module
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("get_module_by_id", &netlist::get_module_by_id, py::arg("id"), R"(
        Get a single module specified by its id.

        :param int id: The module id.
        :returns: The module.
        :rtype: hal_py.module
)");

    py_netlist.def_property_readonly("modules", &netlist::get_modules, R"(
        Get a set of all modules of the netlist including the top module.

        :rtype: set[hal_py.module]
)");

    py_netlist.def("get_modules", &netlist::get_modules, R"(
        Get all modules of the netlist. The top module is included!

        :returns: A set of modules.
        :rtype: set[hal_py.module]
)");

    py_netlist.def_property_readonly("top_module", &netlist::get_top_module, R"(
        The top module of the netlist.

        :type: hal_py.module
)");

    py_netlist.def("get_top_module", &netlist::get_top_module, R"(
        Get the top module of the netlist.

        :returns: The top module.
        :rtype: hal_py.module
)");

    py_netlist.def("get_module_by_id", &netlist::get_module_by_id, py::arg("id"), R"(
        Get a single module specified by its id.

        :param int id: The module id.
        :returns: The module.
        :rtype: hal_py.module
)");

    py_netlist.def("is_module_in_netlist", &netlist::is_module_in_netlist, py::arg("module"), R"(
Checks whether a module is registered in the netlist.

        :param hal_py.module module: The module to check.
        :returns: True if the module is in netlist.
        :rtype: bool
)");

    py_netlist.def("get_unique_gate_id", &netlist::get_unique_gate_id, R"(
        Gets an unoccupied gate id. The value 0 is reserved and represents an invalid id.

        :returns: An unoccupied unique id.
        :rtype: int
)");

    py_netlist.def("create_gate",
                   py::overload_cast<u32, std::shared_ptr<const gate_type>, const std::string&, float, float>(&netlist::create_gate),
                   py::arg("id"),
                   py::arg("gt"),
                   py::arg("name"),
                   py::arg("x") = -1,
                   py::arg("y") = -1,
                   R"(
        Creates and adds a new gate to the netlist.

        :param int id: The unique ID != 0 for the new gate.
        :param hal_py.gate_type gt: The gate type.
        :param str name: A name for the gate.
        :param float x: The x-coordinate of the gate.
        :param float y: The y-coordinate of the gate.
        :returns: The new gate on success, None on error.
        :rtype: hal_py.gate or None
)");

    py_netlist.def("create_gate",
                   py::overload_cast<std::shared_ptr<const gate_type>, const std::string&, float, float>(&netlist::create_gate),
                   py::arg("gt"),
                   py::arg("name"),
                   py::arg("x") = -1,
                   py::arg("y") = -1,
                   R"(
        Creates and adds a new gate to the netlist.
        It is identifiable via its unique ID which is automatically set to the next free ID.

        :param hal_py.gate_type gt: The gate type.
        :param str name: A name for the gate.
        :param float x: The x-coordinate of the gate.
        :param float y: The y-coordinate of the gate.
        :returns: The new gate on success, None on error.
        :rtype: hal_py.gate or None
)");

    py_netlist.def("delete_gate", &netlist::delete_gate, py::arg("gate"), R"(
        Removes a gate from the netlist.

        :param gate: The gate to be removed.
        :type gate: hal_py.gate
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("is_gate_in_netlist", &netlist::is_gate_in_netlist, py::arg("gate"), R"(
        Check wether a gate is registered in the netlist.

        :param gate: The gate to check.
        :type gate: hal_py.gate
        :returns: True if the gate is in netlist.
        :rtype: bool
)");

    py_netlist.def("get_gate_by_id", &netlist::get_gate_by_id, py::arg("gate_id"), R"(
        Get a gate specified by id.

        :param int gate_id: The gate's id.
        :returns: The gate or None.
        :rtype: hal_py.gate or None
)");

    py_netlist.def_property_readonly(
        "gates", [](const std::shared_ptr<netlist>& n) { return n->get_gates(); }, R"(
        A set containing all gates of the netlist.

        :type: set[hal_py.gate]
)");

    py_netlist.def("get_gates", &netlist::get_gates, py::arg("filter") = nullptr, R"(
        Get all gates of the netlist. You can filter the set before output with the optional parameters.

        :param lambda filter: Filter for the gates.
        :returns: A set of gates.
        :rtype: set[hal_py.gate]
)");

    py_netlist.def("mark_vcc_gate", &netlist::mark_vcc_gate, py::arg("gate"), R"(
        Mark a gate as global vcc gate.

        :param gate: The gate.
        :type gate: hal_py.gate
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("mark_gnd_gate", &netlist::mark_gnd_gate, py::arg("gate"), R"(
        Mark a gate as global gnd gate.

        :param gate: The gate.
        :type gate: hal_py.gate
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("unmark_vcc_gate", &netlist::unmark_vcc_gate, py::arg("gate"), R"(
Unmark a global vcc gate.

        :param  gate: The gate.
        :type gate: hal_py.gate
        :rtype: bool
)");

    py_netlist.def("unmark_gnd_gate", &netlist::unmark_gnd_gate, py::arg("gate"), R"(
        Unmark a global gnd gate.

        :param gate: The gate.
        :type gate: hal_py.gate
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("is_vcc_gate", &netlist::is_vcc_gate, py::arg("gate"), R"(
        Checks whether a gate is a global vcc gate.

        :param gate: The gate to check.
        :type gate: hal_py.gate
        :returns: True if the gate is a global vcc gate.
        :rtype: bool
)");

    py_netlist.def("is_gnd_gate", &netlist::is_gnd_gate, py::arg("gate"), R"(
        Checks whether a gate is a global gnd gate.

        :param gate: The gate to check.
        :type gate: hal_py.gate
        :returns: True if the gate is a global gnd gate.
        :rtype: bool
)");

    py_netlist.def_property_readonly("vcc_gates", &netlist::get_vcc_gates, R"(
        A set containing all global vcc gates.

        :type: set[hal_py.gate]
)");

    py_netlist.def("get_vcc_gates", &netlist::get_vcc_gates, R"(
        Get all global vcc gates.

        :returns: A set of gates.
        :rtype: set[hal_py.gate]
)");

    py_netlist.def_property_readonly("gnd_gates", &netlist::get_gnd_gates, R"(
        A set containing all global gnd gates.

        :type: set[hal_py.gate]
)");

    py_netlist.def("get_gnd_gates", &netlist::get_gnd_gates, R"(
        Get all global gnd gates.

        :returns: A set of gates.
        :rtype: set[hal_py.gate]
)");

    py_netlist.def("get_unique_net_id", &netlist::get_unique_net_id, R"(
        Gets an unoccupied net id. The value 0 is reserved and represents an invalid id.

        :returns: An unoccupied unique id.
        :rtype: int
)");

    py_netlist.def("create_net", py::overload_cast<const u32, const std::string&>(&netlist::create_net), py::arg("id"), py::arg("name"), R"(
        Creates and adds a new net to the netlist. It is identifiable via its unique id.

        :param int id: The unique id != 0 for the new net.
        :param str name: A name for the net.
        :returns: The new net on success, None on error.
        :rtype: hal_py.net or None
)");

    py_netlist.def("create_net", py::overload_cast<const std::string&>(&netlist::create_net), py::arg("name"), R"(
        Creates and adds a new net to the netlist. It is identifiable via its unique ID which is automatically set to the next free ID.

        :param str name: A name for the net.
        :returns: The new net on success, None on error.
        :rtype: hal_py.net or None
)");

    py_netlist.def("delete_net", &netlist::delete_net, py::arg("net"), R"(
        Removes a net from the netlist.

        :param hal_py.net net: The net to be removed.
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("is_net_in_netlist", &netlist::is_net_in_netlist, py::arg("net"), R"(
        Checks whether a net is registered in the netlist.

        :param hal_py.net net: The net to check.
        :returns: True if the net is in netlist.
        :rtype: bool
)");

    py_netlist.def("get_net_by_id", &netlist::get_net_by_id, py::arg("net_id"), R"(
        Get a net specified by id.

        :param int net_id: The net's id.
        :returns: The net or None.
        :rtype: hal_py.net or None
)");

    py_netlist.def_property_readonly(
        "nets", [](const std::shared_ptr<netlist>& n) { return n->get_nets(); }, R"(
        A set containing all nets of the netlist.

        :type: set[hal_py.net]
)");

    py_netlist.def("get_nets", &netlist::get_nets, py::arg("filter") = nullptr, R"(
        Get all nets of the netlist. You can filter the set before output with the optional parameters.

        :param lambda filter: Filter for the nets.
        :returns: A set of nets.
        :rtype: set[hal_py.net]
)");

    py_netlist.def("mark_global_input_net", &netlist::mark_global_input_net, py::arg("net"), R"(
        Mark a net as a global input net.

        :param hal_py.net net: The net.
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("mark_global_output_net", &netlist::mark_global_output_net, py::arg("net"), R"(
        Mark a net as a global output net.

        :param hal_py.net net: The net.
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("unmark_global_input_net", &netlist::unmark_global_input_net, py::arg("net"), R"(
        Unmark a global input net.

        :param hal_py.net net: The net.
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("unmark_global_output_net", &netlist::unmark_global_output_net, py::arg("net"), R"(
        Unmark a global output net.

        :param hal_py.net net: The net.
        :returns: True on success.
        :rtype: bool
)");

    py_netlist.def("is_global_input_net", &netlist::is_global_input_net, py::arg("net"), R"(
        Checks wether a net is a global input net.

        :param hal_py.net net: The net to check.
        :returns: True if the net is a global input net.
        :rtype: bool
)");

    py_netlist.def("is_global_output_net", &netlist::is_global_output_net, py::arg("net"), R"(
        Checks wether a net is a global output net.

        :param hal_py.net net: The net to check.
        :returns: True if the net is a global output net.
        :rtype: bool
)");

    py_netlist.def_property_readonly("global_input_nets", &netlist::get_global_input_nets, R"(
        A set of all global input nets.

        :type: set[hal_py.net]
)");

    py_netlist.def("get_global_input_nets", &netlist::get_global_input_nets, R"(
        Get all global input nets.

        :returns: A set of nets.
        :rtype: set[hal_py.net]
)");

    py_netlist.def_property_readonly("global_output_nets", &netlist::get_global_output_nets, R"(
        A set of all global output nets.

        :type: set[hal_py.net]
)");

    py_netlist.def("get_global_output_nets", &netlist::get_global_output_nets, R"(
        Get all global output nets.

        :returns: A set of nets.
        :rtype: set[hal_py.net]
)");
}