#include "bindings.h"

void gate_type_sequential_init(py::module& m)
{
    py::class_<gate_type_sequential, gate_type, std::shared_ptr<gate_type_sequential>> py_gate_type_sequential(
        m, "gate_type_sequential", R"(Sequential gate type class containing information about the internals of a specific sequential gate type.)");

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
}