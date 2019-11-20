#include "def.h"
#include <hdl_parser/hdl_parser_dispatcher.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#ifdef COMPILER_CLANG
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor-modified"
#endif

#include "core/log.h"
#include "core/utils.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "netlist/boolean_function.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"

#include "hdl_writer/hdl_writer_dispatcher.h"

#include "core/interface_gui.h"
#include "core/plugin_manager.h"

using map_string_to_set_of_string = std::map<std::string, std::set<std::string>>;

class Pyi_base : public i_base
{
public:
    using i_base::i_base;

    std::string get_name() const override
    {
        PYBIND11_OVERLOAD_PURE(std::string, /* Return type */
                               i_base,      /* Parent class */
                               get_name,
                               NULL); /* Name of function in C++ (must match Python name) */
    }

    std::string get_version() const override
    {
        PYBIND11_OVERLOAD_PURE(std::string, /* Return type */
                               i_base,      /* Parent class */
                               get_version,
                               NULL); /* Name of function in C++ (must match Python name) */
    }
};

class Pyi_gui : public i_gui
{
public:
    using i_gui::i_gui;

    bool exec(std::shared_ptr<netlist> g) override
    {
        PYBIND11_OVERLOAD_PURE(bool,  /* Return type */
                               i_gui, /* Parent class */
                               exec,  /* Name of function in C++ (must match Python name) */
                               g);
    }
};

namespace py = pybind11;
#ifdef PYBIND11_MODULE

PYBIND11_MODULE(hal_py, m)
{
    m.doc() = "hal python bindings";
#else
PYBIND11_PLUGIN(hal_py)
{
    py::module m("hal_py", "hal python bindings");
#endif    // ifdef PYBIND11_MODULE

    m.def("log_info", [](std::string& message) { log_info("python_context", message); }, R"( some documentation info)");

    py::class_<hal::path>(m, "hal_path").def(py::init<>()).def(py::init<const hal::path&>()).def(py::init<const std::string&>()).def("__str__", [](hal::path& p) -> std::string {
        return std::string(p.c_str());
    });

    py::implicitly_convertible<std::string, hal::path>();

    py::class_<data_container, std::shared_ptr<data_container>>(m, "data_container")
        .def("set_data", &data_container::set_data, py::arg("category"), py::arg("key"), py::arg("value_data_type"), py::arg("value"), py::arg("log_with_info_level") = false, R"(
Sets a custom data entry
If it does not exist yet, it is added.

:param str category: Key category
:param str key: Data key
:param str data_type: Data type of value
:param str value: Data value
:param bool log_with_info_level: Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
:returns: True on success.
:rtype: bool
)")
        .def("delete_data", &data_container::delete_data, py::arg("category"), py::arg("key"), py::arg("log_with_info_level") = false, R"(
Deletes custom data.

:param str category: Category of key
:param str key: Data key
:param bool log_with_info_level: Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
:returns: True on success.
:rtype: bool
)")
        .def_property_readonly("data", &data_container::get_data, R"(
Gets all stored data.

:returns: A dict from ((1) category, (2) key) to ((1) type, (2) value)
:rtype: dict[tuple(str,str), tuple(str,str)]
)")
        .def("get_data_by_key", &data_container::get_data_by_key, py::arg("category"), py::arg("key"), R"(
Gets data specified by key and category

:param str category: Category of key
:param str key: Data key
:returns: The tuple ((1) type, (2) value)
:rtype: tuple(str, str)
)")
        .def_property_readonly("data_keys", &data_container::get_data_keys, R"(
Returns all data key

:returns: A list of tuples ((1) category, (2) key)
:rtype: list[tuple(str,str)]
)");

    m.def_submodule("core_utils", R"(
HAL Core Utility functions.
)")
        .def("get_binary_directory", &core_utils::get_binary_directory, R"(
Get the path to the executable of HAL.

:returns: The path.
:rtype: hal_py.hal_path
)")
        .def("get_base_directory", &core_utils::get_base_directory, R"(
Get the base path to the HAL installation.
1. Use Environment Variable HAL_BASE_PATH
2. If current executable is hal (not e.g. python3 interpreter) use it's path to determine base path.
3. Try to find hal executable in path and use its base path.

:returns: The path.
:rtype: hal_py.hal_path
)")
        .def("get_library_directory", &core_utils::get_library_directory, R"(
Get the path to the shared and static libraries of HAL.
Relative to the binary directory.

:returns: The path.
:rtype: hal_py.hal_path
)")
        .def("get_share_directory", &core_utils::get_share_directory, R"(
Get the path to the sh
Relative to the binary

:returns: The path.
:rtype: hal_py.hal_path
)")
        .def("get_user_share_directory", &core_utils::get_user_share_directory, R"(
Get the path to shared objects and files provided by the user.
home/.local/share for Unix

:returns: The path.
:rtype: hal_py.hal_path
)")
        .def("get_user_config_directory", &core_utils::get_user_config_directory, R"(
Get the path to the configuration directory of the user.
home/.config/hal for Unix

:returns: The path.
:rtype: hal_py.hal_path
)")
        .def("get_default_log_directory", &core_utils::get_default_log_directory, py::arg("source_file") = "", R"(
Get the path to the default directory for log files.
If an hdl source file is provided, the function returns the parent directory, otherwise get_user_share_directory() / "log".

:param source_file: The hdl source file.
:type source_file: hal_py.hal_path
:returns: The path.
:rtype: hal_py.hal_path
)")
        .def("get_gate_library_directories", &core_utils::get_gate_library_directories, R"(
Get the paths where gate libraries are searched.
Contains the share and user share directories.

:returns: A list of paths.
:rtype: list(hal_py.hal_path)
)")
        .def("get_plugin_directories", &core_utils::get_plugin_directories, R"(
Get the paths where plugins are searched.
Contains the library and user share directories.

:returns: A vector of paths.
)");

    //     py::class_<gate_library, std::shared_ptr<gate_library>>(m, "gate_library")
    //         .def(py::init<const std::string&>(), R"(
    // Constructor.

    // :param str name: Name of the gate library.
    // )")
    //         .def_property_readonly("name", &gate_library::get_name, R"(
    // Returns the library name.

    // :returns: The library's name.
    // :rtype: str
    // )")
    //         .def_property_readonly("gate_types", &gate_library::get_gate_types, R"(
    // Get all gate types of the library.

    // :returns: Set of gate types.
    // :rtype: set(str)
    // )")
    //         .def_property_readonly("input_pins", &gate_library::get_input_pins, R"(
    // Get all input pin types of the library.

    // :returns: Set of input pin types.
    // :rtype: set(str)
    // )")
    //         .def_property_readonly("output_pins", &gate_library::get_output_pins, R"(
    // Get all output pin types of the library.

    // :returns: Set of output pin types.
    // :rtype: set(str)
    // )")
    //         .def_property_readonly("inout_pins", &gate_library::get_inout_pins, R"(
    // Get all inout pin types of the library.

    // :returns: Set of inout pin types.
    // :rtype: set(str)
    // )")
    //         .def_property_readonly("gnd_gate_types", &gate_library::get_gnd_gate_types, R"(
    // Get all global gnd gate types of the library.

    // :returns: Set of global gnd gate types.
    // :rtype: set(str)
    // )")
    //         .def_property_readonly("vcc_gate_types", &gate_library::get_vcc_gate_types, R"(
    // Get all global vcc gate types of the library.

    // :returns: Set of global vcc gate types.
    // :rtype: set(str)
    // )")
    //         .def_property_readonly("gate_type_map_to_input_pins", &gate_library::get_gate_type_map_to_input_pins, R"(
    // Get all input pin types for all gate types of the library.

    // :returns: Dictionary of gate type to input pin types.
    // :rtype: dict[str, list(str))]
    // )")
    //         .def_property_readonly("gate_type_map_to_output_pins", &gate_library::get_gate_type_map_to_output_pins, R"(
    // Get all output pin types for all gate types of the library.

    // :returns: Dictionary of gate types to output pin types.
    // :rtype: dict[str, list(str))]
    // )")
    //         .def_property_readonly("gate_type_map_to_inout_pins", &gate_library::get_gate_type_map_to_inout_pins, R"(
    // Get all inout pin types for all gate types of the library.

    // :returns: Dictionary of gate type to inout pin types.
    // :rtype: dict[str, list(str))]
    // )")
    //         .def_property_readonly("vhdl_includes", &gate_library::get_vhdl_includes, R"(
    // Get the VHDL includes of the library.

    // :returns: VHDL includes to use by serializer.
    // :rtype: list(str)
    // )");

    py::class_<endpoint, std::shared_ptr<endpoint>>(m, "endpoint")
        .def(py::init<>(), R"(
Constructor.
)")
        .def(py::self < py::self, R"(
Standard "less than". Required for searching through sets.

:returns: True if endpoint is less than compare target.
:rtype: bool
)")
        .def(py::self == py::self, R"(
Standard "equals". Required for searching through sets.

:returns: True if endpoint is equal to compare target.
:rtype: bool
)")
        .def(py::self != py::self, R"(
Standard "unequal".

:returns: True if endpoint is unequal to compare target.
:rtype: bool
)")
        .def_property("gate", &endpoint::get_gate, &endpoint::set_gate, R"(
The endpoint's gate.

:type: hal_py.gate
)")
        .def("get_gate", &endpoint::get_gate, R"(
Returns the gate of the current endpoint.

:returns: The gate.
:rtype: hal_py.gate
)")
        .def("set_gate", &endpoint::set_gate, py::arg("gate"), R"(
   Sets the gate of the endpoint.

:param gate: Gate to be set.
:type gate: hal_py.gate
)")
        .def_property("pin_type", &endpoint::get_pin_type, &endpoint::set_pin_type, R"(
The pin type of the current endpoint.

:type: str
)")
        .def("get_pin_type", &endpoint::get_pin_type, R"(
Returns the pin type of the current endpoint.

:returns: The pin type.
:rtype: str
)")
        .def("set_pin_type", &endpoint::set_pin_type, py::arg("type"), R"(
   Sets the pin type of the current endpoint.

   :param str type: Pin type to be set.
)");

    py::class_<netlist, std::shared_ptr<netlist>>(m, "netlist")
        .def(py::init<std::shared_ptr<gate_library>>())
        .def_property("id", &netlist::get_id, &netlist::set_id, R"(
The netlist's id. If not changed via set_id() the id is zero.

:type: int
)")
        .def("get_id", &netlist::get_id, R"(
Get the netlist's id. If not changed via set_id() the id is zero.

:returns: The netlist's id.
:rtype: int
)")
        .def("set_id", &netlist::set_id, py::arg("id"), R"(
Sets the netlist id to a new value.

:param int id: The new netlist id.
)")
        .def_property("input_filename", &netlist::get_input_filename, &netlist::set_input_filename, R"(
The file name of the input design.

:type: str
)")
        .def("get_input_filename", &netlist::get_input_filename, R"(
Get the file name of the input design.

:returns: The input file's name.
:rtype: hal_py.hal_path
)")
        .def("set_input_filename", &netlist::set_input_filename, py::arg("input_filename"), R"(
Set the file name of the input design.

:param input_filename: The path to the input file.
:type input_filename: hal_py.hal_path
)")
        .def_property("design_name", &netlist::get_design_name, &netlist::set_design_name, R"(
The design's name.

:type: str
)")
        .def("get_design_name", &netlist::get_design_name, R"(
Get the design name.

:returns: The design name.
:rtype: str
)")
        .def("set_design_name", &netlist::set_design_name, py::arg("design_name"), R"(
Sets the design name.

:param str design_name: New design name.
)")
        .def_property("device_name", &netlist::get_device_name, &netlist::set_device_name, R"(
Hardware device's name.

:type: str
)")
        .def("get_device_name", &netlist::get_device_name, R"(
Get the name of the hardware device if specified.

:returns: The target device name.
:rtype: str
)")
        .def("set_device_name", &netlist::set_device_name, py::arg("device_name"), R"(
Set the name of the target hardware device.

:param str divice_name: Name of hardware device.
)")

        .def("get_gate_library", &netlist::get_gate_library, R"(
Get the gate library associated with the netlist.

:returns: The gate library.
:rtype: hal_py.gate_library
)")

        .def("get_unique_gate_id", &netlist::get_unique_gate_id, R"(
Gets an unoccupied gate id. The value 0 is reserved and represents an invalid id.

:returns: An unoccupied unique id.
:rtype: int
)")
        .def("create_gate",
             py::overload_cast<u32, std::shared_ptr<const gate_type>, const std::string&, float, float>(&netlist::create_gate),
             py::arg("id"),
             py::arg("gate_type"),
             py::arg("name"),
             py::arg("x") = -1,
             py::arg("y") = -1,
             R"(
Creates and adds a new gate to the netlist.

:param int id: The unique ID != 0 for the new gate.
:param str gate_type: The gate type.
:param str name: A name for the gate.
:param float x: The x-coordinate of the game.
:param float y: The y-coordinate of the game.
:returns: The new gate on success, None on error.
:rtype: hal_py.gate or None
)")
        .def("create_gate",
             py::overload_cast<std::shared_ptr<const gate_type>, const std::string&, float, float>(&netlist::create_gate),
             py::arg("gate_type"),
             py::arg("name"),
             py::arg("x") = -1,
             py::arg("y") = -1,
             R"(
Creates and adds a new gate to the netlist.
It is identifiable via its unique ID which is automatically set to the next free ID.

:param str gate_type: The gate type.
:param str name: A name for the gate.
:param float x: The x-coordinate of the game.
:param float y: The y-coordinate of the game.
:returns: The new gate on success, None on error.
:rtype: hal_py.gate or None
)")
        .def("delete_gate", &netlist::delete_gate, py::arg("gate"), R"(
Removes a gate from the netlist.

:param gate: The gate to be removed.
:type gate: hal_py.gate
:returns: True on success.
:rtype: bool
)")
        .def("is_gate_in_netlist", &netlist::is_gate_in_netlist, py::arg("gate"), R"(
Check wether a gate is registered in the netlist.

:param gate: The gate to check.
:type gate: hal_py.gate
:returns: True if the gate is in netlist.
:rtype: bool
)")
        .def("get_gate_by_id", &netlist::get_gate_by_id, py::arg("gate_id"), R"(
Get a gate specified by id.

:param int gate_id: The gate's id.
:returns: The gate or None.
:rtype: hal_py.gate or None
)")
        .def_property_readonly("gates", [](netlist& n) -> std::set<std::shared_ptr<gate>> { return n.get_gates(); }, R"(
Get all gates of the netlist.

:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def("get_gates", &netlist::get_gates, py::arg("gate_type_filter") = DONT_CARE, py::arg("name_filter") = DONT_CARE, R"(
Get all gates of the netlist. You can filter the set before output with the optional parameters.

:param str gate_type_filter: Filter for the gate type.
:param str name_filter: Filter for the name.
:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def("mark_vcc_gate", &netlist::mark_vcc_gate, py::arg("gate"), R"(
Mark a gate as global vcc gate.

:param gate: The new gate.
:type gate: hal_py.gate
:returns: True on success.
:rtype: bool
)")
        .def("mark_gnd_gate", &netlist::mark_gnd_gate, py::arg("gate"), R"(
Mark a gate as global gnd gate.

:param gate: The new gate.
:type gate: hal_py.gate
:returns: True on success.
:rtype: bool
)")
        .def("unmark_vcc_gate", &netlist::unmark_vcc_gate, py::arg("gate"), R"(
Unmark a global vcc gate.

:param  gate: The new gate.
:type gate: hal_py.gate
:rtype: bool
)")
        .def("unmark_gnd_gate", &netlist::unmark_gnd_gate, py::arg("gate"), R"(
Unmark a global gnd gate.

:param gate: The new gate.
:type gate: hal_py.gate
:returns: True on success.
:rtype: bool
)")
        .def("is_vcc_gate", &netlist::is_vcc_gate, py::arg("gate"), R"(
Checks whether a gate is a global vcc gate.

:param gate: The gate to check.
:type gate: hal_py.gate
:returns: True if the gate is a global vcc gate.
:rtype: bool
)")
        .def("is_gnd_gate", &netlist::is_gnd_gate, py::arg("gate"), R"(
Checks whether a gate is a global gnd gate.

:param gate: The gate to check.
:type gate: hal_py.gate
:returns: True if the gate is a global gnd gate.
:rtype: bool
)")
        .def_property_readonly("vcc_gates", &netlist::get_vcc_gates, R"(
Get all global vcc gates.

:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def("get_vcc_gates", &netlist::get_vcc_gates, R"(
Get all global vcc gates.

:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def_property_readonly("gnd_gates", &netlist::get_gnd_gates, R"(
Get all global gnd gates.

:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def("get_gnd_gates", &netlist::get_gnd_gates, R"(
Get all global gnd gates.

:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def("get_unique_net_id", &netlist::get_unique_net_id, R"(
Gets an unoccupied net id. The value 0 is reserved and represents an invalid id.

:returns: An unoccupied unique id.
:rtype: int
)")
        .def("get_number_of_nets", &netlist::get_number_of_nets, R"(
Get number/amounts of nets in netlist.

:returns: Number of nets.
:rtype: int
)")
        .def("create_net", py::overload_cast<const u32, const std::string&>(&netlist::create_net), py::arg("id"), py::arg("name"), R"(
Creates and adds a new net to the netlist. It is identifiable via its unique id.

:param int id: The unique id != 0 for the new net.
:param name: A name for the net.
:returns: The new net on success, None on error.
:rtype: hal_py.net or None
)")
        .def("create_net", py::overload_cast<const std::string&>(&netlist::create_net), py::arg("name"), R"(
Creates and adds a new net to the netlist. It is identifiable via its unique ID which is automatically set to the next free ID.

:param name: A name for the net.
:returns: The new net on success, None on error.
:rtype: hal_py.net or None
)")
        .def("delete_net", &netlist::delete_net, py::arg("net"), R"(
Removes a net from the netlist.

:param net: The net to be removed.
:type net: hal_py.net
:returns: True on success.
:rtype: bool
)")
        .def("is_net_in_netlist", &netlist::is_net_in_netlist, py::arg("net"), R"(
Checks whether a net is registered in the netlist.

:param net: The net to check.
:type net: hal_py.net
:returns: True if the net is in netlist.
:rtype: bool
)")
        .def("get_net_by_id", &netlist::get_net_by_id, py::arg("net_id"), R"(
Get a net specified by id.

:param int net_id: The net's id.
:returns: The net or None.
:rtype: hal_py.net or None
)")
        .def_property_readonly("nets", [](netlist& n) -> std::unordered_set<std::shared_ptr<net>> { return n.get_nets(); }, R"(
Get all nets of the netlist.

:returns: A set of nets.
:rtype: set(hal_py.net)
)")
        .def("get_nets", &netlist::get_nets, py::arg("name_filter") = DONT_CARE, R"(
Get all nets of the netlist. You can filter the set before output with the optional parameters.

:param str name_filter: Filter for the name.
:returns: A set of nets.
:rtype: set(hal_py.net)
)")
        .def("mark_global_input_net", &netlist::mark_global_input_net, py::arg("net"), R"(
Mark a net as a global input net.

:param net: The net.
:type net: hal_py.net
:returns: True on success.
:rtype: bool
)")
        .def("mark_global_output_net", &netlist::mark_global_output_net, py::arg("net"), R"(
Mark a net as a global output net.

:param net: The net.
:type net: hal_py.net
:returns: True on success.
:rtype: bool
)")
        .def("unmark_global_input_net", &netlist::unmark_global_input_net, py::arg("net"), R"(
Unmark a global input net.

:param net: The net.
:type net: hal_py.net
:returns: True on success.
:rtype: bool
)")
        .def("unmark_global_output_net", &netlist::unmark_global_output_net, py::arg("net"), R"(
Unmark a global output net.

:param net: The net.
:type net: hal_py.net
:returns: True on success.
:rtype: bool
)")
        .def("is_global_input_net", &netlist::is_global_input_net, py::arg("net"), R"(
Checks wether a net is a global input net.

:param net: The net to check.
:type net: hal_py.net
:returns: True if the net is a global input net.
:rtype: bool
)")
        .def("is_global_output_net", &netlist::is_global_output_net, py::arg("net"), R"(
Checks wether a net is a global output net.

:param net: The net to check.
:type net: hal_py.net
:returns: True if the net is a global output net.
:rtype: bool
)")
        .def_property_readonly("global_input_nets", &netlist::get_global_input_nets, R"(
Get all global input nets.

:returns: A set of nets.
:rtype: set(hal_py.net)
)")
        .def("get_global_input_nets", &netlist::get_global_input_nets, R"(
Get all global input nets.

:returns: A set of nets.
:rtype: set(hal_py.net)
)")
        .def_property_readonly("global_output_nets", &netlist::get_global_output_nets, R"(
Get all global output nets.

:returns: A set of nets.
:rtype: set(hal_py.net)
)")
        .def("get_global_output_nets", &netlist::get_global_output_nets, R"(
Get all global output nets.

:returns: A set of nets.
:rtype: set(hal_py.net)
)")

        .def("get_unique_module_id", &netlist::get_unique_module_id, R"(
Gets an unoccupied module id. The value of 0 is reserved and represents an invalid id.

:returns: An unoccupied id.
:rtype: int
)")
        .def("create_module", py::overload_cast<const u32, const std::string&, std::shared_ptr<module>>(&netlist::create_module), py::arg("id"), py::arg("name"), py::arg("parent"), R"(
Creates and adds a new module to the netlist. It is identifiable via its unique id.

:param int id: The unique id != 0 for the new module.
:param str name: A name for the module.
:param hal_py.module parent: The parent module.
:returns: The new module on succes, None on error.
:rtype: hal_py.module or None
)")
        .def("create_module", py::overload_cast<const std::string&, std::shared_ptr<module>>(&netlist::create_module), py::arg("name"), py::arg("parent"), R"(
Creates and adds a new module to the netlist. It is identifiable via its unique ID which is automatically set to the next free ID.

:param str name: A name for the module.
:param hal_py.module parent: The parent module.
:returns: The new module on succes, None on error.
:rtype: hal_py.module or None
)")
        .def("delete_module", &netlist::delete_module, py::arg("module"), R"(
Removes a module from the netlist.

:param module: The module to be removed.
:type module: hal_py.module
:returns: True on success.
:rtype: bool
)")
        .def_property_readonly("modules", [](netlist& n) -> std::set<std::shared_ptr<module>> { return n.get_modules(); }, R"(
Get all modules of the netlist.

:returns: A set of modules.
:rtype: set(hal_py.module)
)")
        .def("get_modules", &netlist::get_modules, R"(
Get all modules of the netlist. The top module is included!

:returns: A set of modules.
:rtype: set(hal_py.module)
)")

        .def("get_top_module", &netlist::get_top_module, R"(
Get the top module of the netlist.

:returns: The top module.
:rtype: hal_py.module
)")

        .def("get_module_by_id", &netlist::get_module_by_id, py::arg("id"), R"(
Get a single module specified by its id.

:param int id: The module id.
:returns: The module.
:rtype: hal_py.module
)")
        .def("is_module_in_netlist", &netlist::is_module_in_netlist, py::arg("module"), R"(
Checks whether a module is registered in the netlist.

:param hal_py.module module: THe module to check.
:returns: True if the module is in netlist.
:rtype: bool
)");

    py::class_<gate, data_container, std::shared_ptr<gate>>(m, "gate", R"(
HAL Gate functions.
)")
        .def_property_readonly("id", &gate::get_id, R"(
Gets the gate's unique id.

:returns: The gate's id.
:rtype: int
)")
        .def("get_id", &gate::get_id, R"(
Gets the gate's unique id.

:returns: The gate's id.
:rtype: int
)")

        .def_property("name", &gate::get_name, &gate::set_name, R"(
The gate's name.

:param str name: The new name.
:returns: The gate's name.
:rtype: str
)")
        .def("get_name", &gate::get_name, R"(
Gets the gate's name.

:returns: The name.
:rtype: str
)")
        .def("set_name", &gate::set_name, py::arg("name"), R"(
Sets the gate's name.

:param str name: The new name.
)")

        .def_property_readonly("type", &gate::get_type, R"(
Gets the type of the gate.

:returns: The gate's type.
:rtype: str
)")
        .def("get_type", &gate::get_type, R"(
Gets the type of the gate.

:returns: The gate's type.
:rtype: str
)")

        .def_property("x", &gate::get_location_x, &gate::set_location_x, R"(
The physical location x-coordinate of the gate in the layout.
)")
        .def("get_location_x", &gate::get_location_x, R"(
Gets the physical location x-coordinate of the gate in the layout.

:returns: The gate's x-coordinate.
:rtype: float
)")
        .def("set_location_x", &gate::set_location_x, R"(
Sets the physical location x-coordinate of the gate in the layout.
Only positive values are valid, negative values will be regarded as no location assigned.

:returns: The gate's x-coordinate.
:param float x: The gate's x-coordinate.
)")
        .def_property("y", &gate::get_location_y, &gate::set_location_y, R"(
The physical location y-coordinate of the gate in the layout.
)")
        .def("get_location_y", &gate::get_location_y, R"(
Gets the physical location y-coordinate of the gate in the layout.

:returns: The gate's y-coordinate.
:rtype: float
)")
        .def("set_location_y", &gate::set_location_y, R"(
Sets the physical location y-coordinate of the gate in the layout.
Only positive values are valid, negative values will be regarded as no location assigned.

:returns: The gate's y-coordinate.
:param float y: The gate's y-coordinate.
)")
        .def_property("location", &gate::get_location, &gate::set_location, R"(
The physical location of the gate in the layout.
)")
        .def("get_location", &gate::get_location, R"(
Gets the physical location of the gate in the layout.

:returns: A tuple <x-coordinate, y-coordinate>.
:rtype: tuple(float, float)
)")
        .def("set_location", &gate::set_location, R"(
Sets the physical location of the gate in the layout.
Only positive values are valid, negative values will be regarded as no location assigned.

:param tuple(float, float) location: A pair <x-coordinate, y-coordinate>.
)")
        .def("has_location", &gate::has_location, R"(
Checks whether the gate's location in the layout is available.

:returns: Whether valid location data is available.
:rtype: bool
)")

        .def("get_module", &gate::get_module, R"(
Gets the module this gate is contained in.

:returns: The owning module.
:rtype: hal_py.module
)")
        .def("get_boolean_function", &gate::get_boolean_function, py::arg("name") = "", R"(
Get the boolean function associated with a specific name.
This name can for example be an output pin of the gate or a defined functionality like "RESET".

:param str name: The function name. If name is empty, the function of the first output pin is returned. If there is no function for the given name, the constant 'X' is returned.
:returns: The boolean function.
:rtype: boolean_function
)")
        .def("get_boolean_functions", &gate::get_boolean_functions, py::arg("only_custom_functions") = false, R"(
Get all boolean functions associated with this gate.

:param bool only_custom_functions: if true, this returns only the functions which were set via set_boolean_function
:returns: A map from function name to function.
:rtype: dict
)")
        .def("set_boolean_function", &gate::set_boolean_function, R"(
Set the boolean function with a specific name only for this gate.

:param str name:  The function name, usually an output port.
:param boolean_function func:  The function.
)")
        .def("mark_vcc_gate", &gate::mark_vcc_gate, R"(
Mark this gate as a global vcc gate.

:returns: True on success.
:rtype: bool
)")
        .def("mark_gnd_gate", &gate::mark_gnd_gate, R"(
Mark this gate as a global gnd gate.

:returns: True on success.
:rtype: bool
)")
        .def("unmark_vcc_gate", &gate::unmark_vcc_gate, R"(
Unmark this gate as a global vcc gate.

:returns: True on success.
:rtype: bool
)")
        .def("unmark_gnd_gate", &gate::unmark_gnd_gate, R"(
Unmark this gate as a global gnd gate.

:returns: True on success.
:rtype: bool
)")
        .def("is_vcc_gate", &gate::is_vcc_gate, R"(
Checks whether this gate is a global vcc gate.

:returns: True if the gate is a global vcc gate.
:rtype: bool
)")
        .def("is_gnd_gate", &gate::is_gnd_gate, R"(
Checks whether this gate is a global gnd gate.

:returns: True if the gate is a global gnd gate.
:rtype: bool
)")

        .def_property_readonly("input_pins", &gate::get_input_pins, R"(
Get all input pin types of the gate.

:returns: A list of input pin types.
:rtype: list(str)
)")
        .def("get_input_pins", &gate::get_input_pins, R"(
Get all input pin types of the gate.

:returns: A list of input pin types.
:rtype: list(str)
)")
        .def_property_readonly("output_pins", &gate::get_output_pins, R"(
Get all output pin types of the gate.

:returns: A list of output pin types.
:rtype: list(str)
)")
        .def("get_output_pins", &gate::get_output_pins, R"(
Get all output pin types of the gate.

:returns: A list of output pin types.
:rtype: list(str)
)")
        .def_property_readonly("fan_in_nets", &gate::get_fan_in_nets, R"(
Gets all fan-in nets, i.e. all nets that are connected to one of the input pins.

:returns: A set of all connected input nets.
:rtype: set(hal_py.net)
)")
        .def("get_fan_in_nets", &gate::get_fan_in_nets, R"(
Gets all fan-in nets, i.e. all nets that are connected to one of the input pins.

:returns: A set of all connected input nets.
:rtype: set(hal_py.net)
)")
        .def("get_fan_in_net", &gate::get_fan_in_net, py::arg("pin_type"), R"(
Get the fan-in net which is connected to a specific input pin.

:param str pin_type: The input pin type.
:returns: The connected input net.
:rtype: hal_py.net
)")
        .def_property_readonly("fan_out_nets", &gate::get_fan_out_nets, R"(
Get all fan-out nets, i.e. all nets that are connected to one of the output pins.

:returns: A set of all connected output nets.
:rtype: set(hal_py.net)
)")
        .def("get_fan_out_nets", &gate::get_fan_out_nets, R"(
Get all fan-out nets, i.e. all nets that are connected to one of the output pins.

:returns: A set of all connected output nets.
:rtype: set(hal_py.net)
)")
        .def("get_fan_out_net", &gate::get_fan_out_net, py::arg("pin_type"), R"(
Get the fan-out net which is connected to a specific output pin.

:param str pin_type: The output pin type.
:returns: The connected output net.
:rtype: hal_py.net
)")

        .def("get_unique_predecessors", &gate::get_unique_predecessors, py::arg("this_pin_type") = DONT_CARE, py::arg("pred_pin_type") = DONT_CARE, py::arg("gate_type") = DONT_CARE, R"(
Get all unique predecessors of a gate filterable by the gate's input pin and a specific gate type.

:param str this_pin_type: The filter for the input pin type of the this gate. Leave empty for no filtering.
:param str pred_pin_type: The filter for the output pin type of the predecessor gate. Leave empty for no filtering.
:param str gate_type: The filter for target gate types. Leave empty for no filtering.
:returns: A set of unique predecessors endpoints.
:rtype: set(hal_py.endpoint)
)")
        .def("get_predecessors", &gate::get_predecessors, py::arg("this_pin_type") = DONT_CARE, py::arg("pred_pin_type") = DONT_CARE, py::arg("gate_type") = DONT_CARE, R"(
Get all direct predecessors of a gate filterable by the gate's input pin and a specific gate type.

:param str this_pin_type: The filter for the input pin type of the this gate. Leave empty for no filtering.
:param str pred_pin_type: The filter for the output pin type of the predecessor gate. Leave empty for no filtering.
:param str gate_type: The filter for target gate types. Leave empty for no filtering.
:returns: A list of predecessors endpoints.
:rtype: list(hal_py.endpoint)
)")
        .def("get_predecessor", &gate::get_predecessor, py::arg("this_pin_type"), py::arg("pred_pin_type") = DONT_CARE, py::arg("gate_type") = DONT_CARE, R"(
Get the direct predecessor of a gate connected to a specific input pin and filterable by a specific gate type.

:param str this_pin_type: The input pin type of the this gate. Leave empty for no filtering.
:param str pred_pin_type: The filter for the output pin type of the predecessor gate. Leave empty for no filtering.
:param str gate_type: The filter for target gate types. Leave empty for no filtering.
:returns: The predecessor endpoint.
:rtype: hal_py.endpoint
)")
        .def("get_unique_successors", &gate::get_unique_successors, py::arg("this_pin_type") = DONT_CARE, py::arg("suc_pin_type") = DONT_CARE, py::arg("gate_type") = DONT_CARE, R"(
Get all direct unique successors of a gate filterable by the gate's output pin and a specific gate type.

:param str this_pin_type: The output pin type of the this gate. Leave empty for no filtering.
:param str suc_pin_type: The filter for the input pin type of the successor gate. Leave empty for no filtering.
:param str gate_type: The filter for target gate types. Leave empty for no filtering.
:returns: A set of unique successor endpoints.
:rtype: set(hal_py.endpoint)
)")
        .def("get_successors", &gate::get_successors, py::arg("this_pin_type") = DONT_CARE, py::arg("suc_pin_type") = DONT_CARE, py::arg("gate_type") = DONT_CARE, R"(
Get all direct successors of a gate filterable by the gate's output pin and a specific gate type.

:param str this_pin_type: The output pin type of the this gate. Leave empty for no filtering.
:param str suc_pin_type: The filter for the input pin type of the successor gate. Leave empty for no filtering.
:param str gate_type: The filter for target gate types. Leave empty for no filtering.
:returns: A list of successor endpoints.
:rtype: list(hal_py.endpoint)
)");

    py::class_<net, data_container, std::shared_ptr<net>>(m, "net", R"(
HAL Net functions.
)")
        .def_property_readonly("id", &net::get_id, R"(
Gets the unique id of the net.

:returns: The net's id.
:rtype: int
)")
        .def("get_id", &net::get_id, R"(
Gets the unique id of the net.

:returns: The net's id.
:rtype: int
)")
        .def_property("name", &net::get_name, &net::set_name, R"(
The net's name.

:type: str
)")
        .def("get_name", &net::get_name, R"(
Gets the net's name.

:returns: The name.
:rtype: str
)")
        .def("set_name", &net::set_name, py::arg("name"), R"(
Sets the net's name.

:param str name: The new name.
)")

        .def("set_src", py::overload_cast<std::shared_ptr<gate> const, const std::string&>(&net::set_src), py::arg("gate"), py::arg("pin_type"), R"(
Sets the source of this net to a gate's output pin.

:param gate: The source gate.
:type gate: hal_py.gate
:param str pin_type: THe pin of the source gate.
:returns: True on succes.
:rtype: bool
)")
        .def("set_src", py::overload_cast<endpoint>(&net::set_src), py::arg("endpoint"), R"(
Sets the source of this net to a gate's output pin.

:param endpoint: The source endpoint.
:type endpoint: hal_py.endpoint
:returns: True on success.
:rtype: bool
)")
        .def("remove_src", &net::remove_src, R"(
Removes the source of the net.

:returns: True on success.
:rtype: bool
)")
        .def("get_src", &net::get_src, py::arg("gate_type") = DONT_CARE, R"(
Get the src of the net specified by type. If the specifications don't match the actual source, the gate element of the returned endpoint is None.

:param str gate_type: The desired source gate type.
:returns: The source endpoint.
:rtype: hal_py.endpoint
)")
        .def("get_src_by_type", &net::get_src_by_type, py::arg("gate_type"), R"(
Get the src of the net specified by type. If the specifications don't match the actual source, the gate element of the returned endpoint is None.

:param str gate_type: The desired source gate type.
:returns: The source endpoint.
:rtype: hal_py.endpoint
)")

        .def("add_dst", py::overload_cast<std::shared_ptr<gate> const, const std::string&>(&net::add_dst), py::arg("gate"), py::arg("pin_type"), R"(
Add a destination to this net.

:param gate: The destination gate.
:type gate: hal_py.gate
:param str pin_type: The input pin of the gate.
:returns: True on succes
:rtype: bool
)")
        .def("add_dst", py::overload_cast<endpoint>(&net::add_dst), py::arg("dst"), R"(
Add a destination to this net.

:param dst: The destination endpoint.
:type dst: hal_py.endpoint
:returns: True on succes.
:rtype: bool
)")
        .def("remove_dst", py::overload_cast<std::shared_ptr<gate> const, const std::string&>(&net::remove_dst), py::arg("gate"), py::arg("pin_type"), R"(
Remove a destination from this net.

:param gate: The destination gate.
:type gate: hal_py.gate
:param str pin_type: The input pin of the gate. Leave empty if the pin does not matter.
:returns: True on succes
:rtype: bool
)")
        .def("remove_dst", py::overload_cast<endpoint>(&net::remove_dst), py::arg("dst"), R"(
Remove a destination from this net.

:param dst: The destination endpoint.
:type dst: hal_py.endpoint
:returns: True on succes.
:rtype: bool
)")
        .def("is_a_dst", py::overload_cast<std::shared_ptr<gate> const, const std::string&>(&net::is_a_dst, py::const_), py::arg("gate"), py::arg("pin_type") = DONT_CARE, R"(
Check whether a gate's input pin is a destination of this net.

:param gate: The destination gate.
:type gate: hal_py.gate
:param str pin_type: The input pin of the gate. Leave empty if the pin does not matter.
:returns: True if the input's pin is a destination.
:rtype: bool
)")
        .def("is_a_dst", py::overload_cast<endpoint>(&net::is_a_dst, py::const_), py::arg("endpoint"), R"(
Check whether a gate's input pin is a destination of this net.

:param endpoint: The input endpoint.
:type endpoint: hal_py.endpoint
:returns: True if the input's pin is a destination.
:rtype: bool
)")
        .def_property_readonly("num_of_dsts", &net::get_num_of_dsts, R"(
Get the number of destinations.

:returns: The number of destinations of this net.
:rtype: int
)")
        .def("get_num_of_dsts", &net::get_num_of_dsts, R"(
Get the number of destinations.

:returns: The number of destinations of this net.
:rtype: int
)")
        .def("get_dsts", &net::get_dsts, py::arg("gate_type") = DONT_CARE, R"(
Get the list of destinations of the net specified by type.

:param str gate_type: The desired destination gate type.
:returns: A list of destination endpoints.
:rtype: list(hal_py.endpoint)
)")
        .def("get_dsts_by_type", &net::get_dsts_by_type, py::arg("gate_type"), R"(
Get the list of destinations of the net specified by type.

:param str gate_type: The desired destination gate type.
:returns: A list of destination endpoints.
:rtype: list(hal_py.endpoint)
)")
        .def("is_unrouted", &net::is_unrouted, R"(
Check whether the net is routed, i.e. it has no source or no destinations.

:returns: True if the net is unrouted.
:rtype: bool
)")

        .def("mark_global_input_net", &net::mark_global_input_net, R"(
Mark this net as a global input net.

:returns: True on success.
:rtype: bool
)")
        .def("mark_global_output_net", &net::mark_global_output_net, R"(
Mark this net as a global output net.

:returns: True on success.
:rtype: bool
)")
        .def("unmark_global_input_net", &net::unmark_global_input_net, R"(
Unmark this net as a global input net.

:returns: True on success.
:rtype: bool
)")
        .def("unmark_global_output_net", &net::unmark_global_output_net, R"(
Unmark this net as a global output net.

:returns: True on success.
:rtype: bool
)")
        .def("is_global_input_net", &net::is_global_input_net, R"(
Checks whether this net is a global input net.

:returns: True if the net is a global input net.
:rtype: bool
)")
        .def("is_global_output_net", &net::is_global_output_net, R"(
Checks whether this net is a global output net.

:returns: True if the net is a global output net.
:rtype: bool
)");

    // module dir
    py::class_<module, std::shared_ptr<module>, data_container>(m, "module")
        .def_property_readonly("id", &module::get_id, R"(
Returns the unique ID of the module object.

:returns: The unique id.
:rtype: int
)")
        .def("get_id", &module::get_id, R"(
Returns the unique ID of the module object.

:returns: The unique id.
:rtype: int
)")
        .def_property("name", &module::get_name, &module::set_name, R"(
The module's name.

:type: str
)")
        .def("get_name", &module::get_name, R"(
Gets the module's name.

:returns: The name.
:rtype: str
)")
        .def("set_name", &module::set_name, py::arg("name"), R"(
Sets the module's name.

:param str name: The new name.
)")
        .def("get_parent_module", &module::get_parent_module, R"(
Get the parent of this module.
This returns None for the top module.

:returns: The parent module.
:rtype: hal_py.module or None
)")
        .def("set_parent_module", &module::set_parent_module, py::arg("new_parent"), R"(
Set the parent of this module.
If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.

:returns: True if the parent was changed
:rtype: bool
)")
        .def("get_submodules", &module::get_submodules, py::arg("name_filter") = DONT_CARE, py::arg("recursive") = false, R"(
Get all direct submodules of this submodule.
If recursive parameter is true, all indirect submodules are also included.

:param str name_filter: Filter for the name.
:param bool recursive: Look into submodules aswell.
:returns: The set of submodules:
:rtype: set(hal_py.module)
)")

        .def("contains_module", &module::contains_module, py::arg("other"), py::arg("recusive") = false, R"(
Checks whether another module is a submodule of this module. If \p recursive is true, all indirect submodules are also included.

:param other: Other module to check
:param recursive: Look into submodules too
:type other: hal_py.module
:type recursive: bool
:returns: True if the module is a submodule
:rtype: bool
)")
        .def("get_netlist", &module::get_netlist, R"(
Get the netlist this module is associated with.

:returns: The netlist.
:rtype: hal_py.netlist
)")
        .def("get_input_nets", &module::get_input_nets, py::arg("name_filter") = DONT_CARE, R"(
Get the input nets to this module.
A module input net is either a global input to the netlist or has a source outside of the module.

:param str name_filter: Filter for the name.
:returns: A set of module input nets.
:rtype: set(hal_py.net)
)")
        .def("get_output_nets", &module::get_output_nets, py::arg("name_filter") = DONT_CARE, R"(
Get the output nets to this module.
A module output net is either a global output of the netlist or has a destination outside of the module.

:param str name_filter: Filter for the name.
:returns: The set of module output nets.
:rtype: set(hal_py.net)
)")
        .def("get_internal_nets", &module::get_internal_nets, py::arg("name_filter") = DONT_CARE, R"(
Get the internal nets to this module.
A net is internal if its source and at least one output are inside the module.
Therefore it may contain some nets that are also regarded as output nets.

:param str name_filter: Filter for the name.
:returns: The set of internal nets.
:rtype: set(hal_py.net)
)")
        .def_property_readonly("gates", [](module& n) -> std::set<std::shared_ptr<gate>> { return n.get_gates(); }, R"(
Gets all gates of the module.

:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def("get_gates", &module::get_gates, py::arg("gate_type_filter") = DONT_CARE, py::arg("name_filter") = DONT_CARE, py::arg("recursive") = false, R"(
Returns all associated gates.
You can filter with the optional parameters.
If the parameter recursive is true, all submodules are searched aswell.

:param str gate_type_filter: Filter for the gate type.
:param str name_filter: Filter for the name.
:param bool recursive: Look into submodules too.
:returns: A set of gates.
:rtype: set(hal_py.gate)
)")
        .def("get_gate_by_id", &module::get_gate_by_id, py::arg("id"), py::arg("recursive") = false, R"(
Get a gate specified by id. If recursive parameter is true, all submodule are searched aswell.

:param int id: The gate's id.
:param bool recursive: Look into submodules too.
:returns: The gate or None.
:rtype: hal_py.gate or None
)")
        .def("assign_gate", &module::assign_gate, py::arg("gate"), R"(
Moves a gate into this module. The gate is removed from its previous module in the process.

:param gate: The gate to add.
:type gate: hal_py.gate
:returns: True on success.
:rtype: bool
)")
        .def("remove_gate", &module::remove_gate, py::arg("gate"), R"(
Removes a gate from the module object.

:param gate: The gate to remove.
:type gate: hal_py.gate
:returns: True on success.
:rtype: bool
)")
        .def("contains_gate", &module::contains_gate, py::arg("gate"), py::arg("recusive") = false, R"(
Checks whether a gate is in the module. If \p recursive is true, all submodules are searched as well.

:param gate: The gate to search for.
:param recursive: Look into submodules too
:type gate: hal_py.gate
:type recursive: bool
:returns: True if the gate is in the object.
:rtype: bool
)");

    m.def_submodule("netlist_factory")
        .def("create_netlist", &netlist_factory::create_netlist, py::arg("gate_library_name"), R"(
Creates a new netlist for a specific gate library.

:param str gate_library_name: Name of hardware gate library.
:returns: The new netlist.
:rtype: hal_py.netlist
)")
        .def("load_netlist",
             py::overload_cast<const hal::path&, const std::string&, const std::string&>(&netlist_factory::load_netlist),
             py::arg("hdl_file"),
             py::arg("language"),
             py::arg("gate_library_name"),
             R"(
Creates a new netlist for a specific file.

:param hdl_file: Name of the hdl file.
:type hdl_file: hal_py.hal_path
:param str language: Programming language uses in parameter file_name.
:param gate_library_name: Name of hardware gate library.
:returns: The new netlist.
:rtype: hal_py.netlist
)")
        .def("load_netlist", py::overload_cast<const hal::path&>(&netlist_factory::load_netlist), py::arg("hal_file"), R"(
Creates a new netlist for a specific '.hal' file.

:param hal_file: Name of the '.hal' file.
:type hal_file: hal_py.hal_path
:returns: The new netlist.
:rtype: hal_py.netlist
)");

    // hdl_file_writer/hdl_writer
    m.def_submodule("hdl_writer_dispatcher")
        .def("get_cli_options", &hdl_writer_dispatcher::get_cli_options, R"(
Returns the command line interface options of the hdl writer dispatcher.

:returns: The options.
:rtype: list(list(str, list(str), set(str)))
)")
        .def("write", py::overload_cast<std::shared_ptr<netlist>, const std::string&, const hal::path&>(&hdl_writer_dispatcher::write), py::arg("netlist"), py::arg("format"), py::arg("file_name"), R"(
Writes the netlist into a file with a defined format.

:param netlist: The netlist.
:type netlist: hal_py.netlist
:param format: The target format of the file, e.g. vhdl, verilog...
:type format: str
:param file_name: The input file.
:type file_name: hal_py.hal_path
:returns: True on success.
:rtype: bool
)");

    m.def_submodule("plugin_manager")
        .def("get_plugin_names", &plugin_manager::get_plugin_names, R"(
Get the names of all loaded plugins.

:returns: The set of plugin names.
:rtype: set(str)
)")
        .def("load_all_plugins", &plugin_manager::load_all_plugins, py::arg("file_names") = std::vector<hal::path>(), R"(
Load all plugins in the specified directories. If \p directory is empty, the default directories will be searched.

:param file_names: A list of directory paths.
:type file_names: hal_py.hal_path
:returns: True on success.
:rtype: bool
)")
        .def("load", &plugin_manager::load, py::arg("plugin_name"), py::arg("file_name"), R"(
Load a single plugin by specifying a name and the file path.

:param str plugin_name: The desired name, unique in the framework.
:param file_name: The path to the plugin file.
:type file_name: hal_py.hal_path
:returns: True on success.
:rtype: bool
)")
        .def("unload_all_plugins", &plugin_manager::unload_all_plugins, R"(
Releases all plugins and associated resources.

:returns: True on success.
:rtype: bool
)")
        .def(
            "get_plugin_instance", [](const std::string& plugin_name) -> std::shared_ptr<i_base> { return plugin_manager::get_plugin_instance<i_base>(plugin_name, true); }, py::arg("plugin_name"), R"(
Gets the basic interface for a plugin specified by name.

:param str plugin_name: The plugin name.
:returns: The interface base for the plugin.
:rtype: hal_py.i_base
)");

    py::class_<i_base, std::shared_ptr<i_base>, Pyi_base>(m, "i_base")
        .def_property_readonly("name", &i_base::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def("get_name", &i_base::get_name, R"(
Get the name of the plugin.

:returns: Plugin name.
:rtype: str
)")
        .def_property_readonly("version", &i_base::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)")
        .def("get_version", &i_base::get_version, R"(
Get the version of the plugin.

:returns: Plugin version.
:rtype: str
)");

    py::class_<i_gui, std::shared_ptr<i_gui>, Pyi_gui>(m, "i_gui").def("exec", &i_gui::exec, py::arg("netlist"), R"(
Generic call to run the GUI.

:param netlist: The netlist object for the GUI.
:type netlist: hal_py.netlist
:returns: True on success.
:rtype: bool
)");

    py::enum_<boolean_function::value>(m, "Value").value("X", boolean_function::value::X).value("ZERO", boolean_function::value::ZERO).value("ONE", boolean_function::value::ONE).export_values();

    py::class_<boolean_function>(m, "boolean_function")
        .def(py::init<>())
        .def(py::init<const std::string&, bool>())
        .def(py::init<boolean_function::value>())
        .def("substitute", &boolean_function::substitute, R"(
Substitutes a variable with another function (can again be a single variable).
Applies to all instances of the variable in the function.

:param str variable_name:  The variable to substitute
:param boolean_function function:  The function to take the place of the varible
:returns: The new boolean function.
:rtype: boolean_function
)")
        .def("evaluate", &boolean_function::evaluate, py::arg("inputs") = std::map<std::string, boolean_function::value>(), R"(
Evaluates the function on the given inputs and returns the result.

:param map(str,value) inputs:  A map from variable names to values.
:returns: The value that the function evaluates to.
:rtype: value
)")
        .def("is_constant_one", &boolean_function::is_constant_one, R"(
Checks whether the function constantly outputs ONE.

:returns: True if function is constant ONE, false otherwise.
:rtype: bool
)")
        .def("is_constant_zero", &boolean_function::is_constant_zero, R"(
Checks whether the function constantly outputs ZERO.

:returns: True if function is constant ZERO, false otherwise.
:rtype: bool
)")
        .def("is_empty", &boolean_function::is_empty, R"(
Checks whether the function is empty.

:returns: True if function is empty, false otherwise.
:rtype: bool
)")
        .def("get_variables", &boolean_function::get_variables, R"(
Get all variable names used in this boolean function.

:returns: A set of all variable names.
:rtype: set(str)
)")
        .def_static("from_string", &boolean_function::from_string, R"(
Returns the boolean function as a string.

:param str expression: String containing a boolean function.
:returns: The boolean function extracted from the string.
:rtype: boolean_function
)")
        .def("__str__", [](const boolean_function& f) { return f.to_string(); })
        .def(py::self & py::self)
        .def(py::self | py::self)
        .def(py::self ^ py::self)
        .def(py::self &= py::self)
        .def(py::self |= py::self)
        .def(py::self ^= py::self)
        .def(!py::self)
        .def("is_dnf", &boolean_function::is_dnf, R"(
Tests whether the function is in DNF.

:returns: True if in DNF, false otherwise.
:rtype: bool
)")
        .def("to_dnf", &boolean_function::to_dnf, R"(
Gets the plain DNF representation of the function.

:returns: The DNF as a boolean function.
:rtype: boolean_function
)")
        .def("optimize", &boolean_function::optimize, R"(
Optimizes the function by first converting it to DNF and then applying the Quine-McCluskey algorithm.

:returns: The optimized boolean function.
:rtype: boolean_function
)")
        .def("get_truth_table", &boolean_function::get_truth_table, py::arg("ordered_variables") = std::vector<std::string>(), R"(
Get the truth table outputs of the function.
WARNING: Exponential runtime in the number of variables!

Output is the vector of output values when walking the truth table in ascending order.
The variable values are changed in order of appearance, i.e.:
first_var second_var | output_vector_index
    0         0      |   0
    1         0      |   1
    0         1      |   2
    1         1      |   3

If ordered_variables is empty, all included variables are used and ordered alphabetically.

:param list(str) ordered_variables: Specific order in which the inputs shall be structured in the truth table.
:returns: The vector of output values.
:rtype: list(value)
)");

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}

#pragma GCC diagnostic pop
