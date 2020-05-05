#include "def.h"

#include "core/interface_gui.h"
#include "core/log.h"
#include "core/plugin_manager.h"
#include "core/utils.h"

#include "netlist/hdl_parser/hdl_parser_dispatcher.h"

#include "netlist/hdl_writer/hdl_writer_dispatcher.h"

#include "netlist/boolean_function.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_type/gate_type.h"
#include "netlist/gate_library/gate_type/gate_type_lut.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"
#include "gui/gui_api/gui_api.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#ifdef COMPILER_CLANG
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor-modified"
#endif

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "pybind11/functional.h"

#pragma GCC diagnostic pop

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


py::class_<hal::path> py_path(m, "hal_path");
    
py_path.def(py::init<>());

py_path.def(py::init<const hal::path&>());
    
py_path.def(py::init<const std::string&>()).def("__str__", [](hal::path& p) -> std::string {return std::string(p.c_str());});

py::implicitly_convertible<std::string, hal::path>();


py::class_<data_container, std::shared_ptr<data_container>> py_data_container(m, "data_container");


py_data_container.def(py::init<>(), R"(
        Construct a new data container.
)");

py_data_container.def("set_data", &data_container::set_data, py::arg("category"), py::arg("key"), py::arg("value_data_type"), py::arg("value"), py::arg("log_with_info_level") = false, R"(
        Sets a custom data entry
        If it does not exist yet, it is added.

        :param str category: Key category
        :param str key: Data key
        :param str data_type: Data type of value
        :param str value: Data value
        :param bool log_with_info_level: Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
        :returns: True on success.
        :rtype: bool
)");

py_data_container.def("delete_data", &data_container::delete_data, py::arg("category"), py::arg("key"), py::arg("log_with_info_level") = false, R"(
        Deletes custom data.

        :param str category: Category of key
        :param str key: Data key
        :param bool log_with_info_level: Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
        :returns: True on success.
        :rtype: bool
)");

py_data_container.def_property_readonly("data", &data_container::get_data, R"(
        A dict from ((1) category, (2) key) to ((1) type, (2) value) containing the stored data.

        :type: dict[tuple(str,str),tuple(str,str)]
)");

py_data_container.def("get_data_by_key", &data_container::get_data_by_key, py::arg("category"), py::arg("key"), R"(
        Gets data specified by key and category

        :param str category: Category of key
        :param str key: Data key
        :returns: The tuple ((1) type, (2) value)
        :rtype: tuple(str,str)
)");

py_data_container.def_property_readonly("data_keys", &data_container::get_data_keys, R"(
        A list of tuples ((1) category, (2) key) containing all the data keys.

        :type: list[tuple(str,str)]
)");

auto py_core_utils = m.def_submodule("core_utils", R"(
        HAL Core Utility functions.
)");

py_core_utils.def("get_binary_directory", &core_utils::get_binary_directory, R"(
        Get the path to the executable of HAL.

        :returns: The path.
        :rtype: hal_py.hal_path
)");

py_core_utils.def("get_base_directory", &core_utils::get_base_directory, R"(
        Get the base path to the HAL installation.
        1. Use Environment Variable HAL_BASE_PATH
        2. If current executable is hal (not e.g. python3 interpreter) use it's path to determine base path.
        3. Try to find hal executable in path and use its base path.

        :returns: The path.
        :rtype: hal_py.hal_path
)");

py_core_utils.def("get_library_directory", &core_utils::get_library_directory, R"(
        Get the path to the shared and static libraries of HAL.
        Relative to the binary directory.

        :returns: The path.
        :rtype: hal_py.hal_path
)");

py_core_utils.def("get_share_directory", &core_utils::get_share_directory, R"(
        Get the path to the sh
        Relative to the binary

        :returns: The path.
        :rtype: hal_py.hal_path
)");

py_core_utils.def("get_user_share_directory", &core_utils::get_user_share_directory, R"(
        Get the path to shared objects and files provided by the user.
        home/.local/share for Unix

        :returns: The path.
        :rtype: hal_py.hal_path
)");

py_core_utils.def("get_user_config_directory", &core_utils::get_user_config_directory, R"(
        Get the path to the configuration directory of the user.
        home/.config/hal for Unix

        :returns: The path.
        :rtype: hal_py.hal_path
)");

py_core_utils.def("get_default_log_directory", &core_utils::get_default_log_directory, py::arg("source_file") = "", R"(
        Get the path to the default directory for log files.
        If an hdl source file is provided, the function returns the parent directory, otherwise get_user_share_directory() / "log".

        :param source_file: The hdl source file.
        :type source_file: hal_py.hal_path
        :returns: The path.
        :rtype: hal_py.hal_path
)");

py_core_utils.def("get_gate_library_directories", &core_utils::get_gate_library_directories, R"(
        Get the paths where gate libraries are searched.
        Contains the share and user share directories.

        :returns: A list of paths.
        :rtype: list[hal_py.hal_path]
)");

py_core_utils.def("get_plugin_directories", &core_utils::get_plugin_directories, R"(
        Get the paths where plugins are searched.
        Contains the library and user share directories.

        :returns: A vector of paths.
)");


py::class_<gate_type, std::shared_ptr<gate_type>> py_gate_type(m, "gate_type", R"(
        Gate type class containing information about the internals of a specific gate type.
)");

py::enum_<gate_type::base_type>(py_gate_type, "base_type", R"(
        Represents the base type of a gate type. Available are: combinatorial, lut, ff, and latch.
        )")

        .value("combinatorial", gate_type::base_type::combinatorial)
        .value("lut", gate_type::base_type::lut)
        .value("ff", gate_type::base_type::ff)
        .value("latch", gate_type::base_type::latch)
        .export_values();

py_gate_type.def(py::init<const std::string&>(), py::arg("name"), R"(
        Construct a new gate type.

        :param str name: The name of the gate type.
)");

py_gate_type.def(py::self == py::self, R"(
        Test whether two gate type objects are equal.

        :returns: True when both gate type objects are equal, false otherwise.
        :rtype: bool
)");

py_gate_type.def(py::self != py::self, R"(
        Test whether two gate type objects are unequal.

        :returns: True when both gate type objects are unequal, false otherwise.
        :rtype: bool
)");

py_gate_type.def("__str__", [](const gate_type& gt) { return gt.to_string(); });

py_gate_type.def_property_readonly("name", &gate_type::get_name, R"(
        The name of the gate type.

        :type: str
)");

py_gate_type.def("get_name", &gate_type::get_name, R"(
        Get the name of the gate type.

        :returns: The name of the gate type.
        :rtype: str
)");

py_gate_type.def("get_base_type", &gate_type::get_base_type, R"(
        Get the base type of the gate type. The base type can be either combinatorial, lut, ff, or latch.

        :returns: The base type of the gate type.
        :rtype: hal_py.gate_type.base_type
)");

py_gate_type.def("add_input_pin", &gate_type::add_input_pin, py::arg("input_pin"), R"(
        Add an input pin to the gate type.

        :param str input_pin: The name of an input pin.
)");

py_gate_type.def("add_input_pins", &gate_type::add_input_pins, py::arg("input_pins"), R"(
        Add a list of input pins to the gate type.

        :param list[str] input_pins: The list of input pins.
)");

py_gate_type.def_property_readonly("input_pins", &gate_type::get_input_pins, R"(
        A list of input pins of the gate type.

        :type: list[str]
)");

py_gate_type.def("get_input_pins", &gate_type::get_input_pins, R"(
        Get a list of input pins of the gate type.

        :returns: A list of input pins of the gate type.
        :rtype: list[str]
)");

py_gate_type.def("add_output_pin", &gate_type::add_output_pin, py::arg("output_pin"), R"(
        Add an output pin to the gate type.

        :param str output_pin: The name of an output pin.
)");

py_gate_type.def("add_output_pins", &gate_type::add_output_pins, py::arg("output_pins"), R"(
        Add a list of output pins to the gate type.

        :param list[str] output_pins: The list of output pins.
)");

py_gate_type.def_property_readonly("output_pins", &gate_type::get_output_pins, R"(
        A list of output pins of the gate type.

        :type: list[str]
)");

py_gate_type.def("get_output_pins", &gate_type::get_output_pins, R"(
        Get a list of output pins of the gate type.

        :returns: A vector of output pins of the gate type.
        :rtype: list[str]
)");

py_gate_type.def("add_boolean_function", &gate_type::add_boolean_function, py::arg("pin_name"), py::arg("bf"), R"(
        Add a boolean function with the specified name to the gate type.

        :param str name: The name of the boolean function.
        :param hal_py.boolean_function bf: A boolean function object.
)");

py_gate_type.def_property_readonly("boolean_functions", &gate_type::get_boolean_functions, R"(
        A map from function names to the boolean functions of the gate type.

        :type: dict[str,hal_py.boolean_function]
)");

py_gate_type.def("get_boolean_functions", &gate_type::get_boolean_functions, R"(
        Get a map containing the boolean functions of the gate type.

        :returns: A map from function names to boolean functions.
        :rtype: dict[str,hal_py.boolean_function]
)");


py::class_<gate_type_lut, gate_type, std::shared_ptr<gate_type_lut>> py_gate_type_lut(m, "gate_type_lut", R"(
        LUT gate type class containing information about the internals of a specific LUT gate type.
)");

py_gate_type_lut.def(py::init<const std::string&>(), py::arg("name"), R"(
        Construct a new LUT gate type.

        :param str name: The name of the LUT gate type.
)");

py_gate_type_lut.def("add_output_from_init_string_pin", &gate_type_lut::add_output_from_init_string_pin, py::arg("output_pin"), R"(
        Adds an output pin to the collection of output pins that generate their output not from a boolean function but an initialization string.

        :param str output_pin: The name of the output string.
)");

py_gate_type_lut.def_property_readonly("output_from_init_string_pins", &gate_type_lut::get_output_from_init_string_pins, R"(
        The set of output pins that generate their output not from a boolean function but an initialization string.

        :type: set[str]
)");

py_gate_type_lut.def("get_output_from_init_string_pins", &gate_type_lut::get_output_from_init_string_pins, R"(
        Get the set of output pins that generate their output not from a boolean function but an initialization string.

        :returns: Set of oputput pin names.
        :rtype: set[str]
)");

py_gate_type_lut.def_property("config_data_category", &gate_type_lut::get_config_data_category, &gate_type_lut::set_config_data_category, R"(
        The category in which to find the INIT string.

        :type: str
)");

py_gate_type_lut.def("set_config_data_category", &gate_type_lut::set_config_data_category, py::arg("category"), R"(
        Set the category in which to find the INIT string.

        :param str category: The category as a string.
)");

py_gate_type_lut.def("get_config_data_category", &gate_type_lut::get_config_data_category, R"(
        Get the category in which to find the INIT string.

        :returns: The category as a string.
        :rtype: str
)");


py_gate_type_lut.def_property("config_data_identifier", &gate_type_lut::get_config_data_identifier, &gate_type_lut::set_config_data_identifier, R"(
        The identifier used to specify the INIT string.

        :type: str
)");

py_gate_type_lut.def("set_config_data_identifier", &gate_type_lut::set_config_data_identifier, py::arg("identifier"), R"(
        Set the identifier used to specify the INIT string.

        :param str identifier: The identifier as a string.
)");

py_gate_type_lut.def("get_config_data_identifier", &gate_type_lut::get_config_data_identifier, R"(
        Get the identifier used to specify the INIT string.

        :returns: The identifier as a string.
        :rtype: str
)");

py_gate_type_lut.def_property("config_data_ascending_order", &gate_type_lut::is_config_data_ascending_order, &gate_type_lut::set_config_data_ascending_order, R"(
        The bit-order of the INIT string, true if ascending.

        :type: bool
)");

py_gate_type_lut.def_property("config_data_ascending_order", &gate_type_lut::is_config_data_ascending_order, &gate_type_lut::set_config_data_ascending_order, R"(
        The bit-order of the INIT string, true if ascending.

        :type: bool
)");

py_gate_type_lut.def("set_config_data_ascending_order", &gate_type_lut::set_config_data_ascending_order, py::arg("ascending"), R"(
        Set the bit-order of the INIT string.

        :param bool ascending: True if ascending bit-order, false otherwise.
)");

py_gate_type_lut.def("is_config_data_ascending_order", &gate_type_lut::is_config_data_ascending_order, R"(
Get the bit-order of the INIT string.

:returns: True if ascending bit-order, false otherwise.
:rtype: bool
)");


py::class_<gate_type_sequential, gate_type, std::shared_ptr<gate_type_sequential>> py_gate_type_sequential(m, "gate_type_sequential", R"(Sequential gate type class containing information about the internals of a specific sequential gate type.)");

py::enum_<gate_type_sequential::set_reset_behavior>(py_gate_type_sequential, "set_reset_behavior", R"(
        Represents the behavior that a sequential cell shows when both set and reset are active. Available are: U (not specified for gate type), L (set to ZERO), H (set to ONE), N (no change), T (toggle), and X (undefined).)")
        .value("U", gate_type_sequential::set_reset_behavior::U)
        .value("L", gate_type_sequential::set_reset_behavior::L)
        .value("H", gate_type_sequential::set_reset_behavior::H)
        .value("N", gate_type_sequential::set_reset_behavior::N)
        .value("T", gate_type_sequential::set_reset_behavior::T)
        .value("X", gate_type_sequential::set_reset_behavior::X)
        .export_values();

py_gate_type_sequential.def(py::init<const std::string&, gate_type::base_type>(), py::arg("name"), py::arg("bt"), R"(
        Construct a new sequential gate type.

        :param str name: The name of the sequential gate type.
        :param hal_py.gate_type.base_type bt: The base type of the sequential gate type.
)");

py_gate_type_sequential.def("add_state_output_pin", &gate_type_sequential::add_state_output_pin, py::arg("output_pin"), R"(
        Adds an output pin to the collection of output pins that generate their output from the next_state function.

        :param str output_pin_name: Name of the output pin.
)");

py_gate_type_sequential.def("get_state_output_pins", &gate_type_sequential::get_state_output_pins, R"(
        Get the output pins that use the next_state function to generate their output.

        :returns: The set of output pin names.
        :rtype: set[str]
)");

py_gate_type_sequential.def("get_state_output_pins", &gate_type_sequential::get_state_output_pins, R"(
        Get the output pins that use the next_state function to generate their output.

        :returns: The set of output pin names.
        :rtype: set[str]
)");

py_gate_type_sequential.def("get_state_output_pins", &gate_type_sequential::get_state_output_pins, R"(
        Get the output pins that use the next_state function to generate their output.

        :returns: The set of output pin names.
        :rtype: set[str]
)");

py_gate_type_sequential.def("add_inverted_state_output_pin", &gate_type_sequential::add_inverted_state_output_pin, py::arg("output_pin"), R"(
        Adds an output pin to the collection of output pins that generate their output from the inverted next_state function.

        :param str output_pin_name: Name of the output pin.
)");

py_gate_type_sequential.def("get_inverted_state_output_pins", &gate_type_sequential::get_inverted_state_output_pins, R"(
        Get the output pins that use the inverted next_state function to generate their output.

        :returns: The set of output pin names.
        :rtype: set[str]
)");

/*py_gate_type_sequential.def_property("set_reset_behavior", &gate_type_sequential::get_set_reset_behavior, &gate_type_sequential::set_set_reset_behavior, R"(
        Set the behavior that describes the internal state when both set and reset are active.

        :type: tuple(hal_py.set_reset_behavior, hal_py.set_reset_behavior)
)");*/

py_gate_type_sequential.def("set_set_reset_behavior", &gate_type_sequential::set_set_reset_behavior, py::arg("sb1"), py::arg("sb2"), R"(
        Set the behavior that describes the internal state when both set and reset are active.

        :param hal_py.set_reset_behavior sb1: The value specifying the behavior for the internal state.
        :param hal_py.set_reset_behavior sb2: The value specifying the behavior for the inverted internal state.
)");

py_gate_type_sequential.def("get_set_reset_behavior", &gate_type_sequential::get_set_reset_behavior, R"(
        Get the behavior that describes the internal state when both set and reset are active.

        :returns: A tuple of values specifying the behavior of the internal state and the inverted internal state.
        :rytpe: tuple(hal_py.set_reset_behavior, hal_py.set_reset_behavior)
)");

py_gate_type_sequential.def_property("init_data_category", &gate_type_sequential::get_init_data_category, &gate_type_sequential::set_init_data_category, R"(
        The category in which to find the INIT string.

        :type: str
)");

py_gate_type_sequential.def("set_init_data_category", &gate_type_sequential::set_init_data_category, py::arg("category"), R"(
        Set the category in which to find the INIT string.

        :param str category: The category as a string.
)");

py_gate_type_sequential.def("get_init_data_category", &gate_type_sequential::get_init_data_category, R"(
        Get the category in which to find the INIT string.

        :returns: The category as a string.
        :rtype: str
)");

py_gate_type_sequential.def_property("init_data_identifier", &gate_type_sequential::get_init_data_identifier, &gate_type_sequential::set_init_data_identifier, R"(
        The identifier used to specify the INIT string.

        :type: str
)");

py_gate_type_sequential.def("set_init_data_identifier", &gate_type_sequential::set_init_data_identifier, py::arg("identifier"), R"(
        Set the identifier used to specify the INIT string.

        :param str identifier: The identifier as a string.
)");

py_gate_type_sequential.def("get_init_data_identifier", &gate_type_sequential::get_init_data_identifier, R"(
        Get the identifier used to specify the INIT string.

        :returns: The identifier as a string.
        :rtype: str
)");


py::class_<gate_library, std::shared_ptr<gate_library>> py_gate_library(m, "gate_library", R"(Gate library class containing information about the gates contained in the library.)");

py_gate_library.def(py::init<const std::string&>(), py::arg("name"), R"(
        Construct a new gate library.

        :param str name: Name of the gate library.
)");

py_gate_library.def_property_readonly("name", &gate_library::get_name, R"(
        The name of the library.

        :type: str
)");

py_gate_library.def("get_name", &gate_library::get_name, R"(
        Get the name of the library.

        :returns: The name.
        :rtype: str
)");

py_gate_library.def("add_gate_types", &gate_library::add_gate_type, py::arg("gt"), R"(
        Add a gate type to the gate library.

        :param hal_py.gate_type gt: The gate type object.
)");

py_gate_library.def_property_readonly("gate_types", &gate_library::get_gate_types, R"(
        A dict from gate type names to gate type objects containing all gate types of the gate library.

        :type: dict[str,hal_py.gate_type]
)");

py_gate_library.def("get_gate_types", &gate_library::get_gate_types, R"(
        Get dict from gate type names to gate type objects containing all gate types of the gate library.

        :returns: A dict from gate type names to gate type objects.
        :rtype: dict[str,hal_py.gate_type]
)");

py_gate_library.def_property_readonly("vcc_gate_types", &gate_library::get_vcc_gate_types, R"(
        A dict from gate type names to gate type objects containing all VCC gate types of the gate library.

        :type: dict[str,hal_py.gate_type]
)");

py_gate_library.def("get_vcc_gate_types", &gate_library::get_vcc_gate_types, R"(
        Get dict from gate type names to gate type objects containing all VCC gate types of the gate library.

        :returns: A dict from VCC gate type names to gate type objects.
        :rtype: dict[str,hal_py.gate_type]
)");

py_gate_library.def_property_readonly("gnd_gate_types", &gate_library::get_vcc_gate_types, R"(
        A dict from gate type names to gate type objects containing all GND gate types of the gate library.

        :type: dict[str,hal_py.gate_type]
)");

py_gate_library.def("get_gnd_gate_types", &gate_library::get_gnd_gate_types, R"(
        Get dict from gate type names to gate type objects containing all GND gate types of the gate library.

        :returns: A dictp from GND gate type names to gate type objects.
        :rtype: dict[str,hal_py.gate_type]
)");

py_gate_library.def("add_include", &gate_library::add_include, py::arg("include"), R"(
        Add a necessary includes of the gate library, e.g., VHDL libraries.

        :param str inc: The include to add.
)");

py_gate_library.def_property_readonly("includes", &gate_library::get_includes, R"(
        A list of necessary includes of the gate library, e.g., VHDL libraries.

        :returns: VHDL includes to use by serializer.
        :rtype: list[str]
)");

py_gate_library.def("get_includes", &gate_library::get_includes, R"(
        Get a list of necessary includes of the gate library, e.g., VHDL libraries.

        :returns: A list of includes.
        :rtype: list[str]
)");


py::class_<endpoint, std::shared_ptr<endpoint>> py_endpoint(m, "endpoint");

py_endpoint.def(py::init<const std::shared_ptr<gate>&, const std::string&, bool>(), py::arg("gate"), py::arg("pin"), py::arg("is_a_destination"), R"(
        Construct a new endpoint.
)");

py_endpoint.def(py::self < py::self, R"(
        Standard "less than". Required for searching through sets.

        :returns: True if endpoint is less than compare target.
        :rtype: bool
)");

py_endpoint.def(py::self == py::self, R"(
        Standard "equals". Required for searching through sets.

        :returns: True if endpoint is equal to compare target.
        :rtype: bool
)");

py_endpoint.def(py::self != py::self, R"(
        Standard "unequal".

        :returns: True if endpoint is unequal to compare target.
        :rtype: bool
)");

// FIXME this type doesn't make sense
py_endpoint.def_property_readonly("gate", &endpoint::get_gate, R"(
        The endpoint's gate.

        :type: hal_py.get_gate()
)");

// FIXME this type doesn't make sense
py_endpoint.def("get_gate", &endpoint::get_gate, R"(
        Returns the gate of the current endpoint.

        :returns: The gate.
        :rtype: hal_py.get_gate()
)");

// FIXME below the docs say "pin type", which is obsolete
py_endpoint.def_property_readonly("pin", &endpoint::get_pin, R"(
        The pin type of the current endpoint.

        :type: str
)");

py_endpoint.def("get_pin", &endpoint::get_pin, R"(
        Returns the pin type of the current endpoint.

        :returns: The pin type.
        :rtype: str
)");

py_endpoint.def_property_readonly("is_source", &endpoint::is_source_pin, R"(
        Checks the pin type of the current endpoint.

        :type: bool
)");

py_endpoint.def("is_source_pin", &endpoint::is_source_pin, R"(
        Checks the pin type of the current endpoint.

        :returns: The pin type.
        :rtype: bool
)");

py_endpoint.def_property_readonly("is_destination", &endpoint::is_destination_pin, R"(
        Checks the pin type of the current endpoint.

        :type: bool
)");

py_endpoint.def("is_destination_pin", &endpoint::is_destination_pin, R"(
        Checks the pin type of the current endpoint.

        :returns: The pin type.
        :rtype: bool
)");

py::class_<netlist, std::shared_ptr<netlist>> py_netlist(m, "netlist", R"(Netlist class containing information about the netlist including its gates, modules, and nets, as well as the underlying gate library.)");

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

py_netlist.def("create_module", py::overload_cast<const u32, const std::string&, std::shared_ptr<module>, const std::vector<std::shared_ptr<gate>>&>(&netlist::create_module), py::arg("id"), py::arg("name"), py::arg("parent"), py::arg("gates") = std::vector<std::shared_ptr<gate>>(), R"(
        Creates and adds a new module to the netlist. It is identifiable via its unique id.

        :param int id: The unique id != 0 for the new module.
        :param str name: A name for the module.
        :param hal_py.module parent: The parent module.
        :param list gates: Gates to add to the module.
        :returns: The new module on succes, None on error.
        :rtype: hal_py.module or None
)");

py_netlist.def("create_module", py::overload_cast<const std::string&, std::shared_ptr<module>, const std::vector<std::shared_ptr<gate>>&>(&netlist::create_module), py::arg("name"), py::arg("parent"), py::arg("gates") = std::vector<std::shared_ptr<gate>>(), R"(
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

// FIXME "game"??, "get_gate()" ist kein Typ
py_netlist.def("create_gate", py::overload_cast<u32, std::shared_ptr<const gate_type>, const std::string&, float, float>(&netlist::create_gate),
        py::arg("id"),
        py::arg("gt"),
        py::arg("name"),
        py::arg("x") = -1,
        py::arg("y") = -1, R"(
        Creates and adds a new gate to the netlist.

        :param int id: The unique ID != 0 for the new gate.
        :param hal_py.gate_type gt: The gate type.
        :param str name: A name for the gate.
        :param float x: The x-coordinate of the game.
        :param float y: The y-coordinate of the game.
        :returns: The new gate on success, None on error.
        :rtype: hal_py.get_gate() or None
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("create_gate", py::overload_cast<std::shared_ptr<const gate_type>, const std::string&, float, float>(&netlist::create_gate),
        py::arg("gt"),
        py::arg("name"),
        py::arg("x") = -1,
        py::arg("y") = -1, R"(
        Creates and adds a new gate to the netlist.
        It is identifiable via its unique ID which is automatically set to the next free ID.

        :param hal_py.gate_type gt: The gate type.
        :param str name: A name for the gate.
        :param float x: The x-coordinate of the game.
        :param float y: The y-coordinate of the game.
        :returns: The new gate on success, None on error.
        :rtype: hal_py.get_gate() or None
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("delete_gate", &netlist::delete_gate, py::arg("gate"), R"(
        Removes a gate from the netlist.

        :param gate: The gate to be removed.
        :type gate: hal_py.get_gate()
        :returns: True on success.
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("is_gate_in_netlist", &netlist::is_gate_in_netlist, py::arg("gate"), R"(
        Check wether a gate is registered in the netlist.

        :param gate: The gate to check.
        :type gate: hal_py.get_gate()
        :returns: True if the gate is in netlist.
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("get_gate_by_id", &netlist::get_gate_by_id, py::arg("gate_id"), R"(
        Get a gate specified by id.

        :param int gate_id: The gate's id.
        :returns: The gate or None.
        :rtype: hal_py.get_gate() or None
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def_property_readonly("gates", [](const std::shared_ptr<netlist>& n){return n->get_gates();}, R"(
        A set containing all gates of the netlist.

        :type: set[hal_py.get_gate()]
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("get_gates", &netlist::get_gates, py::arg("filter") = nullptr, R"(
        Get all gates of the netlist. You can filter the set before output with the optional parameters.

        :param lambda filter: Filter for the gates.
        :returns: A set of gates.
        :rtype: set[hal_py.get_gate()]
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("mark_vcc_gate", &netlist::mark_vcc_gate, py::arg("gate"), R"(
        Mark a gate as global vcc gate.

        :param gate: The gate.
        :type gate: hal_py.get_gate()
        :returns: True on success.
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("mark_gnd_gate", &netlist::mark_gnd_gate, py::arg("gate"), R"(
        Mark a gate as global gnd gate.

        :param gate: The gate.
        :type gate: hal_py.get_gate()
        :returns: True on success.
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("unmark_vcc_gate", &netlist::unmark_vcc_gate, py::arg("gate"), R"(
Unmark a global vcc gate.

        :param  gate: The gate.
        :type gate: hal_py.get_gate()
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("unmark_gnd_gate", &netlist::unmark_gnd_gate, py::arg("gate"), R"(
        Unmark a global gnd gate.

        :param gate: The gate.
        :type gate: hal_py.get_gate()
        :returns: True on success.
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("is_vcc_gate", &netlist::is_vcc_gate, py::arg("gate"), R"(
        Checks whether a gate is a global vcc gate.

        :param gate: The gate to check.
        :type gate: hal_py.get_gate()
        :returns: True if the gate is a global vcc gate.
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("is_gnd_gate", &netlist::is_gnd_gate, py::arg("gate"), R"(
        Checks whether a gate is a global gnd gate.

        :param gate: The gate to check.
        :type gate: hal_py.get_gate()
        :returns: True if the gate is a global gnd gate.
        :rtype: bool
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def_property_readonly("vcc_gates", &netlist::get_vcc_gates, R"(
        A set containing all global vcc gates.

        :type: set[hal_py.get_gate()]
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("get_vcc_gates", &netlist::get_vcc_gates, R"(
        Get all global vcc gates.

        :returns: A set of gates.
        :rtype: set[hal_py.get_gate()]
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def_property_readonly("gnd_gates", &netlist::get_gnd_gates, R"(
        A set containing all global gnd gates.

        :type: set[hal_py.get_gate()]
)");

// FIXME "get_gate()" ist kein Typ
py_netlist.def("get_gnd_gates", &netlist::get_gnd_gates, R"(
        Get all global gnd gates.

        :returns: A set of gates.
        :rtype: set[hal_py.get_gate()]
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

py_netlist.def_property_readonly("nets", [](const std::shared_ptr<netlist>& n){return n->get_nets();}, R"(
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

py::class_<gate, data_container, std::shared_ptr<gate>> py_gate(m, "gate", R"(Gate class containing information about a gate including its location, functions, and module.)");

py_gate.def_property_readonly("id", &gate::get_id, R"(
        The unique ID of the gate.

        :type: int
)");

py_gate.def("get_id", &gate::get_id, R"(
        Gets the gate's unique id.

        :returns: The gate's id.
        :type: int
)");

py_gate.def_property_readonly("netlist", &gate::get_netlist, R"(
        The parent netlist of the gate.

        :type: hal_py.netlist
)");

py_gate.def("get_netlist", &gate::get_netlist, R"(
        Gets the parent netlist of the gate.

        :returns: The netlist.
        :rtype: hal_py.netlist
)");

py_gate.def_property("name", &gate::get_name, &gate::set_name, R"(
        The name of the gate.

        :type: str
)");

py_gate.def("get_name", &gate::get_name, R"(
        Gets the gate's name.

        :returns: The name.
        :rtype: str
)");

py_gate.def("set_name", &gate::set_name, py::arg("name"), R"(
        Sets the gate's name.

        :param str name: The new name.
)");

py_gate.def_property_readonly("type", &gate::get_type, R"(
        The type of the gate

        :type: hal_py.gate_type
)");

py_gate.def("get_type", &gate::get_type, R"(
        Gets the type of the gate.

        :returns: The gate's type.
        :rtype: hal_py.gate_type
)");

py_gate.def("has_location", &gate::has_location, R"(
        Checks whether the gate's location in the layout is available.

        :returns: True if valid location data is available, false otherwise.
        :rtype: bool
)");

py_gate.def_property("location_x", &gate::get_location_x, &gate::set_location_x, R"(
        The x-coordinate of the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :type: float
)");

py_gate.def("get_location_x", &gate::get_location_x, R"(
        Gets the x-coordinate of the physical location of the gate in the layout.

        :returns: The x-coordinate.
        :rtype: float
)");

py_gate.def("set_location_x", &gate::set_location_x, py::arg("x"), R"(
        Sets the x-coordinate of the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :param float x: The x-coordinate.
)");

py_gate.def_property("location_y", &gate::get_location_y, &gate::set_location_y, R"(
        The y-coordinate of the physical location of the gate in the layout.
        Only positive values are valid, negative values will be regarded as no location assigned.

        :type: float
)");

py_gate.def("get_location_y", &gate::get_location_y, R"(
        Gets the y-coordinate of the physical location of the gate in the layout.

        :returns: The  y-coordinate.
        :rtype: float
)");

py_gate.def("set_location_y", &gate::set_location_y, py::arg("y"), R"(
        Sets the y-coordinate of the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :param float y: The  y-coordinate.
)");

py_gate.def_property("location", &gate::get_location, &gate::set_location, R"(
        The physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :type: tuple(float,float)
)");

py_gate.def("get_location", &gate::get_location, R"(
        Gets the physical location of the gate in the layout.

        :returns: A tuple <x-coordinate, y-coordinate>.
        :rtype: tuple(float,float)
)");

py_gate.def("set_location", &gate::set_location, py::arg("location"), R"(
        Sets the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :param tuple(float,float) location: A pair <x-coordinate, y-coordinate>.
)");

py_gate.def_property_readonly("module", &gate::get_module, R"(
        The module in which this gate is contained.

        :type: hal_py.module
)");

py_gate.def("get_module", &gate::get_module, R"(
        Gets the module in which this gate is contained.

        :returns: The module.
        :rtype: hal_py.module
)");

py_gate.def("get_boolean_function", &gate::get_boolean_function, py::arg("name") = "", R"(
        Get the boolean function associated with a specific name. This name can for example be an output pin of the gate or a defined functionality like "reset".
        If name is empty, the function of the first output pin is returned. If there is no function for the given name, the constant 'X' is returned.

        :param str name: The function name.
        :returns: The boolean function.
        :rtype: hal_py.boolean_function
)");

py_gate.def_property_readonly("boolean_functions", [](const std::shared_ptr<gate>& g){return g->get_boolean_functions();}, R"(
        A map from function name to boolean function for all boolean functions associated with this gate.

        :rtype: dict[str,hal_py.boolean_function]
)");

py_gate.def("get_boolean_functions", &gate::get_boolean_functions, py::arg("only_custom_functions") = false, R"(
        Get a map from function name to boolean function for all boolean functions associated with this gate.

        :param bool only_custom_functions: If true, this returns only the functions which were set via set_boolean_function.
        :returns: A map from function name to function.
        :rtype: dict[str,hal_py.boolean_function]
)");

py_gate.def("add_boolean_function", &gate::add_boolean_function, py::arg("name"), py::arg("func"), R"(
        Add the boolean function with the specified name only for this gate.

        :param str name:  The function name, usually an output port.
        :param hal_py.boolean_function func:  The function.
)");

py_gate.def("mark_vcc_gate", &gate::mark_vcc_gate, R"(
        Mark this gate as a global vcc gate.

        :returns: True on success.
        :rtype: bool
)");

py_gate.def("mark_gnd_gate", &gate::mark_gnd_gate, R"(
        Mark this gate as a global gnd gate.

        :returns: True on success.
        :rtype: bool
)");

py_gate.def("unmark_vcc_gate", &gate::unmark_vcc_gate, R"(
        Unmark this gate as a global vcc gate.

        :returns: True on success.
        :rtype: bool
)");

py_gate.def("unmark_gnd_gate", &gate::unmark_gnd_gate, R"(
        Unmark this gate as a global gnd gate.

        :returns: True on success.
        :rtype: bool
)");

py_gate.def("is_vcc_gate", &gate::is_vcc_gate, R"(
        Checks whether this gate is a global vcc gate.

        :returns: True if the gate is a global vcc gate.
        :rtype: bool
)");

py_gate.def("is_gnd_gate", &gate::is_gnd_gate, R"(
        Checks whether this gate is a global gnd gate.

        :returns: True if the gate is a global gnd gate.
        :rtype: bool
)");

py_gate.def_property_readonly("input_pins", &gate::get_input_pins, R"(
        A list of all input pin types of the gate.

        :type: list[str]
)");

py_gate.def("get_input_pins", &gate::get_input_pins, R"(
        Get a list of all input pin types of the gate.

        :returns: A list of input pin types.
        :rtype: list[str]
)");

py_gate.def_property_readonly("output_pins", &gate::get_output_pins, R"(
        A list of all output pin types of the gate.

        :type: list[str]
)");

py_gate.def("get_output_pins", &gate::get_output_pins, R"(
        Get a list of all output pin types of the gate.

        :returns: A list of output pin types.
        :rtype: list[str]
)");

py_gate.def_property_readonly("fan_in_nets", &gate::get_fan_in_nets, R"(
        A set of all fan-in nets of the gate, i.e. all nets that are connected to one of the input pins.

        :type: set[hal_py.net]
)");

py_gate.def("get_fan_in_nets", &gate::get_fan_in_nets, R"(
        Get a set of all fan-in nets of the gate, i.e. all nets that are connected to one of the input pins.

        :returns: A set of all connected input nets.
        :rtype: set[hal_py.net]
)");

py_gate.def("get_fan_in_net", &gate::get_fan_in_net, py::arg("pin_type"), R"(
        Get the fan-in net which is connected to a specific input pin.

        :param str pin_type: The input pin type.
        :returns: The connected input net.
        :rtype: hal_py.net
)");

py_gate.def_property_readonly("fan_out_nets", &gate::get_fan_out_nets, R"(
        A set of all fan-out nets of the gate, i.e. all nets that are connected to one of the output pins.

        :returns: A set of all connected output nets.
        :rtype: set[hal_py.net]
)");

py_gate.def("get_fan_out_nets", &gate::get_fan_out_nets, R"(
        Get a set of all fan-out nets of the gate, i.e. all nets that are connected to one of the output pins.

        :returns: A set of all connected output nets.
        :rtype: set[hal_py.net]
)");

py_gate.def("get_fan_out_net", &gate::get_fan_out_net, py::arg("pin_type"), R"(
        Get the fan-out net which is connected to a specific output pin.

        :param str pin_type: The output pin type.
        :returns: The connected output net.
        :rtype: hal_py.net
)");

py_gate.def_property_readonly("unique_predecessors", [](const std::shared_ptr<gate>& g){ return g->get_unique_predecessors();}, R"(
        A set of all unique predecessor gates of the gate.

        :type: list[hal_py.gate]
)");

py_gate.def("get_unique_predecessors", &gate::get_unique_predecessors, py::arg("filter") = nullptr, R"(
        Get a set of all unique predecessor endpoints of the gate filterable by the gate's input pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A set of unique predecessors endpoints.
        :rtype: set[hal_py.endpoint]
)");

py_gate.def_property_readonly("predecessors", [](const std::shared_ptr<gate>& g){ return g->get_predecessors();}, R"(
        A list of all all direct predecessor endpoints of the gate.

        :type: list[hal_py.endpoint]
)");

py_gate.def("get_predecessors", &gate::get_predecessors, py::arg("filter") = nullptr, R"(
        Get a list of all direct predecessor endpoints of the gate filterable by the gate's input pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A list of predecessors endpoints.
                :rtype: list[hal_py.endpoint]
)");

py_gate.def("get_predecessor", &gate::get_predecessor, py::arg("which_pin"), R"(
        Get the direct predecessor endpoint of the gate connected to a specific input pin and filterable by a specific gate type.

        :param str which_pin: The input pin type of the this gate. Leave empty for no filtering.
        :returns: The predecessor endpoint.
        :rtype: hal_py.endpoint
)");

py_gate.def_property_readonly("unique_successors", [](const std::shared_ptr<gate>& g){ return g->get_unique_successors();}, R"(
        A set of all unique successor gates of the gate.

        :type: list[hal_py.gate]
)");

py_gate.def("get_unique_successors", &gate::get_unique_successors, py::arg("filter") = nullptr, R"(
        Get a set of all unique successors of the gate filterable by the gate's output pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A set of unique successor endpoints.
        :rtype: set[hal_py.endpoint]
)");

py_gate.def_property_readonly("successors", [](const std::shared_ptr<gate>& g){ return g->get_successors();}, R"(
        A list of all direct successor endpoints of the gate.

        :type: list[hal_py.endpoint]
)");

py_gate.def("get_successors", &gate::get_successors, py::arg("filter") = nullptr, R"(
        Get a list of all direct successor endpoints of the gate filterable by the gate's output pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A list of successor endpoints.
        :rtype: list[hal_py.endpoint]
)");


py::class_<net, data_container, std::shared_ptr<net>>py_net(m, "net", R"(Net class containing information about a net including its source and destination.)");

py_net.def_property_readonly("id", &net::get_id, R"(
        The unique id of the net.

        :type: int
)");

py_net.def("get_id", &net::get_id, R"(
        Get the unique id of the net.

        :returns: The net's id.
        :rtype: int
)");

py_net.def_property("name", &net::get_name, &net::set_name, R"(
        The name of the net.

        :type: str
)");

py_net.def("get_name", &net::get_name, R"(
        Get the name of the net.

        :returns: The name.
        :rtype: str
)");

py_net.def("set_name", &net::set_name, py::arg("name"), R"(
        Set the name of the net.

        :param str name: The new name.
)");

// FIXME docs need update, "get_gate()" is not a type
py_net.def("add_source", py::overload_cast<const std::shared_ptr<gate>&, const std::string&>(&net::add_source), py::arg("gate"), py::arg("pin"), R"(
        Sets the source of this net to a gate's output pin.

        :param hal_py.get_gate() gate: The source gate.
        :param str pin: The pin of the source gate.
        :returns: True on succes.
        :rtype: bool
)");

// FIXME docs need update
py_net.def("add_source", py::overload_cast<const endpoint&>(&net::add_source), py::arg("endpoint"), R"(
        Sets the source of this net to a gate's output pin.

        :param hal_py.endpoint endpoint: The source endpoint.
        :returns: True on success.
        :rtype: bool
)");

py_net.def("remove_source", py::overload_cast<const std::shared_ptr<gate>&, const std::string&>(&net::remove_source), py::arg("gate"), py::arg("pin"), R"(
        Removes the source of the net.
        :param hal_py.get_gate() gate: The source gate.
        :param str pin: The pin of the source gate.
        :returns: True on succes.
        :rtype: bool
)");

py_net.def("remove_source", py::overload_cast<const endpoint&>(&net::remove_source), py::arg("endpoint"), R"(
        Removes the source of the net.

        :param hal_py.endpoint endpoint: The source endpoint.
        :returns: True on success.
        :rtype: bool
)");

py_net.def("is_a_source", py::overload_cast<const std::shared_ptr<gate>&, const std::string&>(&net::is_a_source, py::const_), py::arg("gate"), py::arg("pin"), R"(
        Check whether a gate's input pin is a source of this net.

        :param gate: The source gate.
        :type gate: hal_py.get_gate()
        :param str pin: The input pin of the gate. Leave empty if the pin does not matter.
        :returns: True if the input's pin is a source.
        :rtype: bool
)");

py_net.def("is_a_source", py::overload_cast<const endpoint&>(&net::is_a_source, py::const_), py::arg("endpoint"), R"(
        Check whether a gate's input pin is a source of this net.

        :param endpoint: The input endpoint.
        :type endpoint: hal_py.endpoint
        :returns: True if the input's pin is a source.
        :rtype: bool
)");

py_net.def_property_readonly("num_of_sources", &net::get_num_of_sources, R"(
        The number of sources of the net.

        :type: int
)");

py_net.def("get_num_of_sources", &net::get_num_of_sources, R"(
        Get the number of sources.

        :returns: The number of sources of this net.
        :rtype: int
)");

py_net.def_property_readonly("sources", [](const std::shared_ptr<net>& n){return n->get_sources();}, R"(
        Get the vector of sources of the net.

        :type: set[hal_py.net]
)");

py_net.def("get_sources", &net::get_sources, py::arg("filter") = nullptr, R"(
        Get the vector of sources of the net.

        :param filter: a filter for endpoints.
        :returns: A list of source endpoints.
        :rtype: list[hal_py.endpoint]
)");

py_net.def("get_source", &net::get_source, R"(
        Get the (first) source of the net specified by type.
        If there is no source, the gate of the returned endpoint is null.

        :param str gate_type: The desired source gate type.
        :returns: The source endpoint.
        :rtype: hal_py.endpoint
)");

// FIXME "get_gate()" is not a type
py_net.def("add_destination", py::overload_cast<const std::shared_ptr<gate>&, const std::string&>(&net::add_destination), py::arg("gate"), py::arg("pin"), R"(
        Add a destination to this net.

        :param gate: The destination gate.
        :type gate: hal_py.get_gate()
        :param str pin: The input pin of the gate.
        :returns: True on succes
        :rtype: bool
)");
        
py_net.def("add_destination", py::overload_cast<const endpoint&>(&net::add_destination), py::arg("destination"), R"(
        Add a destination to this net.

        :param destination: The destination endpoint.
        :type destination: hal_py.endpoint
        :returns: True on succes.
        :rtype: bool
)");

// FIXME "get_gate()" is not a type
py_net.def("remove_destination", py::overload_cast<const std::shared_ptr<gate>&, const std::string&>(&net::remove_destination), py::arg("gate"), py::arg("pin"), R"(
        Remove a destination from this net.

        :param gate: The destination gate.
        :type gate: hal_py.get_gate()
        :param str pin: The input pin of the gate. Leave empty if the pin does not matter.
        :returns: True on succes
        :rtype: bool
)");

py_net.def("remove_destination", py::overload_cast<const endpoint&>(&net::remove_destination), py::arg("destination"), R"(
        Remove a destination from this net.

        :param destination: The destination endpoint.
        :type destination: hal_py.endpoint
        :returns: True on succes.
        :rtype: bool
)");

py_net.def("is_a_destination", py::overload_cast<const std::shared_ptr<gate>&, const std::string&>(&net::is_a_destination, py::const_), py::arg("gate"), py::arg("pin"), R"(
        Check whether a gate's input pin is a destination of this net.

        :param gate: The destination gate.
        :type gate: hal_py.get_gate()
        :param str pin: The input pin of the gate. Leave empty if the pin does not matter.
        :returns: True if the input's pin is a destination.
        :rtype: bool
)");

py_net.def("is_a_destination", py::overload_cast<const endpoint&>(&net::is_a_destination, py::const_), py::arg("endpoint"), R"(
        Check whether a gate's input pin is a destination of this net.

        :param endpoint: The input endpoint.
        :type endpoint: hal_py.endpoint
        :returns: True if the input's pin is a destination.
        :rtype: bool
)");
        
py_net.def_property_readonly("num_of_destinations", &net::get_num_of_destinations, R"(
        The number of destinations of the net.

        :type: int
)");
        
py_net.def("get_num_of_destinations", &net::get_num_of_destinations, R"(
        Get the number of destinations.

        :returns: The number of destinations of this net.
        :rtype: int
)");
        
py_net.def_property_readonly("destinations", [](const std::shared_ptr<net>& n){return n->get_destinations();}, R"(
        Get the vector of destinations of the net.

        :type: set[hal_py.net]
)");

py_net.def("get_destinations", &net::get_destinations, py::arg("filter") = nullptr, R"(
        Get the vector of destinations of the net.

        :param filter: a filter for endpoints.
        :returns: A list of destination endpoints.
        :rtype: list[hal_py.endpoint]
)");

py_net.def("is_unrouted", &net::is_unrouted, R"(
        Check whether the net is routed, i.e. it has no source or no destinations.

        :returns: True if the net is unrouted.
        :rtype: bool
)");

py_net.def("mark_global_input_net", &net::mark_global_input_net, R"(
        Mark this net as a global input net.

        :returns: True on success.
        :rtype: bool
)");
        
py_net.def("mark_global_output_net", &net::mark_global_output_net, R"(
        Mark this net as a global output net.

        :returns: True on success.
        :rtype: bool
)");

py_net.def("unmark_global_input_net", &net::unmark_global_input_net, R"(
        Unmark this net as a global input net.

        :returns: True on success.
        :rtype: bool
)");

py_net.def("unmark_global_output_net", &net::unmark_global_output_net, R"(
        Unmark this net as a global output net.

:returns: True on success.
:rtype: bool
)");

py_net.def("is_global_input_net", &net::is_global_input_net, R"(
        Checks whether this net is a global input net.

        :returns: True if the net is a global input net.
        :rtype: bool
)");

py_net.def("is_global_output_net", &net::is_global_output_net, R"(
        Checks whether this net is a global output net.

        :returns: True if the net is a global output net.
        :rtype: bool
)");


py::class_<module, std::shared_ptr<module>, data_container> py_module(m, "module", R"(Module class containing information about a module including its gates, submodules, and parent module.)");
        
py_module.def_property_readonly("id", &module::get_id, R"(
        The unique ID of the module object.

        :type: int
)");

py_module.def("get_id", &module::get_id, R"(
        Returns the unique ID of the module object.

        :returns: The unique id.
        :rtype: int
)");

py_module.def_property("name", &module::get_name, &module::set_name, R"(
        The name of the module.

        :type: str
)");

py_module.def("get_name", &module::get_name, R"(
        Gets the module's name.

        :returns: The name.
        :rtype: str
)");
        
py_module.def("set_name", &module::set_name, py::arg("name"), R"(
        Sets the module's name.

        :param str name: The new name.
)");

py_module.def_property("parent_module", &module::get_parent_module, &module::set_parent_module, R"(
        The parent module of this module. Set to None for the top module.

        :type: hal_py.module or None
)");

py_module.def("get_parent_module", &module::get_parent_module, R"(
        Get the parent of this module.
        This returns None for the top module.

        :returns: The parent module.
        :rtype: hal_py.module or None
)");

py_module.def("set_parent_module", &module::set_parent_module, py::arg("new_parent"), R"(
        Set the parent of this module.
        If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.

        :returns: True if the parent was changed
        :rtype: bool
)");

py_module.def_property_readonly("submodules", [](const std::shared_ptr<module>& mod) { return mod->get_submodules(); }, R"(
        A set of all direct submodules of this module.

        :type: set[hal_py.module]
)");

py_module.def("get_submodules", &module::get_submodules, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
        Get all direct submodules of this module.
        If recursive parameter is true, all indirect submodules are also included.

        :param lambda filter: Filter for the modules.
        :param bool recursive: Look into submodules aswell.
        :returns: The set of submodules:
        :rtype: set[hal_py.module]
)");

py_module.def("contains_module", &module::contains_module, py::arg("other"), py::arg("recusive") = false, R"(
        Checks whether another module is a submodule of this module. If \p recursive is true, all indirect submodules are also included.

        :param other: Other module to check
        :param recursive: Look into submodules too
        :type other: hal_py.module
        :type recursive: bool
        :returns: True if the module is a submodule
        :rtype: bool
)");

py_module.def_property_readonly("netlist", &module::get_netlist, R"(
        The netlist this module is associated with.

        :type: hal_py.netlist
)");

py_module.def("get_netlist", &module::get_netlist, R"(
        Get the netlist this module is associated with.

        :returns: The netlist.
        :rtype: hal_py.netlist
)");
        
py_module.def_property_readonly("input_nets", &module::get_input_nets, R"(
        The input nets to this module.

        :type: set[hal_py.net]
)");

py_module.def("get_input_nets", &module::get_input_nets, R"(
        Get the input nets to this module.
        A module input net is either a global input to the netlist or has a source outside of the module.

        :returns: A set of module input nets.
        :rtype: set[hal_py.net]
)");
        
py_module.def_property_readonly("output_nets", &module::get_output_nets, R"(
        The output nets to this module.

        :type: set[hal_py.net]
)");
        
py_module.def("get_output_nets", &module::get_output_nets, R"(
        Get the output nets to this module.
        A module output net is either a global output of the netlist or has a destination outside of the module.

        :returns: The set of module output nets.
        :rtype: set[hal_py.net]
)");
        
py_module.def_property_readonly("internal_nets", &module::get_internal_nets, R"(
        The internal nets to this module.

        :type: set[hal_py.net]
)");

py_module.def("get_internal_nets", &module::get_internal_nets, R"(
        Get the internal nets to this module. A net is internal if its source and at least one output are inside the module.
        Therefore it may contain some nets that are also regarded as output nets.

        :returns: The set of internal nets.
        :rtype: set[hal_py.net]
)");

// FIXME "get_gate()" is not a type
py_module.def_property_readonly("gates", [](const std::shared_ptr<module>& mod) { return mod->get_gates(); }, R"(
        The set of all gates belonging to the module.

        :type: set[hal_py.get_gate()]
)");

// FIXME "get_gate()" is not a type
py_module.def("get_gates", &module::get_gates, py::arg("filter") = nullptr, py::arg("recursive") = false, R"(
        Returns all associated gates. You can filter with the optional parameters. If the parameter recursive is true, all submodules are searched aswell.

        :param lambda filter: Filter for the gates.
        :param bool recursive: Look into submodules too.
        :returns: A set of gates.
        :rtype: set[hal_py.get_gate()]
)");

// FIXME "get_gate()" is not a type
py_module.def("get_gate_by_id", &module::get_gate_by_id, py::arg("id"), py::arg("recursive") = false, R"(
        Get a gate specified by id. If recursive parameter is true, all submodule are searched aswell.

        :param int id: The gate's id.
        :param bool recursive: Look into submodules too.
        :returns: The gate or None.
        :rtype: hal_py.get_gate() or None
)");

// FIXME "get_gate()" is not a type
py_module.def("assign_gate", &module::assign_gate, py::arg("gate"), R"(
        Moves a gate into this module. The gate is removed from its previous module in the process.

        :param hal_py.get_gate() gate: The gate to add.
        :returns: True on success.
        :rtype: bool
)");

// FIXME "get_gate()" is not a type
py_module.def("remove_gate", &module::remove_gate, py::arg("gate"), R"(
        Removes a gate from the module object.

        :param hal_py.get_gate() gate: The gate to remove.
        :returns: True on success.
        :rtype: bool
)");

// FIXME "get_gate()" is not a type
py_module.def("contains_gate", &module::contains_gate, py::arg("gate"), py::arg("recusive") = false, R"(
        Checks whether a gate is in the module. If \p recursive is true, all submodules are searched as well.

        :param hal_py.get_gate() gate: The gate to search for.
        :param bool recursive: Look into submodules too
        :returns: True if the gate is in the object.
        :rtype: bool
)");

py_module.def("set_input_port_name", &module::set_input_port_name, py::arg("input_net"), py::arg("port_name"), R"(
        Set the name of the port corresponding to the specified input net to the given string.

        :param hal_py.net input_net: The input net.
        :param str port_name: The port name.
)");

py_module.def("set_output_port_name", &module::set_output_port_name, py::arg("output_net"), py::arg("port_name"), R"(
        Set the name of the port corresponding to the specified output net to the given string.

        :param hal_py.net output_net: The output net.
        :param str port_name: The port name.
)");

py_module.def("get_input_port_name", &module::get_input_port_name, py::arg("input_net"), R"(
        Get the name of the port corresponding to the specified input net.
        
        :param hal_py.net input_net: The input net.
        :returns: The port name.
        :rtype: str
)");

py_module.def("get_output_port_name", &module::get_output_port_name, py::arg("output_net"), R"(
        Get the name of the port corresponding to the specified output net.
        
        :param hal_py.net output_net: The output net.
        :returns: The port name.
        :rtype: str
)");

py_module.def("get_input_port_names", &module::get_input_port_names, R"(
        Get the mapping of all input nets to their corresponding port names.
        
        :returns: The map from input net to port name.
        :rtype: dict[hal_py.net,str]
)");

py_module.def("get_output_port_names", &module::get_output_port_names, R"(
        Get the mapping of all output nets to their corresponding port names.
        
        :returns: The map from output net to port name.
        :rtype: dict[hal_py.net,str]
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


auto py_plugin_manager =  m.def_submodule("plugin_manager");

py_plugin_manager.def("get_plugin_names", &plugin_manager::get_plugin_names, R"(
        Get the names of all loaded plugins.

        :returns: The set of plugin names.
        :rtype: set(str)
)");

py_plugin_manager.def("load_all_plugins", &plugin_manager::load_all_plugins, py::arg("file_names") = std::vector<hal::path>(), R"(
        Load all plugins in the specified directories. If \p directory is empty, the default directories will be searched.

        :param file_names: A list of directory paths.
        :type file_names: hal_py.hal_path
        :returns: True on success.
        :rtype: bool
)");

py_plugin_manager.def("load", &plugin_manager::load, py::arg("plugin_name"), py::arg("file_name"), R"(
        Load a single plugin by specifying a name and the file path.

        :param str plugin_name: The desired name, unique in the framework.
        :param file_name: The path to the plugin file.
        :type file_name: hal_py.hal_path
        :returns: True on success.
        :rtype: bool
)");

py_plugin_manager.def("unload_all_plugins", &plugin_manager::unload_all_plugins, R"(
        Releases all plugins and associated resources.

        :returns: True on success.
        :rtype: bool
)");

py_plugin_manager.def("get_plugin_instance", [](const std::string& plugin_name) -> std::shared_ptr<i_base> { return plugin_manager::get_plugin_instance<i_base>(plugin_name, true); }, py::arg("plugin_name"), R"(
        Gets the basic interface for a plugin specified by name.

        :param str plugin_name: The plugin name.
        :returns: The interface base for the plugin.
        :rtype: hal_py.i_base
)");


py::class_<i_base, std::shared_ptr<i_base>, Pyi_base> py_i_base(m, "i_base");

py_i_base.def_property_readonly("name", &i_base::get_name, R"(
        The name of the plugin.

        :type: str
)");

py_i_base.def("get_name", &i_base::get_name, R"(
        Get the name of the plugin.

        :returns: Plugin name.
        :rtype: str
)");

py_i_base.def_property_readonly("version", &i_base::get_version, R"(
        The version of the plugin.

        :type: str
)");
        
py_i_base.def("get_version", &i_base::get_version, R"(
        Get the version of the plugin.

        :returns: Plugin version.
        :rtype: str
)");


py::class_<i_gui, std::shared_ptr<i_gui>, Pyi_gui> py_i_gui(m, "i_gui");

py_i_gui.def("exec", &i_gui::exec, py::arg("netlist"), R"(
        Generic call to run the GUI.

        :param netlist: The netlist object for the GUI.
        :type netlist: hal_py.netlist
        :returns: True on success.
        :rtype: bool
)");

py::class_<boolean_function> py_boolean_function(m, "boolean_function", R"(Boolean function class.)");

py::enum_<boolean_function::value>(py_boolean_function, "value", R"(
        Represents the logic value that a boolean function operates on. Available are: X, ZERO, and ONE.)")
        .value("X", boolean_function::value::X)
        .value("ZERO", boolean_function::value::ZERO)
        .value("ONE", boolean_function::value::ONE)
        .export_values();

py_boolean_function.def(py::init<>(), R"(
        Constructor for an empty function. Evaluates to X (undefined). Combining a function with an empty function leaves the other one unchanged.
)");

py_boolean_function.def(py::init<const std::string&>(), py::arg("variable"), R"(
        Constructor for a variable, usable in other functions. Variable name must not be empty.

        :param str variable_name: Name of the variable.
)");

py_boolean_function.def(py::init<boolean_function::value>(), R"(
        Constructor for a constant, usable in other functions.
        The constant can be either X, Zero, or ONE.

        :param hal_py.value constant: A constant value.
)");

py_boolean_function.def("substitute", py::overload_cast<const std::string&, const std::string&>(&boolean_function::substitute, py::const_), py::arg("old_variable_name"), py::arg("new_variable_name"), R"(
        Substitutes a variable with another variable (i.e., variable renaming).
        Applies to all instances of the variable in the function.
        This is just a shorthand for the generic substitute function.

        :param str old_variable_name:  The old variable to substitute
        :param str new_variable_name:  The new variable name
        :returns: The new boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def("substitute", py::overload_cast<const std::string&, const boolean_function&>(&boolean_function::substitute, py::const_), py::arg("variable_name"), py::arg("function"), R"(
        Substitutes a variable with another function (can again be a single variable).
        Applies to all instances of the variable in the function.

        :param str variable_name:  The variable to substitute
        :param hal_py.boolean_function function:  The function to take the place of the varible
        :returns: The new boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def("evaluate", &boolean_function::evaluate, py::arg("inputs") = std::map<std::string, boolean_function::value>(), R"(
        Evaluates the function on the given inputs and returns the result.

        :param dict[str,value] inputs:  A map from variable names to values.
        :returns: The value that the function evaluates to.
        :rtype: hal_py.value
)");

py_boolean_function.def("__call__", [](const boolean_function& f, const std::map<std::string, boolean_function::value>& values) { return f(values); });

py_boolean_function.def("is_constant_one", &boolean_function::is_constant_one, R"(
        Checks whether the function constantly outputs ONE.

        :returns: True if function is constant ONE, false otherwise.
        :rtype: bool
)");

py_boolean_function.def("is_constant_zero", &boolean_function::is_constant_zero, R"(
        Checks whether the function constantly outputs ZERO.

        :returns: True if function is constant ZERO, false otherwise.
        :rtype: bool
)");

py_boolean_function.def("is_empty", &boolean_function::is_empty, R"(
        Checks whether the function is empty.

        :returns: True if function is empty, false otherwise.
        :rtype: bool
)");

py_boolean_function.def_property_readonly("variables", &boolean_function::get_variables, R"(
        A set of all variable names used in this boolean function.

        :type: set[str]
)");

py_boolean_function.def("get_variables", &boolean_function::get_variables, R"(
        Get all variable names used in this boolean function.

        :returns: A set of all variable names.
        :rtype: set[str]
)");

py_boolean_function.def_static("from_string", &boolean_function::from_string, py::arg("expression"), py::arg("variable_names"), R"(
        Parse a function from a string representation.
        Supported operators are  NOT (\"!\", \"'\"), AND (\"&\", \"*\", \" \"), OR (\"|\", \"+\"), XOR (\"^\") and brackets (\"(\", \")\").
        Operator precedence is ! > & > ^ > |

        Since, for example, '(' is interpreted as a new term, but might also be an intended part of a variable,
        a vector of known variable names can be supplied, which are extracted before parsing.
        If there is an error during bracket matching, X is returned for that part.

        :param str expression: String containing a boolean function.
        :param str variable_names: Names of variables to help resolve problematic functions
        :returns: The boolean function extracted from the string.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def("__str__", [](const boolean_function& f) { return f.to_string(); });

py_boolean_function.def(py::self & py::self, R"(
        Combines two boolean functions using an AND operator.

        :returns: The combined boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def(py::self | py::self, R"(
        Combines two boolean functions using an OR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def(py::self ^ py::self, R"(
        Combines two boolean functions using an XOR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def(py::self &= py::self, R"(
        Combines two boolean functions using an AND operator.

        :returns: The combined boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def(py::self |= py::self, R"(
        Combines two boolean functions using an OR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def(py::self ^= py::self, R"(
        Combines two boolean functions using an XOR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def(!py::self, R"(
        Negates the boolean function.

        :returns: The negated boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def(py::self == py::self, R"(
        Tests whether two boolean functions are equal.

        :returns: True when both boolean functions are equal, false otherwise.
        :rtype: bool
)");

py_boolean_function.def(py::self != py::self, R"(
        Tests whether two boolean functions are unequal.

        :returns: True when both boolean functions are unequal, false otherwise.
        :rtype: bool
)");

py_boolean_function.def("is_dnf", &boolean_function::is_dnf, R"(
        Tests whether the function is in DNF.

        :returns: True if in DNF, false otherwise.
        :rtype: bool
)");

py_boolean_function.def("to_dnf", &boolean_function::to_dnf, R"(
        Gets the plain DNF representation of the function.

        :returns: The DNF as a boolean function.
        :rtype: hal_py.boolean_function
)");
       
py_boolean_function.def("optimize", &boolean_function::optimize, R"(
        Optimizes the function by first converting it to DNF and then applying the Quine-McCluskey algorithm.

        :returns: The optimized boolean function.
        :rtype: hal_py.boolean_function
)");

py_boolean_function.def("get_truth_table", &boolean_function::get_truth_table, py::arg("ordered_variables") = std::vector<std::string>(), R"(
        Get the truth table outputs of the function.
        WARNING: Exponential runtime in the number of variables!

        Output is the vector of output values when walking the truth table in ascending order.

        If ordered_variables is empty, all included variables are used and ordered alphabetically.

        :param list[str] ordered_variables: Specific order in which the inputs shall be structured in the truth table.
        :returns: The vector of output values.
        :rtype: list[value]
)");

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}