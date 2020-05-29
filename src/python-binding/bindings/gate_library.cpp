#include "bindings.h"

void gate_library_init(py::module& m)
{
    py::class_<gate_library, std::shared_ptr<gate_library>> py_gate_library(m, "gate_library", R"(Gate library class containing information about the gates contained in the library.)");

    py_gate_library.def_property_readonly("name", &gate_library::get_name, R"(
        The name of the library.

        :type: str
)");

    py_gate_library.def("get_name", &gate_library::get_name, R"(
        Get the name of the library.

        :returns: The name.
        :rtype: str
)");

    py_gate_library.def_property_readonly("path", &gate_library::get_name, R"(
        The path of the library.

        :type: hal_py.hal_path
)");

    py_gate_library.def("get_path", &gate_library::get_name, R"(
        Get the file path of the library.

        :returns: The path.
        :rtype: hal_py.hal_path
)");

    py_gate_library.def("add_gate_types", &gate_library::add_gate_type, py::arg("gt"), R"(
        Add a gate type to the gate library.

        :param hal_py.gate_type gt: The gate type object.
)");

    py_gate_library.def_property_readonly("gate_types", &gate_library::get_gate_types, R"(
        A dict from gate type names to gate type objects containing all gate types of the gate library.

        :type: dict[str,hal_py.gate_type]
)");

    py_gate_library.def("get_gate_types", &gate_library::get_gate_types, R"(
        Get dict from gate type names to gate type objects containing all gate types of the gate library.

        :returns: A dict from gate type names to gate type objects.
        :rtype: dict[str,hal_py.gate_type]
)");

    py_gate_library.def_property_readonly("vcc_gate_types", &gate_library::get_vcc_gate_types, R"(
        A dict from gate type names to gate type objects containing all VCC gate types of the gate library.

        :type: dict[str,hal_py.gate_type]
)");

    py_gate_library.def("get_vcc_gate_types", &gate_library::get_vcc_gate_types, R"(
        Get dict from gate type names to gate type objects containing all VCC gate types of the gate library.

        :returns: A dict from VCC gate type names to gate type objects.
        :rtype: dict[str,hal_py.gate_type]
)");

    py_gate_library.def_property_readonly("gnd_gate_types", &gate_library::get_vcc_gate_types, R"(
        A dict from gate type names to gate type objects containing all GND gate types of the gate library.

        :type: dict[str,hal_py.gate_type]
)");

    py_gate_library.def("get_gnd_gate_types", &gate_library::get_gnd_gate_types, R"(
        Get dict from gate type names to gate type objects containing all GND gate types of the gate library.

        :returns: A dictp from GND gate type names to gate type objects.
        :rtype: dict[str,hal_py.gate_type]
)");

    py_gate_library.def("add_include", &gate_library::add_include, py::arg("include"), R"(
        Add a necessary includes of the gate library, e.g., VHDL libraries.

        :param str inc: The include to add.
)");

    py_gate_library.def_property_readonly("includes", &gate_library::get_includes, R"(
        A list of necessary includes of the gate library, e.g., VHDL libraries.

        :returns: VHDL includes to use by serializer.
        :rtype: list[str]
)");

    py_gate_library.def("get_includes", &gate_library::get_includes, R"(
        Get a list of necessary includes of the gate library, e.g., VHDL libraries.

        :returns: A list of includes.
        :rtype: list[str]
)");
}