#include "bindings.h"

namespace hal
{
    void gate_type_init(py::module& m)
    {
        py::class_<GateType, std::shared_ptr<GateType>> py_gate_type(m, "GateType", R"(
        Gate type class containing information about the internals of a specific gate type.
)");

        py::enum_<GateType::BaseType>(py_gate_type, "BaseType", R"(
        Represents the base type of a gate type. Available are: combinatorial, lut, ff, and latch.
        )")

            .value("combinatorial", GateType::BaseType::combinatorial)
            .value("lut", GateType::BaseType::lut)
            .value("ff", GateType::BaseType::ff)
            .value("latch", GateType::BaseType::latch)
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

        py_gate_type.def("__str__", [](const GateType& gt) { return gt.to_string(); });

        py_gate_type.def_property_readonly("name", &GateType::get_name, R"(
        The name of the gate type.

        :type: str
)");

        py_gate_type.def("get_name", &GateType::get_name, R"(
        Get the name of the gate type.

        :returns: The name of the gate type.
        :rtype: str
)");

        py_gate_type.def("get_base_type", &GateType::get_base_type, R"(
        Get the base type of the gate type. The base type can be either combinatorial, lut, ff, or latch.

        :returns: The base type of the gate type.
        :rtype: hal_py.GateType.BaseType
)");

        py_gate_type.def("add_input_pin", &GateType::add_input_pin, py::arg("input_pin"), R"(
        Add an input pin to the gate type.

        :param str input_pin: The name of an input pin.
)");

        py_gate_type.def("add_input_pins", &GateType::add_input_pins, py::arg("input_pins"), R"(
        Add a list of input pins to the gate type.

        :param list[str] input_pins: The list of input pins.
)");

        py_gate_type.def_property_readonly("input_pins", &GateType::get_input_pins, R"(
        A list of input pins of the gate type.

        :type: list[str]
)");

        py_gate_type.def("get_input_pins", &GateType::get_input_pins, R"(
        Get a list of input pins of the gate type.

        :returns: A list of input pins of the gate type.
        :rtype: list[str]
)");

        py_gate_type.def("add_output_pin", &GateType::add_output_pin, py::arg("output_pin"), R"(
        Add an output pin to the gate type.

        :param str output_pin: The name of an output pin.
)");

        py_gate_type.def("add_output_pins", &GateType::add_output_pins, py::arg("output_pins"), R"(
        Add a list of output pins to the gate type.

        :param list[str] output_pins: The list of output pins.
)");

        py_gate_type.def_property_readonly("output_pins", &GateType::get_output_pins, R"(
        A list of output pins of the gate type.

        :type: list[str]
)");

        py_gate_type.def("get_output_pins", &GateType::get_output_pins, R"(
        Get a list of output pins of the gate type.

        :returns: A vector of output pins of the gate type.
        :rtype: list[str]
)");

        py_gate_type.def("assign_input_pin_group", &GateType::assign_input_pin_group, py::arg("group_name"), py::arg("index_to_pin"), R"(
        Assign existing input pins to a input pin group.

        :param str group_name: The name of the input pin group.
        :param dict[int,str] index_to_pin: Map from index to output pin name.
)");

        py_gate_type.def("assign_input_pin_groups", &GateType::assign_input_pin_groups, py::arg("pin_groups"), R"(
        Assign existing input pins to multiple input pin groups.

        :param dict[str,dict[int,str]] pin_groups: A map from group names to a map from indices to pin names.
)");

        py_gate_type.def("get_input_pin_groups", &GateType::get_input_pin_groups, R"(
        Get all input pin groups of the gate type.

        :returns: A map from group names to a map from indices to pin names.
        :rtype: dict[str,dict[int,str]]
)");

        py_gate_type.def("assign_output_pin_group", &GateType::assign_output_pin_group, py::arg("group_name"), py::arg("index_to_pin"), R"(
        Assign existing output pins to a output pin group.

        :param str group_name: The name of the output pin group.
        :param dict[int,str] index_to_pin: Map from index to output pin name.
)");

        py_gate_type.def("assign_output_pin_groups", &GateType::assign_output_pin_groups, py::arg("pin_groups"), R"(
        Assign existing output pins to multiple output pin groups.

        :param dict[str,dict[int,str]] pin_groups: A map from group names to a map from indices to pin names.
)");

        py_gate_type.def("get_output_pin_groups", &GateType::get_output_pin_groups, R"(
        Get all output pin groups of the gate type.

        :returns: A map from group names to a map from indices to pin names.
        :rtype: dict[str,dict[int,str]]
)");

        py_gate_type.def("add_boolean_function", &GateType::add_boolean_function, py::arg("pin_name"), py::arg("bf"), R"(
        Add a boolean function with the specified name to the gate type.

        :param str name: The name of the boolean function.
        :param hal_py.BooleanFunction bf: A boolean function object.
)");

        py_gate_type.def("add_boolean_functions", &GateType::add_boolean_functions, py::arg("functions"), R"(
        Add multiple boolean functions to the gate type.

        :param dict[str,hal_py.BooleanFunction] functions: Map from function name to boolean function.
)");

        py_gate_type.def_property_readonly("boolean_functions", &GateType::get_boolean_functions, R"(
        A map from function names to the boolean functions of the gate type.

        :type: dict[str,hal_py.BooleanFunction]
)");

        py_gate_type.def("get_boolean_functions", &GateType::get_boolean_functions, R"(
        Get a map containing the boolean functions of the gate type.

        :returns: A map from function names to boolean functions.
        :rtype: dict[str,hal_py.BooleanFunction]
)");
    }
}    // namespace hal
