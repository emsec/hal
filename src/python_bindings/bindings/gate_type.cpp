#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_init(py::module& m)
    {
        py::class_<GateType, RawPtrWrapper<GateType>> py_gate_type(m, "GateType", R"(
        A gate type contains information about its internals such as input and output pins as well as its Boolean functions.
)");

        py::enum_<GateType::BaseType>(py_gate_type, "BaseType", R"(
            Defines the base type of a gate type.
        )")

            .value("combinational", GateType::BaseType::combinational, R"(Represents a combinational gate type.)")
            .value("lut", GateType::BaseType::lut, R"(Represents a combinational LUT gate type.)")
            .value("ff", GateType::BaseType::ff, R"(Represents a sequential FF gate type.)")
            .value("latch", GateType::BaseType::latch, R"(Represents a sequential latch gate type.)")
            .value("ram", GateType::BaseType::ram, R"(Represents a sequential RAM gate type.)")
            .export_values();

        py::enum_<GateType::PinDirection>(py_gate_type, "PinDirection", R"(
            Defines the direction of a pin.
        )")
            .value("input", GateType::PinDirection::input, R"(Input pin.)")
            .value("output", GateType::PinDirection::output, R"(Output pin.)")
            .value("inout", GateType::PinDirection::inout, R"(Inout pin.)")
            .value("internal", GateType::PinDirection::internal, R"(Internal pin.)")
            .export_values();

        py::enum_<GateType::PinType>(py_gate_type, "PinType", R"(Defines the type of a pin.)")
            .value("none", GateType::PinType::none, R"(Default pin.)")
            .value("power", GateType::PinType::power, R"(Power pin.)")
            .value("ground", GateType::PinType::ground, R"(Ground pin.)")
            .value("lut", GateType::PinType::lut, R"(Pin that generates output from LUT initialization string.)")
            .value("state", GateType::PinType::state, R"(Pin that generates output from internal state.)")
            .value("neg_state", GateType::PinType::neg_state, R"(Pin that generates output from negated internal state.)")
            .value("clock", GateType::PinType::clock, R"(Clock pin.)")
            .value("enable", GateType::PinType::enable, R"(Enable pin.)")
            .value("set", GateType::PinType::set, R"(Set pin.)")
            .value("reset", GateType::PinType::reset, R"(Reset pin.)")
            .value("data", GateType::PinType::data, R"(Data pin.)")
            .value("address", GateType::PinType::address, R"(Address pin.)")
            .export_values();

        py::enum_<GateType::ClearPresetBehavior>(py_gate_type, "ClearPresetBehavior", R"(
            Defines the behavior of the gate type in case both clear and preset are active at the same time.
        )")
            .value("U", GateType::ClearPresetBehavior::U, R"(Default value when no behavior is specified.)")
            .value("L", GateType::ClearPresetBehavior::L, R"(Set the internal state to '0'.)")
            .value("H", GateType::ClearPresetBehavior::H, R"(Set the internal state to '1'.)")
            .value("N", GateType::ClearPresetBehavior::N, R"(Do not change the internal state.)")
            .value("T", GateType::ClearPresetBehavior::T, R"(Toggle, i.e., invert the internal state.)")
            .value("X", GateType::ClearPresetBehavior::X, R"(Set the internal state to 'X'.)")
            .export_values();

        py_gate_type.def(py::init<const std::string&, GateType::BaseType>(), py::arg("name"), py::arg("base_type"), R"(
            Construct a new gate type by specifying its name and base type.

            :param str name: The name of the gate type.
            :param hal_py.GateType.BaseType base_type: The base type.
        )");

        py_gate_type.def_property_readonly("id", &GateType::get_id, R"(
            :type: The unique ID of the gate type.
        )");

        py_gate_type.def("get_id", &GateType::get_id, R"(
            Get the unique ID of the gate type.

            :returns: The unique ID of the gate type.
            :rtype: int
        )");

        py_gate_type.def_property_readonly("name", &GateType::get_name, R"(
            The name of the gate type.

            :type: str
        )");

        py_gate_type.def("get_name", &GateType::get_name, R"(
            Get the name of the gate type.

            :returns: The name of the gate type.
            :rtype: str
        )");

        py_gate_type.def_property_readonly("base_type", &GateType::get_base_type, R"(
            The base type of the gate typ, which can be either 'combinatorial', 'lut', ‘ff', or 'latch'.

            :type: hal_py.GateType.BaseType
        )");

        py_gate_type.def("get_base_type", &GateType::get_base_type, R"(
            Get the base type of the gate type.
            The base type can be either 'combinatorial', 'lut', ‘ff', or 'latch'.

            :returns: The base type of the gate type.
            :rtype: hal_py.GateType.BaseType
        )");

        py_gate_type.def(
            "__str__", [](const GateType& gt) { return gt.to_string(); }, R"(
            Get a string describing the given gate type object.

            :returns: A string describing the gate type.
        )");

        py_gate_type.def(py::self == py::self, R"(
            Check whether two gate types are equal.

            :returns: True if both gate types are equal, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def(py::self != py::self, R"(
            Check whether two gate types are unequal.

            :returns: True if both gate types are unequal, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def("add_input_pin", &GateType::add_input_pin, py::arg("pin"), R"(
            Add an input pin to the gate type.

            :param str pin: The name of the input pin to add.
        )");

        py_gate_type.def("add_input_pins", &GateType::add_input_pins, py::arg("pins"), R"(
            Add a list of input pins to the gate type.

            :param list[str] pins: The list of names of input pins to add.
        )");

        py_gate_type.def_property_readonly("input_pins", &GateType::get_input_pins, R"(
            A list of input pin names of the gate type.

            :type: list[str]
        )");

        py_gate_type.def("get_input_pins", &GateType::get_input_pins, R"(
            Get a list of input pins of the gate type.

            :returns: A list of input pin names of the gate type.
            :rtype: list[str]
        )");

        py_gate_type.def("add_output_pin", &GateType::add_output_pin, py::arg("pin"), R"(
            Add an output pin to the gate type.

            :param str pin: The name of the output pin to add.
        )");

        py_gate_type.def("add_output_pins", &GateType::add_output_pins, py::arg("pins"), R"(
            Add a list of output pins to the gate type.

            :param list[str] pins: The list of names of output pins to add.
        )");

        py_gate_type.def_property_readonly("output_pins", &GateType::get_output_pins, R"(
            A list of output pin names of the gate type.

            :type: list[str]
        )");

        py_gate_type.def("get_output_pins", &GateType::get_output_pins, R"(
            Get a list of output pins of the gate type.

            :returns: A list of output pin names of the gate type.
            :rtype: list[str]
        )");

        py_gate_type.def("assign_input_pin_group", &GateType::assign_input_pin_group, py::arg("group_name"), py::arg("index_to_pin"), R"(
            Assign existing input pins to a input pin group.

            :param str group_name: The name of the input pin group.
            :param dict[int,str] index_to_pin: A dict from pin index to input pin name.
        )");

        py_gate_type.def("assign_input_pin_groups", &GateType::assign_input_pin_groups, py::arg("pin_groups"), R"(
            Assign existing input pins to multiple input pin groups.

            :param dict[str,dict[int,str]] pin_groups: A dict from pin group names to a dict from pin indices to pin names.
        )");

        py_gate_type.def_property_readonly("input_pin_groups", &GateType::get_input_pin_groups, R"(
            All input pin groups of the gate type as a dict from pin group names to a dict from pin indices to pin names.

            :type: dict[str,dict[int,str]]
        )");

        py_gate_type.def("get_input_pin_groups", &GateType::get_input_pin_groups, R"(
            Get all input pin groups of the gate type.

            :returns: A dict from pin group names to a dict from pin indices to pin names.
            :rtype: dict[str,dict[int,str]]
        )");

        py_gate_type.def("assign_output_pin_group", &GateType::assign_output_pin_group, py::arg("group_name"), py::arg("index_to_pin"), R"(
            Assign existing output pins to a output pin group.

            :param str group_name: The name of the output pin group.
            :param dict[int,str] index_to_pin: A dict from pin index to output pin name.
        )");

        py_gate_type.def("assign_output_pin_groups", &GateType::assign_output_pin_groups, py::arg("pin_groups"), R"(
            Assign existing output pins to multiple output pin groups.

            :param dict[str,dict[int,str]] pin_groups: A dict from pin group names to a dict from pin indices to pin names.
        )");

        py_gate_type.def_property_readonly("output_pin_groups", &GateType::get_output_pin_groups, R"(
            All output pin groups of the gate type as a dict from pin group names to a dict from pin indices to pin names.

            :type: dict[str,dict[int,str]]
        )");

        py_gate_type.def("get_output_pin_groups", &GateType::get_output_pin_groups, R"(
            Get all output pin groups of the gate type.

            :returns: A dict from pin group names to a dict from pin indices to pin names.
            :rtype: dict[str,dict[int,str]]
        )");

        py_gate_type.def("assign_pin_type", &GateType::assign_pin_type, py::arg("pin"), py::arg("pin_type"), R"(
            Assign a pin type to the given pin. The pin must have been added to the gate type beforehand.
    
            :param str pin: The pin.
            :param hal_py.GateType.PinType pin_type: The pin type to be assigned.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def("get_pin_type", &GateType::get_pin_type, py::arg("pin"), R"(
            Get the pin type of the given pin. If the pin does not exist, the default type 'none' will be returned.
        
            :param str pin: The pin.
            :returns: The pin type.
            :rtype: hal_py.GateType.PinType
        )");

        py_gate_type.def("get_pin_types", &GateType::get_pin_types, R"(
            Get the pin types of all pins as a map.
         
            :returns: A dict from pin to pin type.
            :rtype: dict[str,hal_py.GateType.PinType]
        )");

        py_gate_type.def_property_readonly("pin_types", &GateType::get_pin_types, R"(
            The pin types of all pins as a map.
         
            :type: dict[str,hal_py.GateType.PinType]
        )");

        py_gate_type.def("get_pins_of_type", &GateType::get_pins_of_type, R"(
            Get all pins of the specified pin type.
        
            :param hal_py.GateType.PinType pin_type: The pin type.
            :returns: A list of pins.
            :rtype: list[str]
        )");

        py_gate_type.def("add_boolean_function", &GateType::add_boolean_function, py::arg("pin_name"), py::arg("function"), R"(
            Add a Boolean function with the specified name to the gate type.

            :param str name: The name of the Boolean function.
            :param hal_py.BooleanFunction function: The Boolean function.
        )");

        py_gate_type.def("add_boolean_functions", &GateType::add_boolean_functions, py::arg("functions"), R"(
            Add multiple boolean functions to the gate type.

            :param dict[str,hal_py.BooleanFunction] functions: A dict from Boolean function names to Boolean functions.
        )");

        py_gate_type.def_property_readonly("boolean_functions", &GateType::get_boolean_functions, R"(
            All Boolean functions of the gate type as a dict from Boolean function names to Boolean functions.
            
            :type: dict[str,hal_py.BooleanFunction]
        )");

        py_gate_type.def("get_boolean_functions", &GateType::get_boolean_functions, R"(
            Get all Boolean functions of the gate type.

            :returns: A dict from Boolean function names to Boolean functions.
            :rtype: dict[str,hal_py.BooleanFunction]
        )");

        py_gate_type.def("set_clear_preset_behavior", &GateType::set_clear_preset_behavior, py::arg("cp1"), py::arg("cp2"), R"(
            Set the behavior that describes the internal state when both clear and preset are active at the same time.

            :param hal_py.GateType.ClearPresetBehavior cp1: The value specifying the behavior for the internal state.
            :param hal_py.GateType.ClearPresetBehavior cp2: The value specifying the behavior for the negated internal state.
        )");

        py_gate_type.def("get_clear_preset_behavior", &GateType::get_clear_preset_behavior, R"(
            Get the behavior of the internal state and the negated internal state when both clear and preset are active at the same time.

            :returns: The values specifying the behavior for the internal and negated internal state.
            :rytpe: tuple(hal_py.GateType.ClearPresetBehavior, hal_py.GateType.ClearPresetBehavior)
        )");

        py_gate_type.def_property("config_data_category", &GateType::get_config_data_category, &GateType::set_config_data_category, R"(
            The category in which to find the configuration data associated with this gate type.

            :type: str
        )");

        py_gate_type.def("set_config_data_category", &GateType::set_config_data_category, py::arg("category"), R"(
            Set the category in which to find the configuration data associated with this gate type.

            :param str category:  The data category.
        )");

        py_gate_type.def("get_config_data_category", &GateType::get_config_data_category, R"(
            Get the category in which to find the configuration data associated with this gate type.

            :returns: The data category.
            :rtype: str
        )");

        py_gate_type.def_property("config_data_identifier", &GateType::get_config_data_identifier, &GateType::set_config_data_identifier, R"(
            The identifier used to specify the configuration data associated with this gate type.

            :type: str
        )");

        py_gate_type.def("set_config_data_identifier", &GateType::set_config_data_identifier, py::arg("identifier"), R"(
            Set the identifier used to specify the configuration data associated with this gate type.

            :param str identifier: The data identifier.
        )");

        py_gate_type.def("get_config_data_identifier", &GateType::get_config_data_identifier, R"(
            Get the identifier used to specify the configuration data associated with this gate type.

            :returns: The data identifier.
            :rtype: str
        )");

        py_gate_type.def_property("lut_init_ascending", &GateType::is_lut_init_ascending, &GateType::set_lut_init_ascending, R"(
            For LUT gate types, defines the bit-order of the initialization string. True if ascending bit-order, false otherwise.

            :type: bool
        )");

        py_gate_type.def("set_lut_init_ascending", &GateType::set_lut_init_ascending, py::arg("ascending"), R"(
            For LUT gate types, set the bit-order of the initialization string.

            :param bool ascending: True if ascending bit-order, false otherwise.
        )");

        py_gate_type.def("is_lut_init_ascending", &GateType::is_lut_init_ascending, R"(
            For LUT gate types, get the bit-order of the initialization string.

            :returns: True if ascending bit-order, false otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
