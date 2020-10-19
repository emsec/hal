#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_lut_init(py::module& m)
    {
        py::class_<GateTypeLut, GateType, RawPtrWrapper<GateTypeLut>> py_gate_type_lut(m, "GateTypeLut", R"(
            A LUT gate type contains information about its internals such as input and output pins as well as its Boolean functions.
            In addition to the standard gate type functionality, it provides the foundation to automatically read the initialization string of each LUT gate of the netlist and convert it to a Boolean function.
        )");

        py_gate_type_lut.def(py::init<const std::string&>(), py::arg("name"), R"(
            Construct a new LUT gate type by specifying its name.

            :param str name: The name of the LUT gate type.
        )");

        py_gate_type_lut.def("add_output_from_init_string_pin", &GateTypeLut::add_output_from_init_string_pin, py::arg("output_pin"), R"(
            Add an existing output pin to the collection of output pins that generate their output not from a Boolean function but an initialization string.
            The pin has to be declared as an output pin beforehand.

            :param str output_pin: The name of the output pin to add.
        )");

        py_gate_type_lut.def_property_readonly("output_from_init_string_pins", &GateTypeLut::get_output_from_init_string_pins, R"(
            The set of output pins that generate their output not from a Boolean function but an initialization string.

            :type: set[str]
        )");

        py_gate_type_lut.def("get_output_from_init_string_pins", &GateTypeLut::get_output_from_init_string_pins, R"(
            Get the set of output pins that generate their output not from a Boolean function but an initialization string.

            :returns: The set of output pin names.
            :rtype: set[str]
        )");

        py_gate_type_lut.def_property("config_data_category", &GateTypeLut::get_config_data_category, &GateTypeLut::set_config_data_category, R"(
            The data category in which to find the initialization string.

            :type: str
        )");

        py_gate_type_lut.def("set_config_data_category", &GateTypeLut::set_config_data_category, py::arg("category"), R"(
            Set the data category in which to find the initialization string.

            :param str category:  The data category.
        )");

        py_gate_type_lut.def("get_config_data_category", &GateTypeLut::get_config_data_category, R"(
            Get the data category in which to find the initialization string.

            :returns: The data category.
            :rtype: str
        )");

        py_gate_type_lut.def_property("config_data_identifier", &GateTypeLut::get_config_data_identifier, &GateTypeLut::set_config_data_identifier, R"(
            The data identifier used to specify the initialization string.

            :type: str
        )");

        py_gate_type_lut.def("set_config_data_identifier", &GateTypeLut::set_config_data_identifier, py::arg("identifier"), R"(
            Set the data identifier used to specify the initialization string.

            :param str identifier: The data identifier.
        )");

        py_gate_type_lut.def("get_config_data_identifier", &GateTypeLut::get_config_data_identifier, R"(
            Get the data identifier used to specify the initialization string.

            :returns: The data identifier.
            :rtype: str
        )");

        py_gate_type_lut.def_property("config_data_ascending_order", &GateTypeLut::is_config_data_ascending_order, &GateTypeLut::set_config_data_ascending_order, R"(
            The bit-order of the initialization string, true if ascending.

            :type: bool
        )");

        py_gate_type_lut.def("set_config_data_ascending_order", &GateTypeLut::set_config_data_ascending_order, py::arg("ascending"), R"(
            Set the bit-order of the initialization string.

            :param bool ascending: True if ascending bit-order, false otherwise.
        )");

        py_gate_type_lut.def("is_config_data_ascending_order", &GateTypeLut::is_config_data_ascending_order, R"(
            Get the bit-order of the initialization string.

            :returns: True if ascending bit-order, false otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
