#include "bindings.h"

void gate_type_init(py::module& m) {
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

    py_gate_type.def("assign_input_pin_group", &gate_type::assign_input_pin_group, py::arg("group_name"), py::arg("index_to_pin"), R"(
        Assign existing input pins to a input pin group.

        :param str group_name: The name of the input pin group.
        :param dict[int,str] index_to_pin: Map from index to output pin name.
)");

    py_gate_type.def("assign_input_pin_groups", &gate_type::assign_input_pin_groups, py::arg("pin_groups"), R"(
        Assign existing input pins to multiple input pin groups.

        :param dict[str,dict[int,str]] pin_groups: A map from group names to a map from indices to pin names. 
)");

    py_gate_type.def("get_input_pin_groups", &gate_type::get_input_pin_groups, R"(
        Get all input pin groups of the gate type.

        :returns: A map from group names to a map from indices to pin names. 
        :rtype: dict[str,dict[int,str]]
)");

    py_gate_type.def("assign_output_pin_group", &gate_type::assign_output_pin_group, py::arg("group_name"), py::arg("index_to_pin"), R"(
        Assign existing output pins to a output pin group.

        :param str group_name: The name of the output pin group.
        :param dict[int,str] index_to_pin: Map from index to output pin name.
)");

    py_gate_type.def("assign_output_pin_groups", &gate_type::assign_output_pin_groups, py::arg("pin_groups"), R"(
        Assign existing output pins to multiple output pin groups.

        :param dict[str,dict[int,str]] pin_groups: A map from group names to a map from indices to pin names. 
)");

    py_gate_type.def("get_output_pin_groups", &gate_type::get_output_pin_groups, R"(
        Get all output pin groups of the gate type.

        :returns: A map from group names to a map from indices to pin names. 
        :rtype: dict[str,dict[int,str]]
)");

    py_gate_type.def("add_boolean_function", &gate_type::add_boolean_function, py::arg("pin_name"), py::arg("bf"), R"(
        Add a boolean function with the specified name to the gate type.

        :param str name: The name of the boolean function.
        :param hal_py.boolean_function bf: A boolean function object.
)");

    py_gate_type.def("add_boolean_functions", &gate_type::add_boolean_functions, py::arg("functions"), R"(
        Add multiple boolean functions to the gate type.

        :param dict[str,hal_py.boolean_function] functions: Map from function name to boolean function.
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
}