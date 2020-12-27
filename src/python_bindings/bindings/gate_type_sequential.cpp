#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_sequential_init(py::module& m)
    {
        py::class_<GateTypeSequential, GateType, RawPtrWrapper<GateTypeSequential>> py_gate_type_sequential(m, "GateTypeSequential", R"(
            A sequential gate type contains information about its internals such as input and output pins as well as its Boolean functions.
            In addition to the standard gate type functionality, it provides mechanisms to specify outputs that depend on the internal state of the sequential gate as well as clock pins.
        )");

        py::enum_<GateTypeSequential::ClearPresetBehavior>(py_gate_type_sequential, "ClearPresetBehavior", R"(
            Defines the behavior of the gate type in case both clear and preset are active at the same time.
        )")
            .value("U", GateTypeSequential::ClearPresetBehavior::U, R"(Default value when no behavior is specified.)")
            .value("L", GateTypeSequential::ClearPresetBehavior::L, R"(Set the internal state to '0'.)")
            .value("H", GateTypeSequential::ClearPresetBehavior::H, R"(Set the internal state to '1'.)")
            .value("N", GateTypeSequential::ClearPresetBehavior::N, R"(Do not change the internal state.)")
            .value("T", GateTypeSequential::ClearPresetBehavior::T, R"(Toggle, i.e., invert the internal state.)")
            .value("X", GateTypeSequential::ClearPresetBehavior::X, R"(Set the internal state to 'X'.)")
            .export_values();

        py_gate_type_sequential.def(py::init<const std::string&, GateType::BaseType>(), py::arg("name"), py::arg("base_type"), R"(
            Construct a new LUT gate type by specifying its name and base type.
            The base type must be either 'ff' or 'latch'.

            :param str name: The name of the sequential gate type.
            :param hal_py.GateType.BaseType base_type: The base type of the sequential gate type.
        )");

        py_gate_type_sequential.def("assign_state_output_pin", &GateTypeSequential::assign_state_pin, py::arg("pin_name"), R"(
            Add an existing output pin to the collection of output pins that generate their output from the internal state of the gate.
            The pin has to be declared as an output pin beforehand.

            :param str pin_name: The name of the output pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("state_pins", &GateTypeSequential::get_state_pins, R"(
            The set of output pins that use the internal state of the gate to generate their output.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_state_pins", &GateTypeSequential::get_state_pins, R"(
            Get the output pins that use the internal state of the gate to generate their output.

            :returns: The set of output pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("assign_negated_state_pin", &GateTypeSequential::assign_negated_state_pin, py::arg("pin_name"), R"(
            Add an existing output pin to the collection of output pins that generate their output from the negated internal state of the gate.
            The pin has to be declared as an output pin beforehand.

            :param str pin_name: The name of the output pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("negated_state_pins", &GateTypeSequential::get_negated_state_pins, R"(
            The set of output pins that use the negated internal state of the gate to generate their output.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_negated_state_pins", &GateTypeSequential::get_negated_state_pins, R"(
            Get the output pins that use the negated internal state of the gate to generate their output.

            :returns: The set of output pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("assign_clock_pin", &GateTypeSequential::assign_clock_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of clock pins.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("clock_pins", &GateTypeSequential::get_clock_pins, R"(
            The set of input pins that that are classified as clock pins.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_clock_pins", &GateTypeSequential::get_clock_pins, R"(
            Get all input pins classfied as clock pins.

            :returns: The set of input pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("assign_enable_pin", &GateTypeSequential::assign_enable_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of enable pins.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("enable_pins", &GateTypeSequential::get_enable_pins, R"(
            The set of input pins that that are classified as enable pins.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_enable_pins", &GateTypeSequential::get_enable_pins, R"(
            Get all input pins classfied as enable pins.

            :returns: The set of input pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("assign_reset_pin", &GateTypeSequential::assign_reset_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of reset pins.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("reset_pins", &GateTypeSequential::get_reset_pins, R"(
            The set of input pins that that are classified as reset pins.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_reset_pins", &GateTypeSequential::get_reset_pins, R"(
            Get all input pins classfied as reset pins.

            :returns: The set of input pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("assign_set_pin", &GateTypeSequential::assign_set_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of set pins.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("set_pins", &GateTypeSequential::get_set_pins, R"(
            The set of input pins that that are classified as set pins.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_set_pins", &GateTypeSequential::get_set_pins, R"(
            Get all input pins classfied as set pins.

            :returns: The set of input pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("assign_data_pin", &GateTypeSequential::assign_data_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of data pins.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("data_pins", &GateTypeSequential::get_data_pins, R"(
            The set of input pins that that are classified as data pins.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_data_pins", &GateTypeSequential::get_data_pins, R"(
            Get all input pins classfied as data pins.

            :returns: The set of input pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("set_clear_preset_behavior", &GateTypeSequential::set_clear_preset_behavior, py::arg("cp1"), py::arg("cp2"), R"(
            Set the behavior that describes the internal state when both clear and preset are active at the same time.

            :param hal_py.ClearPresetBehavior cp1: The value specifying the behavior for the internal state.
            :param hal_py.ClearPresetBehavior cp2: The value specifying the behavior for the negated internal state.
        )");

        py_gate_type_sequential.def("get_clear_preset_behavior", &GateTypeSequential::get_clear_preset_behavior, R"(
            Get the behavior of the internal state and the negated internal state when both clear and preset are active at the same time.

            :returns: The values specifying the behavior for the internal and negated internal state.
            :rytpe: tuple(hal_py.ClearPresetBehavior, hal_py.ClearPresetBehavior)
        )");

        py_gate_type_sequential.def_property("init_data_category", &GateTypeSequential::get_init_data_category, &GateTypeSequential::set_init_data_category, R"(
            The data category in which to find the initialization value.

            :type: str
        )");

        py_gate_type_sequential.def("set_init_data_category", &GateTypeSequential::set_init_data_category, py::arg("category"), R"(
            Set the data category in which to find the initialization value.

            :param str category: The data category.
        )");

        py_gate_type_sequential.def("get_init_data_category", &GateTypeSequential::get_init_data_category, R"(
            Get the data category in which to find the initialization value.

            :returns: The data category.
            :rtype: str
        )");

        py_gate_type_sequential.def_property("init_data_identifier", &GateTypeSequential::get_init_data_identifier, &GateTypeSequential::set_init_data_identifier, R"(
            The data identifier used to specify the initialization value.

            :type: str
        )");

        py_gate_type_sequential.def("set_init_data_identifier", &GateTypeSequential::set_init_data_identifier, py::arg("identifier"), R"(
            Set the data identifier used to specify the initialization value.

            :param str identifier: The data identifier.
        )");

        py_gate_type_sequential.def("get_init_data_identifier", &GateTypeSequential::get_init_data_identifier, R"(
            Get the data identifier used to specify the initialization value.

            :returns: The data identifier.
            :rtype: str
        )");
    }
}    // namespace hal
