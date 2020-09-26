#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_sequential_init(py::module& m)
    {
        py::class_<GateTypeSequential, GateType, RawPtrWrapper<GateTypeSequential>> py_gate_type_sequential(
            m, "GateTypeSequential", R"(Sequential gate type class containing information about the internals of a specific sequential gate type.)");

        py::enum_<GateTypeSequential::SetResetBehavior>(py_gate_type_sequential, "SetResetBehavior", R"(
        Represents the behavior that a sequential cell shows when both set and reset are active. Available are: U (not specified for gate type), L (set to ZERO), H (set to ONE), N (no change), T (toggle), and X (undefined).)")
            .value("U", GateTypeSequential::SetResetBehavior::U)
            .value("L", GateTypeSequential::SetResetBehavior::L)
            .value("H", GateTypeSequential::SetResetBehavior::H)
            .value("N", GateTypeSequential::SetResetBehavior::N)
            .value("T", GateTypeSequential::SetResetBehavior::T)
            .value("X", GateTypeSequential::SetResetBehavior::X)
            .export_values();

        py_gate_type_sequential.def(py::init<const std::string&, GateType::BaseType>(), py::arg("name"), py::arg("bt"), R"(
        Construct a new sequential gate type.

        :param str name: The name of the sequential gate type.
        :param hal_py.GateType.BaseType bt: The base type of the sequential gate type.
)");

        py_gate_type_sequential.def("add_state_output_pin", &GateTypeSequential::add_state_output_pin, py::arg("pin_name"), R"(
        Add an output pin to the collection of output pins that generate their output from the next_state function.
        The pin has to be declared as an output pin beforehand.

        :param str pin_name: Name of the output pin.
)");

        py_gate_type_sequential.def("get_state_output_pins", &GateTypeSequential::get_state_output_pins, R"(
        Get the output pins that use the next_state function to generate their output.

        :returns: The set of output pin names.
        :rtype: set[str]
)");

        py_gate_type_sequential.def("add_inverted_state_output_pin", &GateTypeSequential::add_inverted_state_output_pin, py::arg("pin_name"), R"(
        Add an output pin to the collection of output pins that generate their output from the inverted next_state function.
        The pin has to be declared as an output pin beforehand.

        :param str pin_name: Name of the output pin.
)");

        py_gate_type_sequential.def("get_inverted_state_output_pins", &GateTypeSequential::get_inverted_state_output_pins, R"(
        Get the output pins that use the inverted next_state function to generate their output.

        :returns: The set of output pin names.
        :rtype: set[str]
)");

        py_gate_type_sequential.def("add_clock_pin", &GateTypeSequential::add_clock_pin, py::arg("pin_name"), R"(
        Add an input pin to the collection of input pins that are connected to a clock pin.
        The pin has to be declared as an input pin beforehand.

        :param str pin_name: Name of the output pin.
)");

        py_gate_type_sequential.def("get_clock_pins", &GateTypeSequential::get_clock_pins, R"(
        Get the input pins that that are connected to a clock signal.

        :returns: The set of input pin names.
        :rtype: set[str]
)");
        py_gate_type_sequential.def("set_set_reset_behavior", &GateTypeSequential::set_set_reset_behavior, py::arg("sb1"), py::arg("sb2"), R"(
        Set the behavior that describes the internal state when both set and reset are active.

        :param hal_py.SetResetBehavior sb1: The value specifying the behavior for the internal state.
        :param hal_py.SetResetBehavior sb2: The value specifying the behavior for the inverted internal state.
)");

        py_gate_type_sequential.def("get_set_reset_behavior", &GateTypeSequential::get_set_reset_behavior, R"(
        Get the behavior that describes the internal state when both set and reset are active.

        :returns: A tuple of values specifying the behavior of the internal state and the inverted internal state.
        :rytpe: tuple(hal_py.SetResetBehavior, hal_py.SetResetBehavior)
)");

        py_gate_type_sequential.def_property("init_data_category", &GateTypeSequential::get_init_data_category, &GateTypeSequential::set_init_data_category, R"(
        The category in which to find the INIT string.

        :type: str
)");

        py_gate_type_sequential.def("set_init_data_category", &GateTypeSequential::set_init_data_category, py::arg("category"), R"(
        Set the category in which to find the INIT string.

        :param str category: The category as a string.
)");

        py_gate_type_sequential.def("get_init_data_category", &GateTypeSequential::get_init_data_category, R"(
        Get the category in which to find the INIT string.

        :returns: The category as a string.
        :rtype: str
)");

        py_gate_type_sequential.def_property("init_data_identifier", &GateTypeSequential::get_init_data_identifier, &GateTypeSequential::set_init_data_identifier, R"(
        The identifier used to specify the INIT string.

        :type: str
)");

        py_gate_type_sequential.def("set_init_data_identifier", &GateTypeSequential::set_init_data_identifier, py::arg("identifier"), R"(
        Set the identifier used to specify the INIT string.

        :param str identifier: The identifier as a string.
)");

        py_gate_type_sequential.def("get_init_data_identifier", &GateTypeSequential::get_init_data_identifier, R"(
        Get the identifier used to specify the INIT string.

        :returns: The identifier as a string.
        :rtype: str
)");
    }
}    // namespace hal
