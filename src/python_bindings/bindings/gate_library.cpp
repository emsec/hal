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

            :param pathlib.Path path: The path to the gate library file.
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

            :type: pathlib.Path
        )");

        py_gate_library.def("get_path", &GateLibrary::get_name, R"(
            Get the path to the file describing the gate library.

            :returns: The path to the gate library file.
            :rtype: pathlib.Path
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

        py_gate_library.def(
            "create_gate_type",
            [](GateLibrary& self, const std::string& name, std::set<GateTypeProperty> properties = {GateTypeProperty::combinational}) -> GateType* { return self.create_gate_type(name, properties); },
            py::arg("name"),
            py::arg("properties") = std::set({GateTypeProperty::combinational}),
            R"(
            Create a new gate type, add it to the gate library, and return it.

            :param str name: The name of the gate type.
            :param set[hal_py.GateTypeProperty] properties: The properties of the gate type. Defaults to ``hal_py.GateTypeProperty.combinational``.
            :returns: The new gate type instance on success, None otherwise.
            :rtype: hal_py.GateType or None
        )");

        py_gate_library.def(
            "create_ff_gate_type",
            [](GateLibrary& self,
               const std::string& name,
               const std::string& state_identifier,
               const std::string& neg_state_identifier,
               const BooleanFunction& next_state_bf,
               const BooleanFunction& clock_bf,
               const std::string& init_category   = "",
               const std::string& init_identifier = "") -> GateType* {
                std::unique_ptr<GateTypeComponent> init_component = nullptr;
                if (!init_category.empty() && !init_identifier.empty())
                {
                    init_component = GateTypeComponent::create_init_component(init_category, {init_identifier});
                }
                std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(std::move(init_component), state_identifier, neg_state_identifier);
                std::unique_ptr<GateTypeComponent> ff_component    = GateTypeComponent::create_ff_component(std::move(state_component), next_state_bf, clock_bf);
                return self.create_gate_type(name, {GateTypeProperty::sequential, GateTypeProperty::ff}, std::move(ff_component));
            },
            py::arg("name"),
            py::arg("state_identifier"),
            py::arg("neg_state_identifier"),
            py::arg("next_state_bf"),
            py::arg("clock_bf"),
            py::arg("init_category")   = "",
            py::arg("init_identifier") = "",
            R"(
            Create a new gate type, add it to the gate library, and return it.

            :param str name: The name of the gate type.
            :param str state_identifier: The identifier of the internal state.
            :param str neg_state_identifier: The identifier of the negated internal state.
            :param hal_py.BooleanFunction next_state_bf: The function describing the internal state.
            :param hal_py.BooleanFunction clock_bf: The function describing the clock input.
            :param str init_category: The initialization data category. Defaults to an empty string.
            :param str init_identifier: The initialization data identifier. Defaults to an empty string.
            :returns: The new flip-flop gate type instance on success, None otherwise.
            :rtype: hal_py.GateType or None
        )");

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

        py_gate_library.def_property_readonly("gate_types", [](const GateLibrary& self) { return self.get_gate_types(); }, R"(
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
