#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_init(py::module& m)
    {
        py::enum_<GateTypeProperty>(m, "GateTypeProperty", R"(
            A set of available properties for a gate type.
        )")
            .value("combinational", GateTypeProperty::combinational, R"(Combinational gate type.)")
            .value("sequential", GateTypeProperty::sequential, R"(Sequential gate type.)")
            .value("power", GateTypeProperty::power, R"(Power gate type.)")
            .value("ground", GateTypeProperty::ground, R"(Ground gate type.)")
            .value("lut", GateTypeProperty::lut, R"(LUT gate type.)")
            .value("ff", GateTypeProperty::ff, R"(Flip-flop gate type.)")
            .value("latch", GateTypeProperty::latch, R"(Latch gate type.)")
            .value("ram", GateTypeProperty::ram, R"(RAM gate type.)")
            .value("io", GateTypeProperty::io, R"(IO gate type.)")
            .value("dsp", GateTypeProperty::dsp, R"(DSP gate type.)")
            .value("mux", GateTypeProperty::mux, R"(MUX gate type.)")
            .value("buffer", GateTypeProperty::buffer, R"(Buffer gate type.)")
            .value("carry", GateTypeProperty::carry, R"(Carry gate type.)")
            .export_values();

        py::enum_<PinDirection>(m, "PinDirection", R"(
            Defines the direction of a pin.
        )")
            .value("none", PinDirection::none, R"(Invalid pin.)")
            .value("input", PinDirection::input, R"(Input pin.)")
            .value("output", PinDirection::output, R"(Output pin.)")
            .value("inout", PinDirection::inout, R"(Inout pin.)")
            .value("internal", PinDirection::internal, R"(Internal pin.)")
            .export_values();

        py::enum_<PinType>(m, "PinType", R"(Defines the type of a pin.)")
            .value("none", PinType::none, R"(Default pin.)")
            .value("power", PinType::power, R"(Power pin.)")
            .value("ground", PinType::ground, R"(Ground pin.)")
            .value("lut", PinType::lut, R"(Pin that generates output from LUT initialization string.)")
            .value("state", PinType::state, R"(Pin that generates output from internal state.)")
            .value("neg_state", PinType::neg_state, R"(Pin that generates output from negated internal state.)")
            .value("clock", PinType::clock, R"(Clock pin.)")
            .value("enable", PinType::enable, R"(Enable pin.)")
            .value("set", PinType::set, R"(Set pin.)")
            .value("reset", PinType::reset, R"(Reset pin.)")
            .value("data", PinType::data, R"(Data pin.)")
            .value("address", PinType::address, R"(Address pin.)")
            .value("io_pad", PinType::io_pad, R"(IO pad pin.)")
            .value("select", PinType::select, R"(Select pin.)")
            .export_values();

        py::class_<GateType, RawPtrWrapper<GateType>> py_gate_type(m, "GateType", R"(
            A gate type contains information about its internals such as input and output pins as well as its Boolean functions.
        )");

        py::enum_<GateType::ClearPresetBehavior>(py_gate_type, "ClearPresetBehavior", R"(
            Defines the behavior of the gate type in case both clear and preset are active at the same time.
        )")
            .value("L", GateType::ClearPresetBehavior::L, R"(Set the internal state to '0'.)")
            .value("H", GateType::ClearPresetBehavior::H, R"(Set the internal state to '1'.)")
            .value("N", GateType::ClearPresetBehavior::N, R"(Do not change the internal state.)")
            .value("T", GateType::ClearPresetBehavior::T, R"(Toggle, i.e., invert the internal state.)")
            .value("X", GateType::ClearPresetBehavior::X, R"(Set the internal state to 'X'.)")
            .value("undef", GateType::ClearPresetBehavior::undef, R"(Invalid bahavior, used by default.)")
            .export_values();

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

        py_gate_type.def_property_readonly("properties", &GateType::get_properties, R"(
            The properties assigned to the gate type.

            :type: set[hal_py.GateTypeProperty]
        )");

        py_gate_type.def("get_properties", &GateType::get_properties, R"(
            Get the properties assigned to the gate type.

            :returns: The properties of the gate type.
            :rtype: set[hal_py.GateTypeProperty]
        )");

        py_gate_type.def("has_property", &GateType::has_property, R"(
            Check whether the gate type has the specified property.

            :param hal_py.GateTypeProperty type: The property to check for.
            :returns: True if the gate type has the specified property, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def_property_readonly("gate_library", &GateType::get_gate_library, R"(
            The gate library this gate type is associated with.

            :type: hal_py.GateLibrary
        )");

        py_gate_type.def("get_gate_library", &GateType::get_gate_library, R"(
            Get the gate library this gate type is associated with.

            :returns: The gate library.
            :rtype: hal_py.GateLibrary
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

        py_gate_type.def("add_pin", &GateType::add_pin, py::arg("pin"), py::arg("direction"), py::arg("pin_type"), R"(
            Add a pin of the specified direction and type to the gate type.
        
            :param str pin: The pin.
            :param hal_py.PinDirection direction: The pin direction to be assigned.
            :param hal_py.PinType pin_type: The pin type to be assigned.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def("add_pins", &GateType::add_pin, py::arg("pins"), py::arg("direction"), py::arg("pin_type"), R"(
            Add a list of pin of the specified direction and type to the gate type.
        
            :param list[str] pins: The pins.
            :param hal_py.PinDirection direction: The pin direction to be assigned.
            :param hal_py.PinType pin_type: The pin type to be assigned.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def("get_pins", &GateType::get_pins, R"(
            Get all pins belonging to the gate type.
        
            :returns: A list of pins.
            :rtype: list[str]
        )");

        py_gate_type.def_property_readonly("pins", &GateType::get_pins, R"(
            A list of all pins belonging to the gate type.

            :type: list[str]
        )");

        py_gate_type.def("get_pin_direction", &GateType::get_pin_direction, py::arg("pin"), R"(
            Get the pin direction of the given pin. The user has to make sure that the pin exists before calling this function. If the pin does not exist, the direction 'internal' will be returned.
        
            :param str pin: The pin.
            :returns: The pin direction.
            :rtype: hal_py.PinDirection
        )");

        py_gate_type.def("get_pin_directions", &GateType::get_pin_directions, R"(
            Get the pin directions of all pins as a dict.
         
            :returns: A dict from pin to pin direction.
            :rtype: dict[std,hal_py.PinDirection]
        )");

        py_gate_type.def_property_readonly("pin_directions", &GateType::get_pin_directions, R"(
            The pin directions of all pins as a dict.
         
            :type: dict[str,hal_py.PinDirection]
        )");

        py_gate_type.def("get_pins_of_direction", &GateType::get_pins_of_direction, py::arg("direction"), R"(
            Get all pins of the specified pin direction.
        
            :param hal_py.PinDirection direction: The pin direction.
            :returns: A set of pins.
            :rtype: set[str]
        )");

        py_gate_type.def("assign_pin_type", &GateType::assign_pin_type, py::arg("pin"), py::arg("pin_type"), R"(
            Assign a pin type to the given pin. The pin must have been added to the gate type beforehand.
    
            :param str pin: The pin.
            :param hal_py.PinType pin_type: The pin type to be assigned.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def("get_pin_type", &GateType::get_pin_type, py::arg("pin"), R"(
            Get the pin type of the given pin. The user has to make sure that the pin exists before calling this function. If the pin does not exist, the type 'none' will be returned.
        
            :param str pin: The pin.
            :returns: The pin type.
            :rtype: hal_py.PinType
        )");

        py_gate_type.def("get_pin_types", &GateType::get_pin_types, R"(
            Get the pin types of all pins as a dict.
         
            :returns: A dict from pin to pin type.
            :rtype: dict[str,hal_py.PinType]
        )");

        py_gate_type.def_property_readonly("pin_types", &GateType::get_pin_types, R"(
            The pin types of all pins as a dict.
         
            :type: dict[str,hal_py.PinType]
        )");

        py_gate_type.def("get_pins_of_type", &GateType::get_pins_of_type, py::arg("pin_type"), R"(
            Get all pins of the specified pin type.
        
            :param hal_py.PinType pin_type: The pin type.
            :returns: A set of pins.
            :rtype: set[str]
        )");

        py_gate_type.def("assign_pin_group", &GateType::assign_pin_group, py::arg("group"), py::arg("pins"), R"(
            Assign existing pins to a pin group.

            :param str group: The name of the pin group.
            :param list[tuple(int,str)] pins: The pins to be added to the group including their indices.
        )");

        py_gate_type.def("get_pin_group", &GateType::get_pin_group, py::arg("pin"), R"(
            Get the pin type of the given pin. The user has to make sure that the pin exists before calling this function. If the pin is not in a group or the does not exist, an empty string will be returned.
        
            :param str pin: The pin.
            :returns: The pin group.
            :rtype: str
        )");

        py_gate_type.def_property_readonly("pin_groups", &GateType::get_pin_groups, R"(
            All pin groups of the gate type as a dict from pin group names to the pins of each group including their indices.

            :type: dict[str,list[tuple(int,str)]]
        )");

        py_gate_type.def("get_pin_groups", &GateType::get_pin_groups, R"(
            Get all pin groups of the gate type.

            :returns: A dict from pin group names to the pins of each group including their indices.
            :rtype: dict[str,list[tuple(int,str)]]
        )");

        py_gate_type.def("get_pins_of_group", &GateType::get_pins_of_group, py::arg("group"), R"(
            Get all pins of the specified pin group including their indices.
         
            :param str group: The name of the pin group.
            :returns: The pins including their indices.
            :rtype: list[tuple(int,str)]
        )");

        py_gate_type.def("get_pin_of_group_at_index", &GateType::get_pin_of_group_at_index, py::arg("group"), py::arg("index"), R"(
            Get the pin at the specified index of the given group.

            :param str group: The name of the pin group.
            :param int index: The index of the pin.
            :returns: The pin.
            :rtype: str
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
