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

        py_gate_library.def("set_gate_location_data_category", &GateLibrary::set_gate_location_data_category, py::arg("category"), R"(
            Set the data category of the gate location information.

            :param str category: The data category.
        )");

        py_gate_library.def("get_gate_location_data_category", &GateLibrary::get_gate_location_data_category, R"(
            Get the data category of the gate location information.

            :returns: The data category.
            :rtype: str
        )");

        py_gate_library.def("set_gate_location_data_identifiers", &GateLibrary::set_gate_location_data_identifiers, py::arg("x_coordinate"), py::arg("y_coordinate"), R"(
            Set the data identifiers of the gate location information for both the x- and y-coordinates.

            :param str x_coordinate: The data identifier for the x-coordinate.
            :param str y_coordinate: The data identifier for the y-coordinate.
        )");

        py_gate_library.def("get_gate_location_data_category", &GateLibrary::get_gate_location_data_category, R"(
            Get the data identifiers of the gate location information for both the x- and y-coordinates.

            :returns: A pair of data identifiers.
            :rtype: tuple(str,str)
        )");

        // py_gate_library.def("create_gate_type", &GateLibrary::create_gate_type, py::arg("name"), py::arg("properties") = std::set<GateTypeProperty>(), R"(
        //     Create a new gate type, add it to the gate library, and return it.

        //     :param str name: The name of the gate type.
        //     :param set[hal_py.GateTypeProperty] properties: The properties of the gate type.
        //     :returns: The new gate type instance on success, None otherwise.
        //     :rtype: hal_py.GateType
        // )");

        py_gate_library.def("contains_gate_type", &GateLibrary::contains_gate_type, py::arg("gate_type"), R"(
            Check whether the given gate type is contained in this library.

            :param hal_py.GateType gate_type: The gate type.
            :returns: True if the gate type is part of this library, false otherwise.
            :rtype: bool
        )");

        py_gate_library.def("contains_gate_type_by_name", &GateLibrary::contains_gate_type_by_name, py::arg("name"), R"(
            Check by name whether the given gate type is contained in this library.

            :param str name: The name of the gate type.
            :returns: True if the gate type is part of this library, false otherwise.
            :rtype: bool
        )");

        py_gate_library.def("get_gate_type_by_name", &GateLibrary::get_gate_type_by_name, py::arg("name"), R"(
            Get the gate type corresponding to the given name if contained within the library. In case there is no gate type with that name, None is returned.

            :param str name: The name of the gate type.
            :returns: The gate type on success, None otherwise.
            :rtype: hal_py.GateType or None
        )");

        py_gate_library.def_property_readonly("gate_types", &GateLibrary::get_gate_types, R"(
            All gate types of the gate library as as dict from gate type names to gate types.

            :type: dict[str,hal_py.GateType]
        )");

        py_gate_library.def("get_gate_types", &GateLibrary::get_gate_types, py::arg("filter") = nullptr, R"(
            Get all gate types of the library.
            In case a filter is applied, only the gate types matching the filter condition are returned.

            :param lambda filter: The user-defined filter function.
            :returns: A dict from gate type names to gate types.
            :rtype: dict[str,hal_py.GateType]
        )");

        py_gate_library.def("mark_vcc_gate_type", &GateLibrary::mark_vcc_gate_type, py::arg("gate_type"), R"(
            Mark a gate type as a VCC gate type.
        
            :param hal_py.GateType gate_type: The gate type.
            :returns: True on success, false otherwise.
            :rtype: bool
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

        py_gate_library.def("mark_gnd_gate_type", &GateLibrary::mark_gnd_gate_type, py::arg("gate_type"), R"(
            Mark a gate type as a GND gate type.
        
            :param hal_py.GateType gate_type: The gate type.
            :returns: True on success, false otherwise.
            :rtype: bool
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
