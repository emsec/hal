#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_init(py::module& m)
    {
        py::enum_<GateTypeProperty>(m, "GateTypeProperty", R"(
            A set of available properties for a gate type.
        )")
            .value("combinational", GateTypeProperty::combinational, R"(Combinational gate type.)")
            .value("sequential", GateTypeProperty::sequential, R"(Sequential gate type.)")
            .value("power", GateTypeProperty::power, R"(Power gate type.)")
            .value("ground", GateTypeProperty::ground, R"(Ground gate type.)")
            .value("lut", GateTypeProperty::lut, R"(LUT gate type.)")
            .value("ff", GateTypeProperty::ff, R"(Flip-flop gate type.)")
            .value("latch", GateTypeProperty::latch, R"(Latch gate type.)")
            .value("ram", GateTypeProperty::ram, R"(RAM gate type.)")
            .value("io", GateTypeProperty::io, R"(IO gate type.)")
            .value("dsp", GateTypeProperty::dsp, R"(DSP gate type.)")
            .value("mux", GateTypeProperty::mux, R"(MUX gate type.)")
            .value("buffer", GateTypeProperty::buffer, R"(Buffer gate type.)")
            .value("carry", GateTypeProperty::carry, R"(Carry gate type.)")
            .export_values();

        py::enum_<PinDirection>(m, "PinDirection", R"(
            Defines the direction of a pin.
        )")
            .value("none", PinDirection::none, R"(Invalid pin.)")
            .value("input", PinDirection::input, R"(Input pin.)")
            .value("output", PinDirection::output, R"(Output pin.)")
            .value("inout", PinDirection::inout, R"(Inout pin.)")
            .value("internal", PinDirection::internal, R"(Internal pin.)")
            .export_values();

        py::enum_<PinType>(m, "PinType", R"(Defines the type of a pin.)")
            .value("none", PinType::none, R"(Default pin.)")
            .value("power", PinType::power, R"(Power pin.)")
            .value("ground", PinType::ground, R"(Ground pin.)")
            .value("lut", PinType::lut, R"(Pin that generates output from LUT initialization string.)")
            .value("state", PinType::state, R"(Pin that generates output from internal state.)")
            .value("neg_state", PinType::neg_state, R"(Pin that generates output from negated internal state.)")
            .value("clock", PinType::clock, R"(Clock pin.)")
            .value("enable", PinType::enable, R"(Enable pin.)")
            .value("set", PinType::set, R"(Set pin.)")
            .value("reset", PinType::reset, R"(Reset pin.)")
            .value("data", PinType::data, R"(Data pin.)")
            .value("address", PinType::address, R"(Address pin.)")
            .value("io_pad", PinType::io_pad, R"(IO pad pin.)")
            .value("select", PinType::select, R"(Select pin.)")
            .export_values();

        py::enum_<AsyncSetResetBehavior>(m, "AsyncSetResetBehavior", R"(
            Defines the behavior of the gate type in case both asynchronous set and reset are active at the same time.
        )")
            .value("L", AsyncSetResetBehavior::L, R"(Set the internal state to '0'.)")
            .value("H", AsyncSetResetBehavior::H, R"(Set the internal state to '1'.)")
            .value("N", AsyncSetResetBehavior::N, R"(Do not change the internal state.)")
            .value("T", AsyncSetResetBehavior::T, R"(Toggle, i.e., invert the internal state.)")
            .value("X", AsyncSetResetBehavior::X, R"(Set the internal state to 'X'.)")
            .value("undef", AsyncSetResetBehavior::undef, R"(Invalid bahavior, used by default.)")
            .export_values();

        py::class_<GateType, RawPtrWrapper<GateType>> py_gate_type(m, "GateType", R"(
            A gate type contains information about its internals such as input and output pins as well as its Boolean functions.
        )");

        py_gate_type.def_property_readonly("components", &GateType::get_components, R"(
            All components of the gate type as a list.

            :type: list[hal_py.GateTypeComponent]
        )");

        py_gate_type.def("get_components", &GateType::get_components, py::arg("filter") = nullptr, R"(
            Get all components matching the filter condition (if provided) as a list. 
            Returns an empty list if (i) the gate type does not contain any components or (ii) no component matches the filter condition.

            :param lambda filter: The filter applied to all candidate components, disabled by default.
            :returns: The components.
            :rtype: list[hal_py.GateTypeComponent]
        )");

        py_gate_type.def("get_component", &GateType::get_component, py::arg("filter") = nullptr, R"(
            Get a single component matching the filter condition (if provided).
            Returns None if (i) the gate type does not contain any components, (ii) multiple components match the filter condition, or (iii) no component matches the filter condition.

            :param lambda filter: The filter applied to all candidate components.
            :returns: The component or None.
            :rtype: hal_py.GateTypeComponent or None
        )");

        py_gate_type.def("has_component_of_type", &GateType::has_component_of_type, py::arg("type"), R"(
            Check if the gate type contains a component of the specified type.

            :param hal_py.GateTypeComponent.ComponentType type: The component type to check for.
            :returns: True if the gate type contains a component of the speciifed type, False otherwise.
            :rtype: bool
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

        py_gate_type.def_property_readonly("properties", &GateType::get_properties, R"(
            The properties assigned to the gate type.

            :type: set[hal_py.GateTypeProperty]
        )");

        py_gate_type.def("assign_property", &GateType::assign_property, py::arg("property"), R"(
            Assign a new property to the gate type.

            :param hal_py.GateTypeProperty property: The property to assign.
        )");

        py_gate_type.def("get_properties", &GateType::get_properties, R"(
            Get the properties assigned to the gate type.

            :returns: The properties of the gate type.
            :rtype: set[hal_py.GateTypeProperty]
        )");

        py_gate_type.def("has_property", &GateType::has_property, R"(
            Check whether the gate type has the specified property.

            :param hal_py.GateTypeProperty type: The property to check for.
            :returns: True if the gate type has the specified property, false otherwise.
            :rtype: bool
        )");

        py_gate_type.def_property_readonly("gate_library", &GateType::get_gate_library, R"(
            The gate library this gate type is associated with.

            :type: hal_py.GateLibrary
        )");

        py_gate_type.def("get_gate_library", &GateType::get_gate_library, R"(
            Get the gate library this gate type is associated with.

            :returns: The gate library.
            :rtype: hal_py.GateLibrary
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

        py_gate_type.def(
            "create_pin",
            [](GateType& self, const u32 id, const std::string& name, PinDirection direction, PinType type = PinType::none) -> GatePin* {
                auto res = self.create_pin(id, name, direction, type);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("id"),
            py::arg("name"),
            py::arg("direction"),
            py::arg("type") = PinType::none,
            R"(
            Create a gate pin with the specified name.

            :param int id: The ID of the pin.
            :param str name: The name of the pin.
            :param hal_py.PinDirection direction: The direction of the pin.
            :param hal_py.PinType type: The type of the pin. Defaults to hal_py.PinType.none.
            :returns: The gate pin on success, None otherwise.
            :rtype: hal_py.GatePin or None
        )");

        py_gate_type.def(
            "create_pin",
            [](GateType& self, const std::string& name, PinDirection direction, PinType type = PinType::none) -> GatePin* {
                auto res = self.create_pin(name, direction, type);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("name"),
            py::arg("direction"),
            py::arg("type") = PinType::none,
            R"(
            Create a gate pin with the specified name.
            The ID of the pin is set automatically.

            :param str name: The name of the pin.
            :param hal_py.PinDirection direction: The direction of the pin.
            :param hal_py.PinType type: The type of the pin. Defaults to hal_py.PinType.none.
            :returns: The gate pin on success, None otherwise.
            :rtype: hal_py.GatePin or None
        )");

        py_gate_type.def_property_readonly(
            "pins",
            [](const GateType& self) -> std::vector<GatePin*> { return self.get_pins(); },
            R"(
            An ordered list of all pins of the gate type.

            :type: list[hal_py.GatePin]
        )");

        py_gate_type.def("get_pins", &GateType::get_pins, R"(
            Get an ordered list of all pins of the gate type.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :returns: An ordered list of pins.
            :rtype: list[hal_py.GatePin]
        )");

        py_gate_type.def_property_readonly(
            "pin_names",
            [](const GateType& self) -> std::vector<std::string> { return self.get_pin_names(); },
            R"(
            An ordered list of the names of all pins of the gate type.

            :type: list[str]
        )");

        py_gate_type.def("get_pin_names", &GateType::get_pin_names, R"(
            Get an ordered list of the names of all pins of the gate type.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :returns: A list of input pin names of the gate type.
            :param lambda filter: An optional filter.
            :returns: An ordered list of pins.
            :rtype: list[str]
        )");

        py_gate_type.def_property_readonly("input_pins", &GateType::get_input_pins, R"(
            An ordered list of all input pins of the gate type (including inout pins).

            :type: list[hal_py.GatePin]
        )");

        py_gate_type.def("get_input_pins", &GateType::get_input_pins, R"(
            Get an ordered list of all input pins of the gate type (including inout pins).

            :returns: An ordered list of input pins.
            :rtype: list[hal_py.GatePin]
        )");

        py_gate_type.def_property_readonly("input_pin_names", &GateType::get_input_pin_names, R"(
            An ordered list of the names of all input pins of the gate type (including inout pins).

            :type: list[str]
        )");

        py_gate_type.def("get_input_pin_names", &GateType::get_input_pin_names, R"(
            Get an ordered list of the names of all input pins of the gate type (including inout pins).

            :returns: An ordered list of input pin names.
            :rtype: list[str]
         )");

        py_gate_type.def_property_readonly("output_pins", &GateType::get_output_pins, R"(
            An ordered list of all output pins of the gate type (including inout pins).

            :type: list[hal_py.GatePin]
        )");

        py_gate_type.def("get_output_pins", &GateType::get_output_pins, R"(
            Get an ordered list of all output pins of the gate type (including inout pins).

            :returns: An ordered list of output pins.
            :rtype: list[hal_py.GatePin]
        )");

        py_gate_type.def_property_readonly("output_pin_names", &GateType::get_output_pin_names, R"(
            An ordered list of the names of all output pins of the gate type (including inout pins).

            :type: list[str]
        )");

        py_gate_type.def("get_output_pin_names", &GateType::get_output_pin_names, R"(
            Get an ordered list of the names of all output pins of the gate type (including inout pins).

            :returns: An ordered list of output pin names.
            :rtype: list[str]
        )");

        py_gate_type.def("get_pin_by_id", &GateType::get_pin_by_id, py::arg("id"), R"(
            Get the pin corresponding to the given ID.

            :param int id: The ID of the pin.
            :returns: The pin on success, None otherwise.
            :rtype: hal_py.GatePin or None
        )");

        py_gate_type.def("get_pin_by_name", &GateType::get_pin_by_name, py::arg("name"), R"(
            Get the pin corresponding to the given name.

            :param str name: The name of the pin.
            :returns: The pin on success, None otherwise.
            :rtype: hal_py.GatePin or None
        )");

        py_gate_type.def(
            "create_pin_group",
            [](GateType& self,
               const u32 id,
               const std::string& name,
               const std::vector<GatePin*> pins = {},
               PinDirection direction           = PinDirection::none,
               PinType type                     = PinType::none,
               bool ascending                   = false,
               u32 start_index                  = 0) -> PinGroup<GatePin>* {
                auto res = self.create_pin_group(id, name, pins, direction, type, ascending, start_index);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin group:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("id"),
            py::arg("name"),
            py::arg("pins")        = std::vector<GatePin*>(),
            py::arg("direction")   = PinDirection::none,
            py::arg("type")        = PinType::none,
            py::arg("ascending")   = false,
            py::arg("start_index") = 0,
            R"(
            Create a gate pin group with the given name.

            :param int id: The ID of the pin group.
            :param str name: The name of the pin group.
            :param list[hal_py.GatePin] pins: The pins to be assigned to the pin group. Defaults to an empty list.
            :param hal_py.PinDirection direction: The direction of the pin group. Defaults to hal_py.PinDirection.none.
            :param hal_py.PinType type: The type of the pin group. Defaults to hal_py.PinType.none.
            :param bool ascending: Set True for ascending pin order (from 0 to n-1), False otherwise (from n-1 to 0). Defaults to False.
            :param int start_index: The start index of the pin group. Defaults to 0.
            :returns: The pin group on success, None otherwise.
            :rtype: hal_py.GatePinGroup or None
        )");

        py_gate_type.def(
            "create_pin_group",
            [](GateType& self,
               const std::string& name,
               const std::vector<GatePin*> pins = {},
               PinDirection direction           = PinDirection::none,
               PinType type                     = PinType::none,
               bool ascending                   = false,
               u32 start_index                  = 0) -> PinGroup<GatePin>* {
                auto res = self.create_pin_group(name, pins, direction, type, ascending, start_index);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while creating pin group:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("name"),
            py::arg("pins")        = std::vector<GatePin*>(),
            py::arg("direction")   = PinDirection::none,
            py::arg("type")        = PinType::none,
            py::arg("ascending")   = false,
            py::arg("start_index") = 0,
            R"(
            Create a gate pin group with the given name.
            The ID of the pin group is set automatically.

            :param str name: The name of the pin group.
            :param list[hal_py.GatePin] pins: The pins to be assigned to the pin group. Defaults to an empty list.
            :param hal_py.PinDirection direction: The direction of the pin group. Defaults to hal_py.PinDirection.none.
            :param hal_py.PinType type: The type of the pin group. Defaults to hal_py.PinType.none.
            :param bool ascending: Set True for ascending pin order (from 0 to n-1), False otherwise (from n-1 to 0). Defaults to False.
            :param int start_index: The start index of the pin group. Defaults to 0.
            :returns: The pin group on success, None otherwise.
            :rtype: hal_py.GatePinGroup or None
        )");

        py_gate_type.def("get_pin_group_by_id", &GateType::get_pin_group_by_id, py::arg("id"), R"(
            Get the pin group corresponding to the given ID.

            :param int id: The ID of the pin group.
            :returns: The pin group on success, None otherwise.
            :rtype: hal_py.GatePinGroup or None
        )");

        py_gate_type.def("get_pin_group_by_name", &GateType::get_pin_group_by_name, py::arg("name"), R"(
            Get the pin group corresponding to the given name.

            :param str name: The name of the pin group.
            :returns: The pin group on success, None otherwise.
            :rtype: hal_py.GatePinGroup or None
        )");

        py_gate_type.def("add_boolean_function", &GateType::add_boolean_function, py::arg("pin_name"), py::arg("function"), R"(
            Add a Boolean function with the specified name to the gate type.

            :param str name: The name of the Boolean function.
            :param hal_py.BooleanFunction function: The Boolean function.
        )");

        py_gate_type.def("add_boolean_functions", &GateType::add_boolean_functions, py::arg("functions"), R"(
            Add multiple boolean functions to the gate type.

            :param dict[str,hal_py.BooleanFunction] functions: A dict from names to Boolean functions.
        )");

        py_gate_type.def_property_readonly("boolean_functions", &GateType::get_boolean_functions, R"(
            All Boolean functions of the gate type as a dict from names to Boolean functions.
            
            :type: dict[str,hal_py.BooleanFunction]
        )");

        py_gate_type.def("get_boolean_functions", &GateType::get_boolean_functions, R"(
            Get all Boolean functions of the gate type.

            :returns: A map from names to Boolean functions.
            :rtype: dict[str,hal_py.BooleanFunction]
        )");

        py_gate_type.def("get_boolean_function", py::overload_cast<const std::string&>(&GateType::get_boolean_function, const_), py::arg("name"), R"(
            Get the Boolean function specified by the given name.
            This name can for example be an output pin of the gate or any other user-defined function name.

            :param str name: The name of the Boolean function.
            :returns: The Boolean function on success, an empty Boolean function otherwise.
            :rtype: hal_py.BooleanFunction
        )");

        py_gate_type.def("get_boolean_function", py::overload_cast<const GatePin*>(&GateType::get_boolean_function, const_), py::arg("pin") = nullptr, R"(
            Get the Boolean function corresponding to the given output pin.
            If pin is a None, the Boolean function of the first output pin is returned.

            :param hal_py.GatePin pin: The pin.
            :returns: The Boolean function on success, an empty Boolean function otherwise.
            :rtype: hal_py.BooleanFunction
        )");
    }
}    // namespace hal
