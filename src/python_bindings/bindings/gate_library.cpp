#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_library_init(py::module& m)
    {
        py::class_<GateLibrary> py_gate_library(m, "GateLibrary", R"(Gate library class containing information about the gates contained in the library.)");

        py_gate_library.def_property_readonly("name", &GateLibrary::get_name, R"(
        The name of the library.

        :type: str
)");

        py_gate_library.def("get_name", &GateLibrary::get_name, R"(
        Get the name of the library.

        :returns: The name.
        :rtype: str
)");

        py_gate_library.def_property_readonly("path", &GateLibrary::get_name, R"(
        The path of the library.

        :type: hal_py.hal_path
)");

        py_gate_library.def("get_path", &GateLibrary::get_name, R"(
        Get the file path of the library.

        :returns: The path.
        :rtype: hal_py.hal_path
)");

        py_gate_library.def_property_readonly("gate_types", &GateLibrary::get_gate_types, R"(
        A dict from gate type names to gate type objects containing all gate types of the gate library.

        :type: dict[str,hal_py.GateType]
)");

        py_gate_library.def("get_gate_types", &GateLibrary::get_gate_types, R"(
        Get dict from gate type names to gate type objects containing all gate types of the gate library.

        :returns: A dict from gate type names to gate type objects.
        :rtype: dict[str,hal_py.GateType]
)");

        py_gate_library.def_property_readonly("vcc_gate_types", &GateLibrary::get_vcc_gate_types, R"(
        A dict from gate type names to gate type objects containing all VCC gate types of the gate library.

        :type: dict[str,hal_py.GateType]
)");

        py_gate_library.def("get_vcc_gate_types", &GateLibrary::get_vcc_gate_types, R"(
        Get dict from gate type names to gate type objects containing all VCC gate types of the gate library.

        :returns: A dict from VCC gate type names to gate type objects.
        :rtype: dict[str,hal_py.GateType]
)");

        py_gate_library.def_property_readonly("gnd_gate_types", &GateLibrary::get_vcc_gate_types, R"(
        A dict from gate type names to gate type objects containing all GND gate types of the gate library.

        :type: dict[str,hal_py.GateType]
)");

        py_gate_library.def("get_gnd_gate_types", &GateLibrary::get_gnd_gate_types, R"(
        Get dict from gate type names to gate type objects containing all GND gate types of the gate library.

        :returns: A dictp from GND gate type names to gate type objects.
        :rtype: dict[str,hal_py.GateType]
)");

        py_gate_library.def("add_include", &GateLibrary::add_include, py::arg("include"), R"(
        Add a necessary includes of the gate library, e.g., VHDL libraries.

        :param str inc: The include to add.
)");

        py_gate_library.def_property_readonly("includes", &GateLibrary::get_includes, R"(
        A list of necessary includes of the gate library, e.g., VHDL libraries.

        :returns: VHDL includes to use by serializer.
        :rtype: list[str]
)");

        py_gate_library.def("get_includes", &GateLibrary::get_includes, R"(
        Get a list of necessary includes of the gate library, e.g., VHDL libraries.

        :returns: A list of includes.
        :rtype: list[str]
)");
    }
}    // namespace hal
