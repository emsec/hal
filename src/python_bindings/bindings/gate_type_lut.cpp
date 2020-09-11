#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_lut_init(py::module& m)
    {
        py::class_<GateTypeLut, GateType, RawPtrWrapper<GateTypeLut>> py_gate_type_lut(m, "GateTypeLut", R"(
        LUT gate type class containing information about the internals of a specific LUT gate type.
)");

        py_gate_type_lut.def(py::init<const std::string&>(), py::arg("name"), R"(
        Construct a new LUT gate type.

        :param str name: The name of the LUT gate type.
)");

        py_gate_type_lut.def("add_output_from_init_string_pin", &GateTypeLut::add_output_from_init_string_pin, py::arg("output_pin"), R"(
        Adds an output pin to the collection of output pins that generate their output not from a boolean function but an initialization string.

        :param str output_pin: The name of the output string.
)");

        py_gate_type_lut.def_property_readonly("output_from_init_string_pins", &GateTypeLut::get_output_from_init_string_pins, R"(
        The set of output pins that generate their output not from a boolean function but an initialization string.

        :type: set[str]
)");

        py_gate_type_lut.def("get_output_from_init_string_pins", &GateTypeLut::get_output_from_init_string_pins, R"(
        Get the set of output pins that generate their output not from a boolean function but an initialization string.

        :returns: Set of oputput pin names.
        :rtype: set[str]
)");

        py_gate_type_lut.def_property("config_data_category", &GateTypeLut::get_config_data_category, &GateTypeLut::set_config_data_category, R"(
        The category in which to find the INIT string.

        :type: str
)");

        py_gate_type_lut.def("set_config_data_category", &GateTypeLut::set_config_data_category, py::arg("category"), R"(
        Set the category in which to find the INIT string.

        :param str category: The category as a string.
)");

        py_gate_type_lut.def("get_config_data_category", &GateTypeLut::get_config_data_category, R"(
        Get the category in which to find the INIT string.

        :returns: The category as a string.
        :rtype: str
)");

        py_gate_type_lut.def_property("config_data_identifier", &GateTypeLut::get_config_data_identifier, &GateTypeLut::set_config_data_identifier, R"(
        The identifier used to specify the INIT string.

        :type: str
)");

        py_gate_type_lut.def("set_config_data_identifier", &GateTypeLut::set_config_data_identifier, py::arg("identifier"), R"(
        Set the identifier used to specify the INIT string.

        :param str identifier: The identifier as a string.
)");

        py_gate_type_lut.def("get_config_data_identifier", &GateTypeLut::get_config_data_identifier, R"(
        Get the identifier used to specify the INIT string.

        :returns: The identifier as a string.
        :rtype: str
)");

        py_gate_type_lut.def_property("config_data_ascending_order", &GateTypeLut::is_config_data_ascending_order, &GateTypeLut::set_config_data_ascending_order, R"(
        The bit-order of the INIT string, true if ascending.

        :type: bool
)");

        py_gate_type_lut.def_property("config_data_ascending_order", &GateTypeLut::is_config_data_ascending_order, &GateTypeLut::set_config_data_ascending_order, R"(
        The bit-order of the INIT string, true if ascending.

        :type: bool
)");

        py_gate_type_lut.def("set_config_data_ascending_order", &GateTypeLut::set_config_data_ascending_order, py::arg("ascending"), R"(
        Set the bit-order of the INIT string.

        :param bool ascending: True if ascending bit-order, false otherwise.
)");

        py_gate_type_lut.def("is_config_data_ascending_order", &GateTypeLut::is_config_data_ascending_order, R"(
Get the bit-order of the INIT string.

:returns: True if ascending bit-order, false otherwise.
:rtype: bool
)");
    }
}    // namespace hal
