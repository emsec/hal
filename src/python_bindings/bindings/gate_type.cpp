#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_init(py::module& m)
    {
        py::class_<GateType, RawPtrWrapper<GateType>> py_gate_type(m, "GateType", R"(
        Gate type class containing information about the internals of a specific gate type.
)");

        py::enum_<GateType::BaseType>(py_gate_type, "BaseType", R"(
            Represents the base type of a gate type.
        )")

            .value("combinatorial", GateType::BaseType::combinatorial, R"(Represents a combinatorial gate type.)")
            .value("lut", GateType::BaseType::lut, R"(Represents a combinatorial LUT gate type.)")
            .value("ff", GateType::BaseType::ff, R"(Represents a sequential FF gate type.)")
            .value("latch", GateType::BaseType::latch, R"(Represents a sequential latch gate type.)")
            .export_values();

        py_gate_type.def(py::init<const std::string&>(), py::arg("name"), R"(
            Construct a new gate type by specifying its name.

            :param str name: The name of the gate type.
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

        py_gate_type.def("add_input_pin", &GateType::add_input_pin, py::arg("input_pin"), R"(
            Add an input pin to the gate type.

            :param str input_pin: The name of the input pin to add.
        )");

        py_gate_type.def("add_input_pins", &GateType::add_input_pins, py::arg("input_pins"), R"(
            Add a list of input pins to the gate type.

            :param list[str] input_pins: The list of names of input pins to add.
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

        py_gate_type.def("add_output_pin", &GateType::add_output_pin, py::arg("output_pin"), R"(
            Add an output pin to the gate type.

            :param str output_pin: The name of the output pin to add.
        )");

        py_gate_type.def("add_output_pins", &GateType::add_output_pins, py::arg("output_pins"), R"(
            Add a list of output pins to the gate type.

            :param list[str] output_pins: The list of names of output pins to add.
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

        py_gate_type.def("assign_power_pin", &GateType::assign_power_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of power pins.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type.def_property_readonly("power_pins", &GateType::get_power_pins, R"(
            The set of input pins that that are classified as power pins.

            :type: set[str]
        )");

        py_gate_type.def("get_power_pins", &GateType::get_power_pins, R"(
            Get all input pins classfied as power pins.

            :returns: The set of input pin names.
            :rtype: set[str]
        )");

        py_gate_type.def("assign_ground_pin", &GateType::assign_ground_pin, py::arg("pin_name"), R"(
            Add an existing input pin to the collection of ground pins.
            The pin has to be declared as an input pin beforehand.

            :param str pin_name: The name of the input pin to add.
        )");

        py_gate_type.def_property_readonly("ground_pins", &GateType::get_ground_pins, R"(
            The set of input pins that that are classified as ground pins.

            :type: set[str]
        )");

        py_gate_type.def("get_ground_pins", &GateType::get_ground_pins, R"(
            Get all input pins classfied as ground pins.

            :returns: The set of input pin names.
            :rtype: set[str]
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
    }
}    // namespace hal
