#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_init(py::module& m)
    {
        py::class_<Gate, DataContainer, RawPtrWrapper<Gate>> py_gate(m, "Gate", R"(Gate class containing information about a gate including its location, functions, and module.)");

        py_gate.def(py::self == py::self, R"(
            Check whether two gates are equal.
            Does not check for connected nets or containing module.

            :returns: True if both gates are equal, false otherwise.
            :rtype: bool
        )");

        py_gate.def(py::self != py::self, R"(
            Check whether two gates are unequal.
            Does not check for connected nets or containing module.

            :returns: True if both gates are unequal, false otherwise.
            :rtype: bool
        )");

        py_gate.def_property_readonly("id", &Gate::get_id, R"(
            The unique ID of the gate.

            :type: int
        )");

        py_gate.def("get_id", &Gate::get_id, R"(
            Get the unique ID of the gate.

            :returns: The unique id.
            :type: int
        )");

        py_gate.def_property_readonly(
            "netlist", [](Gate* g) { return RawPtrWrapper<Netlist>(g->get_netlist()); }, R"(
            The netlist this gate is associated with.

            :type: hal_py.Netlist
        )");

        py_gate.def(
            "get_netlist", [](Gate* g) { return RawPtrWrapper<Netlist>(g->get_netlist()); }, R"(
            Get the netlist this gate is associated with.

            :returns: The netlist.
            :rtype: hal_py.Netlist
        )");

        py_gate.def_property("name", &Gate::get_name, &Gate::set_name, R"(
            The name of the gate.

            :type: str
        )");

        py_gate.def("get_name", &Gate::get_name, R"(
            Get the name of the gate.

            :returns: The name.
            :rtype: str
        )");

        py_gate.def("set_name", &Gate::set_name, py::arg("name"), R"(
            Set the name of the gate.

            :param str name: The new name.
        )");

        py_gate.def_property_readonly("type", &Gate::get_type, R"(
            The type of the gate

            :type: hal_py.Gate_type
        )");

        py_gate.def("get_type", &Gate::get_type, R"(
            Get the type of the gate.

            :returns: The gate's type.
            :rtype: hal_py.Gate_type
        )");

        py_gate.def("has_location", &Gate::has_location, R"(
            Checks whether the gate's location in the layout is available.

            :returns: True if valid location data is available, false otherwise.
            :rtype: bool
        )");

        py_gate.def_property("location_x", &Gate::get_location_x, &Gate::set_location_x, R"(
            The x-coordinate of the physical location of the gate in the layout. Only positive values are valid, negative values will be regarded as no location assigned.

            :type: int
        )");

        py_gate.def("get_location_x", &Gate::get_location_x, R"(
            Get the physical location x-coordinate of the gate in the layout.
            If no valid physical location x-coordinate is assigned, a negative value is returned.

            :returns: The gate's x-coordinate.
            :rtype: int
        )");

        py_gate.def("set_location_x", &Gate::set_location_x, py::arg("x"), R"(
            Set the physical location x-coordinate of the gate in the layout.
            Only positive values are valid, negative values will be regarded as no location assigned.

            :param int x: The gate's x-coordinate.
        )");

        py_gate.def_property("location_y", &Gate::get_location_y, &Gate::set_location_y, R"(
            The y-coordinate of the physical location of the gate in the layout.
            Only positive values are valid, negative values will be regarded as no location assigned.

            :type: int
        )");

        py_gate.def("get_location_y", &Gate::get_location_y, R"(
            Get the physical location y-coordinate of the gate in the layout.
            If no valid physical location y-coordinate is assigned, a negative value is returned.

            :returns: The gate's y-coordinate.
            :rtype: int
        )");

        py_gate.def("set_location_y", &Gate::set_location_y, py::arg("y"), R"(
            Set the physical location y-coordinate of the gate in the layout.
            Only positive values are valid, negative values will be regarded as no location assigned.

            :param int y: The gate's y-coordinate.
        )");

        py_gate.def_property("location", &Gate::get_location, &Gate::set_location, R"(
            The physical location of the gate in the layout.
            Only positive values are valid, negative values will be regarded as no location assigned.

            :type: tuple(int,int)
        )");

        py_gate.def("get_location", &Gate::get_location, R"(
            Get the physical location of the gate in the layout.
            If no valid physical location coordinate is assigned, a negative value is returned for the respective coordinate.

            :returns: A tuple <x-coordinate, y-coordinate>.
            :rtype: tuple(int,int)
        )");

        py_gate.def("set_location", &Gate::set_location, py::arg("location"), R"(
            Set the physical location of the gate in the layout.
            Only positive coordinates are valid, negative values will be regarded as no location assigned.

            :param tuple(int,int) location: A tuple <x-coordinate, y-coordinate>.
        )");

        py_gate.def_property_readonly("module", &Gate::get_module, R"(
            The module in which contains this gate.

            :type: hal_py.Module
        )");

        py_gate.def("get_module", &Gate::get_module, R"(
            Get the module which contains this gate.

            :returns: The module.
            :rtype: hal_py.Module
        )");

        py_gate.def("get_grouping", &Gate::get_grouping, R"(
            Gets the grouping in which this gate is contained.
            If no grouping contains this gate, *None* is returned.

            :returns: The grouping.
            :rtype: hal_py.Grouping
        )");

        py_gate.def("get_boolean_function", &Gate::get_boolean_function, py::arg("name") = "", R"(
            Get the boolean function associated with a specific name. This name can for example be an output pin of the gate or a defined functionality like "reset".
            If name is empty, the function of the first output pin is returned. If there is no function for the given name, the constant 'X' is returned.

            :param str name: The function name.
            :returns: The boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_gate.def_property_readonly(
            "boolean_functions", [](Gate* g) { return g->get_boolean_functions(); }, R"(
            A dictionary from function name to boolean function for all boolean functions associated with this gate.

            :rtype: dict[str,hal_py.BooleanFunction]
        )");

        py_gate.def("get_boolean_functions", &Gate::get_boolean_functions, py::arg("only_custom_functions") = false, R"(
            Get a dictionary from function name to boolean function for all boolean functions associated with this gate.

            :param bool only_custom_functions: If true, this returns only the functions which were set via :func:`add_boolean_function`.
            :returns: A map from function name to function.
            :rtype: dict[str,hal_py.BooleanFunction]
        )");

        py_gate.def("add_boolean_function", &Gate::add_boolean_function, py::arg("name"), py::arg("func"), R"(
            Add the boolean function with the specified name only for this gate.

            :param str name:  The function name, usually an output port.
            :param hal_py.BooleanFunction func:  The function.
        )");

        py_gate.def("mark_vcc_gate", &Gate::mark_vcc_gate, R"(
            Mark this gate as a global vcc gate.

            :returns: True on success.
            :rtype: bool
        )");

        py_gate.def("mark_gnd_gate", &Gate::mark_gnd_gate, R"(
            Mark this gate as a global gnd gate.

            :returns: True on success.
            :rtype: bool
        )");

        py_gate.def("unmark_vcc_gate", &Gate::unmark_vcc_gate, R"(
            Unmark this gate as a global vcc gate.

            :returns: True on success.
            :rtype: bool
        )");

        py_gate.def("unmark_gnd_gate", &Gate::unmark_gnd_gate, R"(
            Unmark this gate as a global gnd gate.

            :returns: True on success.
            :rtype: bool
        )");

        py_gate.def("is_vcc_gate", &Gate::is_vcc_gate, R"(
            Checks whether this gate is a global vcc gate.

            :returns: True if the gate is a global vcc gate.
            :rtype: bool
        )");

        py_gate.def("is_gnd_gate", &Gate::is_gnd_gate, R"(
            Checks whether this gate is a global gnd gate.

            :returns: True if the gate is a global gnd gate.
            :rtype: bool
        )");

        py_gate.def_property_readonly("input_pins", &Gate::get_input_pins, R"(
            A list of all input pin types of the gate.

            :type: list[str]
        )");

        py_gate.def("get_input_pins", &Gate::get_input_pins, R"(
            Get a list of all input pin types of the gate.

            :returns: A list of input pin types.
            :rtype: list[str]
        )");

        py_gate.def_property_readonly("output_pins", &Gate::get_output_pins, R"(
            A list of all output pin types of the gate.

            :type: list[str]
        )");

        py_gate.def("get_output_pins", &Gate::get_output_pins, R"(
            Get a list of all output pin types of the gate.

            :returns: A list of output pin types.
            :rtype: list[str]
        )");

        py_gate.def_property_readonly("fan_in_nets", &Gate::get_fan_in_nets, R"(
            A list of all fan-in nets of the gate, i.e. all nets that are connected to one of the input pins.

            :type: list[hal_py.Net]
        )");

        py_gate.def("get_fan_in_nets", &Gate::get_fan_in_nets, R"(
            Get a list of all fan-in nets of the gate, i.e. all nets that are connected to one of the input pins.

            :returns: A list of all connected input nets.
            :rtype: list[hal_py.Net]
        )");

        py_gate.def("get_fan_out_endpoints", &Gate::get_fan_out_endpoints, R"(
            Get a list of all fan-out endpoints of the gate,
            i.e. all connected endpoints that represent an output pin of this gate.

            :returns: A list of all output endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def("get_fan_in_net", &Gate::get_fan_in_net, py::arg("pin"), R"(
            Get the fan-in net which is connected to a specific input pin.
            If the input pin type is unknown or no net is connected, *None* is returned.

            :param str pin: The input pin type.
            :returns: The connected input net.
            :rtype: hal_py.Net
        )");

        py_gate.def("get_fan_in_endpoint", &Gate::get_fan_in_endpoint, py::arg("pin"), R"(
            Get the fan-out endpoint which represents a specific input pin.
            If the input pin type is unknown or no net is connected, *None* is returned.

            :param str pin: The output pin type.
            :returns: The output endpoint.
            :rtype: hal_py.Endpoint
        )");

        py_gate.def_property_readonly("fan_out_nets", &Gate::get_fan_out_nets, R"(
            A list of all fan-out nets of the gate, i.e. all nets that are connected to one of the output pins.

            :returns: A list of all connected output nets.
            :rtype: list[hal_py.Net]
        )");

        py_gate.def("get_fan_out_nets", &Gate::get_fan_out_nets, R"(
            Get a list of all fan-out nets of the gate, i.e. all nets that are connected to one of the output pins.

            :returns: A list of all connected output nets.
            :rtype: list[hal_py.Net]
        )");

        py_gate.def("get_fan_in_endpoints", &Gate::get_fan_in_endpoints, R"(
            Get a list of all fan-in endpoints of the gate,
            i.e. all connected endpoints that represent an input pin of this gate.

            :returns: A list of all connected input endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def("get_fan_out_net", &Gate::get_fan_out_net, py::arg("pin"), R"(
            Get the fan-out net which is connected to a specific output pin.
            If the output pin type is unknown or no net is connected, *None* is returned.

            :param str pin: The output pin type.
            :returns: The connected output net.
            :rtype: hal_py.Net
        )");

        py_gate.def("get_fan_out_endpoint", &Gate::get_fan_out_endpoint, py::arg("pin"), R"(
            Get the fan-out endpoint which represents a specific output pin.
            If the input pin type is unknown or no net is connected to the respective pin, *None* is returned.

            :param str pin: The output pin type.
            :returns: The connected output endpoint.
            :rtype: hal_py.Endpoint
        )");

        py_gate.def_property_readonly(
            "unique_predecessors", [](Gate* g) { return g->get_unique_predecessors(); }, R"(
            A list of all unique predecessor gates of the gate.

            :type: list[hal_py.Gate]
        )");

        py_gate.def("get_unique_predecessors", &Gate::get_unique_predecessors, py::arg("filter") = nullptr, R"(
            Get a list of all unique predecessor gates of the gate.
            A filter can be supplied which filters out all potential values that return false.

            :param lambda filter: A function to filter the output, using the input pin type of the gate (1st param) and a connected predecessor endpoint (2nd param). Leave empty for no filtering.
            :returns: A list of unique predecessors endpoints.
            :rtype: list[hal_py.Gate]
        )");

        py_gate.def_property_readonly(
            "predecessors", [](Gate* g) { return g->get_predecessors(); }, R"(
            A list of all all direct predecessor endpoints of the gate.

            :type: list[hal_py.Endpoint]
        )");

        py_gate.def("get_predecessors", &Gate::get_predecessors, py::arg("filter") = nullptr, R"(
            Get a list of all direct predecessor endpoints of the gate filterable by the gate's input pin and a specific gate type.

            :param lambda filter: The function used for filtering. Leave empty for no filtering.
            :returns: A list of predecessors endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def("get_predecessor", &Gate::get_predecessor, py::arg("which_pin"), R"(
            Get the direct predecessor endpoint of the gate connected to a specific input pin.
            If the input pin type is unknown or there is no predecessor endpoint or there are multiple predecessor
            endpoints, *None* is returned.

            :param str which_pin: The input pin type of the this gate.
            :returns: The predecessor endpoint.
            :rtype: hal_py.Endpoint
        )");

        py_gate.def_property_readonly(
            "unique_successors", [](Gate* g) { return g->get_unique_successors(); }, R"(
            A list of all unique successor gates of the gate.

            :type: list[hal_py.Gate]
        )");

        py_gate.def("get_unique_successors", &Gate::get_unique_successors, py::arg("filter") = nullptr, R"(
            Get a list of all unique successor gates of the gate.
            A filter can be supplied which filters out all potential values that return false.

            :param lambda filter: A function to filter the output, using the output pin type of the gate (1st param) and a connected successor endpoint (2nd param). Leave empty for no filtering.
            :returns: A list of unique successor endpoints.
            :rtype: list[hal_py.Gate]
        )");

        py_gate.def_property_readonly(
            "successors", [](Gate* g) { return g->get_successors(); }, R"(
            A list of all direct successor endpoints of the gate.

            :type: list[hal_py.Endpoint]
        )");

        py_gate.def("get_successors", &Gate::get_successors, py::arg("filter") = nullptr, R"(
            Get a list of all direct successor endpoints of the gate.
            A filter can be supplied which filters out all potential values that return false.

            :param lambda filter: A function to filter the output, using the output pin type of the gate (1st param) and a connected successor endpoint (2nd param). Leave empty for no filtering.
            :returns: A list of successor endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def("get_successor", &Gate::get_successor, py::arg("which_pin"), R"(
            Get the direct successor endpoint of the gate connected to a specific input pin.
            If the input pin type is unknown or there is no successor endpoint or there are multiple successor
            endpoints, *None* is returned.

            :param str which_pin: The output pin type of this gate to get the successor from.
            :returns: The successor endpoint.
            :rtype: hal_py.Endpoint
        )");
    }
}    // namespace hal
