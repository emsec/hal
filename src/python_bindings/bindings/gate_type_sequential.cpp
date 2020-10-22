#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_sequential_init(py::module& m)
    {
        py::class_<GateTypeSequential, GateType, RawPtrWrapper<GateTypeSequential>> py_gate_type_sequential(m, "GateTypeSequential", R"(
            A sequential gate type contains information about its internals such as input and output pins as well as its Boolean functions.
            In addition to the standard gate type functionality, it provides mechanisms to specify outputs that depend on the internal state of the sequential gate as well as clock pins.
        )");

        py::enum_<GateTypeSequential::SetResetBehavior>(py_gate_type_sequential, "SetResetBehavior", R"(
            Defines the behavior of the gate type in case both set and reset are active at the same time.
        )")
            .value("U", GateTypeSequential::SetResetBehavior::U, R"(Default value when no behavior is specified.)")
            .value("L", GateTypeSequential::SetResetBehavior::L, R"(Set the internal state to '0'.)")
            .value("H", GateTypeSequential::SetResetBehavior::H, R"(Set the internal state to '1'.)")
            .value("N", GateTypeSequential::SetResetBehavior::N, R"(Do not change the internal state.)")
            .value("T", GateTypeSequential::SetResetBehavior::T, R"(Toggle, i.e., invert the internal state.)")
            .value("X", GateTypeSequential::SetResetBehavior::X, R"(Set the internal state to 'X'.)")
            .export_values();

        py_gate_type_sequential.def(py::init<const std::string&, GateType::BaseType>(), py::arg("name"), py::arg("base_type"), R"(
            Construct a new LUT gate type by specifying its name and base type.
            The base type must be either 'ff' or 'latch'.

            :param str name: The name of the sequential gate type.
            :param hal_py.GateType.BaseType base_type: The base type of the sequential gate type.
        )");

        py_gate_type_sequential.def("add_state_output_pin", &GateTypeSequential::add_state_output_pin, py::arg("pin_name"), R"(
            Add an existing output pin to the collection of output pins that generate their output from the internal state of the gate.
            The pin has to be declared as an output pin beforehand.

            :param str pin_name: The name of the output pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("state_output_pins", &GateTypeSequential::get_state_output_pins, R"(
            The set of names of the output pins that use the internal state of the gate to generate their output.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_state_output_pins", &GateTypeSequential::get_state_output_pins, R"(
            Get the output pins that use the internal state of the gate to generate their output.

            :returns: The set of output pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("add_inverted_state_output_pin", &GateTypeSequential::add_inverted_state_output_pin, py::arg("pin_name"), R"(
            Add an existing output pin to the collection of output pins that generate their output from the inverted internal state of the gate.
            The pin has to be declared as an output pin beforehand.

            :param str pin_name: The name of the output pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("inverted_state_output_pins", &GateTypeSequential::get_inverted_state_output_pins, R"(
            The set of names of the output pins that use the inverted internal state of the gate to generate their output.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_inverted_state_output_pins", &GateTypeSequential::get_inverted_state_output_pins, R"(
            Get the output pins that use the inverted internal state of the gate to generate their output.

            :returns: The set of output pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("add_clock_pin", &GateTypeSequential::add_clock_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of input pins that are connected to a clock signal.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type_sequential.def_property_readonly("clock_pins", &GateTypeSequential::get_clock_pins, R"(
            The set of names of the input pins that that are connected to a clock signal.

            :type: set[str]
        )");

        py_gate_type_sequential.def("get_clock_pins", &GateTypeSequential::get_clock_pins, R"(
            Get the input pins that that are connected to a clock signal.

            :returns: The set of input pin names.
            :rtype: set[str]
        )");

        py_gate_type_sequential.def("set_set_reset_behavior", &GateTypeSequential::set_set_reset_behavior, py::arg("sb1"), py::arg("sb2"), R"(
            Set the behavior that describes the internal state when both set and reset are active at the same time.

            :param hal_py.SetResetBehavior sb1: The value specifying the behavior for the internal state.
            :param hal_py.SetResetBehavior sb2: The value specifying the behavior for the inverted internal state.
        )");

        py_gate_type_sequential.def("get_set_reset_behavior", &GateTypeSequential::get_set_reset_behavior, R"(
            Get the behavior of the internal state and the inverted internal state when both set and reset are active at the same time.

            :returns: The values specifying the behavior for the internal and inverted internal state.
            :rytpe: tuple(hal_py.SetResetBehavior, hal_py.SetResetBehavior)
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
