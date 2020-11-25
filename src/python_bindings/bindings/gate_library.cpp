#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_library_init(py::module& m)
    {
        py::class_<GateLibrary, RawPtrWrapper<GateLibrary>> py_gate_library(m, "GateLibrary", R"(
            A gate library is a collection of gate types including their pins and Boolean functions.
        )");

        py_gate_library.def(py::init<const std::filesystem::path&, const std::string&>(), py::arg("path"), py::arg("name"), R"(
            Construct a gate library by specifying its name and the path to the file that describes the library.

            :param hal_py.hal_path path: The path to the gate library file.
            :param str name: The name of the gate library.
        )");

        py_gate_library.def_property_readonly("name", &GateLibrary::get_name, R"(
            The name of the gate library.

            :type: str
        )");

        py_gate_library.def("get_name", &GateLibrary::get_name, R"(
            Get the name of the gate library.

            :returns: The name of the gate library.
            :rtype: str
        )");

        py_gate_library.def_property_readonly("path", &GateLibrary::get_name, R"(
            The path to the file describing the gate library.

            :type: hal_py.hal_path
        )");

        py_gate_library.def("get_path", &GateLibrary::get_name, R"(
            Get the path to the file describing the gate library.

            :returns: The path to the gate library file.
            :rtype: hal_py.hal_path
        )");

        // TODO find a way to enable 'add_gate_type' via Python
        // py_gate_library.def("add_gate_type", &GateLibrary::add_gate_type, py::arg("gate_type"), R"(
        //     Add a gate type to the gate library.

        //     :param hal_py.GateType gate_type: The gate type to add.
        // )");

        py_gate_library.def("contains_gate_type", &GateLibrary::contains_gate_type, py::arg("gate_type"), R"(
            Check whether the given gate type is contained in this library.

            :param hal_py.GateType gate_type: The gate type to check.
            :returns: True if the gate type is part of this library, false otherwise.
            :rtype: bool
        )");

        py_gate_library.def_property_readonly("gate_types", &GateLibrary::get_gate_types, R"(
            All gate types of the gate library as as dict from gate type names to gate types.

            :type: dict[str,hal_py.GateType]
        )");

        py_gate_library.def("get_gate_types", &GateLibrary::get_gate_types, R"(
            Get all gate types of the library.

            :returns: A dict from gate type names to gate types.
            :rtype: dict[str,hal_py.GateType]
        )");

        py_gate_library.def_property_readonly("vcc_gate_types", &GateLibrary::get_vcc_gate_types, R"(
            All VCC gate types of the gate library as as dict from gate type names to gate types.

            :type: dict[str,hal_py.GateType]
        )");

        py_gate_library.def("get_vcc_gate_types", &GateLibrary::get_vcc_gate_types, R"(
            Get all VCC gate types of the library.

            :returns: A dict from VCC gate type names to gate type objects.
            :rtype: dict[str,hal_py.GateType]
        )");

        py_gate_library.def_property_readonly("gnd_gate_types", &GateLibrary::get_vcc_gate_types, R"(
            All GND gate types of the gate library as as dict from gate type names to gate types.

            :type: dict[str,hal_py.GateType]
        )");

        py_gate_library.def("get_gnd_gate_types", &GateLibrary::get_gnd_gate_types, R"(
            Get all GND gate types of the library.

            :returns: A dict from GND gate type names to gate type objects.
            :rtype: dict[str,hal_py.GateType]
        )");

        py_gate_library.def("add_include", &GateLibrary::add_include, py::arg("include"), R"(
            Add an include required for parsing a corresponding netlist, e.g., VHDL libraries.

            :param str inc: The include to add.
        )");

        py_gate_library.def_property_readonly("includes", &GateLibrary::get_includes, R"(
            A list of includes required for parsing a corresponding netlist, e.g., VHDL libraries.

            :type: list[str]
        )");

        py_gate_library.def("get_includes", &GateLibrary::get_includes, R"(
            Get a list of includes required for parsing a corresponding netlist, e.g., VHDL libraries.

            :returns: A list of includes.
            :rtype: list[str]
        )");
    }
}    // namespace hal
