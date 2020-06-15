#include "bindings.h"

namespace hal
{
    void gate_init(py::module& m)
    {
        py::class_<gate, data_container, std::shared_ptr<gate>> py_gate(m, "gate", R"(Gate class containing information about a gate including its location, functions, and module.)");

        py_gate.def_property_readonly("id", &gate::get_id, R"(
        The unique ID of the gate.

        :type: int
)");

        py_gate.def("get_id", &gate::get_id, R"(
        Gets the gate's unique id.

        :returns: The gate's id.
        :type: int
)");

        py_gate.def_property_readonly("netlist", &gate::get_netlist, R"(
        The parent netlist of the gate.

        :type: hal_py.netlist
)");

        py_gate.def("get_netlist", &gate::get_netlist, R"(
        Gets the parent netlist of the gate.

        :returns: The netlist.
        :rtype: hal_py.netlist
)");

        py_gate.def_property("name", &gate::get_name, &gate::set_name, R"(
        The name of the gate.

        :type: str
)");

        py_gate.def("get_name", &gate::get_name, R"(
        Gets the gate's name.

        :returns: The name.
        :rtype: str
)");

        py_gate.def("set_name", &gate::set_name, py::arg("name"), R"(
        Sets the gate's name.

        :param str name: The new name.
)");

        py_gate.def_property_readonly("type", &gate::get_type, R"(
        The type of the gate

        :type: hal_py.gate_type
)");

        py_gate.def("get_type", &gate::get_type, R"(
        Gets the type of the gate.

        :returns: The gate's type.
        :rtype: hal_py.gate_type
)");

        py_gate.def("has_location", &gate::has_location, R"(
        Checks whether the gate's location in the layout is available.

        :returns: True if valid location data is available, false otherwise.
        :rtype: bool
)");

        py_gate.def_property("location_x", &gate::get_location_x, &gate::set_location_x, R"(
        The x-coordinate of the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :type: float
)");

        py_gate.def("get_location_x", &gate::get_location_x, R"(
        Gets the x-coordinate of the physical location of the gate in the layout.

        :returns: The x-coordinate.
        :rtype: float
)");

        py_gate.def("set_location_x", &gate::set_location_x, py::arg("x"), R"(
        Sets the x-coordinate of the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :param float x: The x-coordinate.
)");

        py_gate.def_property("location_y", &gate::get_location_y, &gate::set_location_y, R"(
        The y-coordinate of the physical location of the gate in the layout.
        Only positive values are valid, negative values will be regarded as no location assigned.

        :type: float
)");

        py_gate.def("get_location_y", &gate::get_location_y, R"(
        Gets the y-coordinate of the physical location of the gate in the layout.

        :returns: The  y-coordinate.
        :rtype: float
)");

        py_gate.def("set_location_y", &gate::set_location_y, py::arg("y"), R"(
        Sets the y-coordinate of the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :param float y: The  y-coordinate.
)");

        py_gate.def_property("location", &gate::get_location, &gate::set_location, R"(
        The physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :type: tuple(float,float)
)");

        py_gate.def("get_location", &gate::get_location, R"(
        Gets the physical location of the gate in the layout.

        :returns: A tuple <x-coordinate, y-coordinate>.
        :rtype: tuple(float,float)
)");

        py_gate.def("set_location", &gate::set_location, py::arg("location"), R"(
        Sets the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

        :param tuple(float,float) location: A pair <x-coordinate, y-coordinate>.
)");

        py_gate.def_property_readonly("module", &gate::get_module, R"(
        The module in which this gate is contained.

        :type: hal_py.module
)");

        py_gate.def("get_module", &gate::get_module, R"(
        Gets the module in which this gate is contained.

        :returns: The module.
        :rtype: hal_py.module
)");

        py_gate.def("get_boolean_function", &gate::get_boolean_function, py::arg("name") = "", R"(
        Get the boolean function associated with a specific name. This name can for example be an output pin of the gate or a defined functionality like "reset".
        If name is empty, the function of the first output pin is returned. If there is no function for the given name, the constant 'X' is returned.

        :param str name: The function name.
        :returns: The boolean function.
        :rtype: hal_py.boolean_function
)");

        py_gate.def_property_readonly(
            "boolean_functions", [](const std::shared_ptr<gate>& g) { return g->get_boolean_functions(); }, R"(
        A map from function name to boolean function for all boolean functions associated with this gate.

        :rtype: dict[str,hal_py.boolean_function]
)");

        py_gate.def("get_boolean_functions", &gate::get_boolean_functions, py::arg("only_custom_functions") = false, R"(
        Get a map from function name to boolean function for all boolean functions associated with this gate.

        :param bool only_custom_functions: If true, this returns only the functions which were set via set_boolean_function.
        :returns: A map from function name to function.
        :rtype: dict[str,hal_py.boolean_function]
)");

        py_gate.def("add_boolean_function", &gate::add_boolean_function, py::arg("name"), py::arg("func"), R"(
        Add the boolean function with the specified name only for this gate.

        :param str name:  The function name, usually an output port.
        :param hal_py.boolean_function func:  The function.
)");

        py_gate.def("mark_vcc_gate", &gate::mark_vcc_gate, R"(
        Mark this gate as a global vcc gate.

        :returns: True on success.
        :rtype: bool
)");

        py_gate.def("mark_gnd_gate", &gate::mark_gnd_gate, R"(
        Mark this gate as a global gnd gate.

        :returns: True on success.
        :rtype: bool
)");

        py_gate.def("unmark_vcc_gate", &gate::unmark_vcc_gate, R"(
        Unmark this gate as a global vcc gate.

        :returns: True on success.
        :rtype: bool
)");

        py_gate.def("unmark_gnd_gate", &gate::unmark_gnd_gate, R"(
        Unmark this gate as a global gnd gate.

        :returns: True on success.
        :rtype: bool
)");

        py_gate.def("is_vcc_gate", &gate::is_vcc_gate, R"(
        Checks whether this gate is a global vcc gate.

        :returns: True if the gate is a global vcc gate.
        :rtype: bool
)");

        py_gate.def("is_gnd_gate", &gate::is_gnd_gate, R"(
        Checks whether this gate is a global gnd gate.

        :returns: True if the gate is a global gnd gate.
        :rtype: bool
)");

        py_gate.def_property_readonly("input_pins", &gate::get_input_pins, R"(
        A list of all input pin types of the gate.

        :type: list[str]
)");

        py_gate.def("get_input_pins", &gate::get_input_pins, R"(
        Get a list of all input pin types of the gate.

        :returns: A list of input pin types.
        :rtype: list[str]
)");

        py_gate.def_property_readonly("output_pins", &gate::get_output_pins, R"(
        A list of all output pin types of the gate.

        :type: list[str]
)");

        py_gate.def("get_output_pins", &gate::get_output_pins, R"(
        Get a list of all output pin types of the gate.

        :returns: A list of output pin types.
        :rtype: list[str]
)");

        py_gate.def_property_readonly("fan_in_nets", &gate::get_fan_in_nets, R"(
        A set of all fan-in nets of the gate, i.e. all nets that are connected to one of the input pins.

        :type: set[hal_py.net]
)");

        py_gate.def("get_fan_in_nets", &gate::get_fan_in_nets, R"(
        Get a set of all fan-in nets of the gate, i.e. all nets that are connected to one of the input pins.

        :returns: A set of all connected input nets.
        :rtype: set[hal_py.net]
)");

        py_gate.def("get_fan_in_net", &gate::get_fan_in_net, py::arg("pin_type"), R"(
        Get the fan-in net which is connected to a specific input pin.

        :param str pin_type: The input pin type.
        :returns: The connected input net.
        :rtype: hal_py.net
)");

        py_gate.def_property_readonly("fan_out_nets", &gate::get_fan_out_nets, R"(
        A set of all fan-out nets of the gate, i.e. all nets that are connected to one of the output pins.

        :returns: A set of all connected output nets.
        :rtype: set[hal_py.net]
)");

        py_gate.def("get_fan_out_nets", &gate::get_fan_out_nets, R"(
        Get a set of all fan-out nets of the gate, i.e. all nets that are connected to one of the output pins.

        :returns: A set of all connected output nets.
        :rtype: set[hal_py.net]
)");

        py_gate.def("get_fan_out_net", &gate::get_fan_out_net, py::arg("pin_type"), R"(
        Get the fan-out net which is connected to a specific output pin.

        :param str pin_type: The output pin type.
        :returns: The connected output net.
        :rtype: hal_py.net
)");

        py_gate.def_property_readonly(
            "unique_predecessors", [](const std::shared_ptr<gate>& g) { return g->get_unique_predecessors(); }, R"(
        A set of all unique predecessor gates of the gate.

        :type: list[hal_py.gate]
)");

        py_gate.def("get_unique_predecessors", &gate::get_unique_predecessors, py::arg("filter") = nullptr, R"(
        Get a set of all unique predecessor endpoints of the gate filterable by the gate's input pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A set of unique predecessors endpoints.
        :rtype: set[hal_py.endpoint]
)");

        py_gate.def_property_readonly(
            "predecessors", [](const std::shared_ptr<gate>& g) { return g->get_predecessors(); }, R"(
        A list of all all direct predecessor endpoints of the gate.

        :type: list[hal_py.endpoint]
)");

        py_gate.def("get_predecessors", &gate::get_predecessors, py::arg("filter") = nullptr, R"(
        Get a list of all direct predecessor endpoints of the gate filterable by the gate's input pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A list of predecessors endpoints.
                :rtype: list[hal_py.endpoint]
)");

        py_gate.def("get_predecessor", &gate::get_predecessor, py::arg("which_pin"), R"(
        Get the direct predecessor endpoint of the gate connected to a specific input pin and filterable by a specific gate type.

        :param str which_pin: The input pin type of the this gate. Leave empty for no filtering.
        :returns: The predecessor endpoint.
        :rtype: hal_py.endpoint
)");

        py_gate.def_property_readonly(
            "unique_successors", [](const std::shared_ptr<gate>& g) { return g->get_unique_successors(); }, R"(
        A set of all unique successor gates of the gate.

        :type: list[hal_py.gate]
)");

        py_gate.def("get_unique_successors", &gate::get_unique_successors, py::arg("filter") = nullptr, R"(
        Get a set of all unique successors of the gate filterable by the gate's output pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A set of unique successor endpoints.
        :rtype: set[hal_py.endpoint]
)");

        py_gate.def_property_readonly(
            "successors", [](const std::shared_ptr<gate>& g) { return g->get_successors(); }, R"(
        A list of all direct successor endpoints of the gate.

        :type: list[hal_py.endpoint]
)");

        py_gate.def("get_successors", &gate::get_successors, py::arg("filter") = nullptr, R"(
        Get a list of all direct successor endpoints of the gate filterable by the gate's output pin and a specific gate type.

        :param lambda filter: The function used for filtering. Leave empty for no filtering.
        :returns: A list of successor endpoints.
        :rtype: list[hal_py.endpoint]
)");
    }
}    // namespace hal
