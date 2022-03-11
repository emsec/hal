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

        py_gate.def("__hash__", &Gate::get_hash, R"(
            Python requires hash for set and dict container.

            :returns: The hash.
            :rtype: Py_hash_t
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

        py_gate.def(
            "get_boolean_function",
            [](const Gate& self, const std::string& name) -> BooleanFunction {
                auto res = self.get_boolean_function(name);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting Boolean function:\n{}", res.get_error().get());
                    return BooleanFunction();
                }
            },
            py::arg("name"),
            R"(
            Get the Boolean function specified by the given name.
            This name can for example be an output pin of the gate or any other user-defined function name.

            :param str name: The name.
            :returns: The Boolean function on success, an empty Boolean function otherwise.
            :rtype: hal_py.BooleanFunction
        )");

        py_gate.def(
            "get_boolean_function",
            [](const Gate& self, const GatePin* pin = nullptr) -> BooleanFunction {
                auto res = self.get_boolean_function(pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting Boolean function:\n{}", res.get_error().get());
                    return BooleanFunction();
                }
            },
            py::arg("pin") = nullptr,
            R"(
            Get the Boolean function corresponding to the given output pin.
            If pin is None, the Boolean function of the first output pin is returned.

            :param hal_py.GatePin pin: The pin.
            :returns: The Boolean function on success, an empty Boolean function otherwise.
            :rtype: hal_py.BooleanFunction
        )");

        py_gate.def_property_readonly(
            "boolean_functions",
            [](const Gate& self) -> std::unordered_map<std::string, BooleanFunction> {
                auto res = self.get_boolean_functions();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting Boolean functions:\n{}", res.get_error().get());
                    return {};
                }
            },
            R"(
            A dict from function name to Boolean function containing all Boolean functions associated with the gate.

            :rtype: dict[str,hal_py.BooleanFunction]
        )");

        py_gate.def(
            "get_boolean_functions",
            [](const Gate& self, bool only_custom_functions = false) -> std::unordered_map<std::string, BooleanFunction> {
                auto res = self.get_boolean_functions(only_custom_functions);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting Boolean functions:\n{}", res.get_error().get());
                    return {};
                }
            },
            py::arg("only_custom_functions") = false,
            R"(
            Get a dict from function name to Boolean function containing all Boolean functions associated with the gate.

            :param bool only_custom_functions: Set True to get only Boolean functions that are local to the gate, False otherwise.
            :returns: A dict from function name to function on success, an empty dict otherwise.
            :rtype: dict[str,hal_py.BooleanFunction]
        )");

        py_gate.def(
            "add_boolean_function",
            [](Gate& self, const std::string& name, const BooleanFunction& func) -> bool {
                auto res = self.add_boolean_function(name, func);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while adding Boolean function:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("name"),
            py::arg("func"),
            R"(
            Add a Boolean function with the given name to the gate.

            :param str name: The function name.
            :param hal_py.BooleanFunction func: The function.
            :returns: True on success, False otherwise.
            :rtype: bool
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

        py_gate.def_property_readonly("fan_in_nets", &Gate::get_fan_in_nets, R"(
            A list of all fan-in nets of the gate, i.e., all nets that are connected to one of the input pins of the gate.

            :type: list[hal_py.Net]
        )");

        py_gate.def("get_fan_in_nets", &Gate::get_fan_in_nets, R"(
            Get a list of all fan-in nets of the gate, i.e., all nets that are connected to one of the input pins of the gate.

            :returns: A list of all fan-in nets.
            :rtype: list[hal_py.Net]
        )");

        py_gate.def_property_readonly("fan_in_endpoints", &Gate::get_fan_in_endpoints, R"(
            A list of all fan-in endpoints of the gate, i.e., all endpoints associated with an input pin of the gate.

            :type: list[hal_py.Endpoint]
        )");

        py_gate.def("get_fan_in_endpoints", &Gate::get_fan_in_endpoints, R"(
            Get a list of all fan-in endpoints of the gate, i.e., all endpoints associated with an input pin of the gate.

            :returns: A list of all fan-in endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def(
            "get_fan_in_net",
            [](const Gate& self, const GatePin* pin) -> Net* {
                auto res = self.get_fan_in_net(pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting fan-in net:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("pin"),
            R"(
            Get the fan-in net which is connected to the specified input pin.

            :param hal_py.GatePin pin: The input pin.
            :returns: The fan-in net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");

        py_gate.def(
            "get_fan_in_endpoint",
            [](const Gate& self, const GatePin* pin) -> Endpoint* {
                auto res = self.get_fan_in_endpoint(pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting fan-in endpoint:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("pin"),
            R"(
            Get the fan-in endpoint corresponding to the specified input pin.

            :param hal_py.GatePin pin: The input pin.
            :returns: The fan-in endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint
        )");

        py_gate.def_property_readonly("fan_out_nets", &Gate::get_fan_out_nets, R"(
            A list of all fan-out nets of the gate, i.e., all nets that are connected to one of the output pins of the gate.

            :type: list[hal_py.Net]
        )");

        py_gate.def("get_fan_out_nets", &Gate::get_fan_out_nets, R"(
            Get a list of all fan-out nets of the gate, i.e., all nets that are connected to one of the output pins of the gate.

            :returns: A list of all fan-out nets.
            :rtype: list[hal_py.Net]
        )");

        py_gate.def_property_readonly("fan_out_endpoints", &Gate::get_fan_out_endpoints, R"(
            A list of all fan-out endpoints of the gate, i.e., all endpoints associated with an output pin of the gate.

            :type: list[hal_py.Endpoint]
        )");

        py_gate.def("get_fan_out_endpoints", &Gate::get_fan_out_endpoints, R"(
            Get a list of all fan-out endpoints of the gate, i.e., all endpoints associated with an output pin of the gate.

            :returns: A list of all fan-out endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def(
            "get_fan_out_net",
            [](const Gate& self, const GatePin* pin) -> Net* {
                auto res = self.get_fan_out_net(pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting fan-out net:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("pin"),
            R"(
            Get the fan-out net which is connected to the specified output pin.

            :param hal_py.GatePin pin: The output pin.
            :returns: The fan-out net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");

        py_gate.def(
            "get_fan_out_endpoint",
            [](const Gate& self, const GatePin* pin) -> Endpoint* {
                auto res = self.get_fan_out_endpoint(pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting fan-out endpoint:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("pin"),
            R"(
            Get the fan-out endpoint corresponding to the specified output pin.

            :param hal_py.GatePin pin: The output pin.
            :returns: The fan-out endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint
        )");

        py_gate.def_property_readonly(
            "unique_predecessors", [](const Gate& g) { return g.get_unique_predecessors(); }, R"(
            A list of all unique predecessor gates of the gate.

            :type: list[hal_py.Gate]
        )");

        py_gate.def("get_unique_predecessors", &Gate::get_unique_predecessors, py::arg("filter") = nullptr, R"(
            Get a list of all unique predecessor gates of the gate. 
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter being evaluated on the gate's input pin as well as the predecessor endpoint.
            :returns: A list of unique predecessors endpoints.
            :rtype: list[hal_py.Gate]
        )");

        py_gate.def_property_readonly(
            "predecessors", [](const Gate& g) { return g.get_predecessors(); }, R"(
            A list of all direct predecessor endpoints of the gate, i.e., all predecessor endpoints that are connected to an input pin of the gate. 

            :type: list[hal_py.Endpoint]
        )");

        py_gate.def("get_predecessors", &Gate::get_predecessors, py::arg("filter") = nullptr, R"(
            Get a list of all direct predecessor endpoints of the gate, i.e., all predecessor endpoints that are connected to an input pin of the gate. 
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter being evaluated on the gate's input pin as well as the predecessor endpoint.
            :returns: A list of predecessors endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def(
            "get_predecessor",
            [](const Gate& self, const GatePin* pin) -> Endpoint* {
                auto res = self.get_predecessor(pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting predecessor endpoint:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("pin"),
            R"(
            Get a single direct predecessor endpoint that is connected to the specified input pin of the gate.
            If there are no or more than one predecessors, None is returned.

            :param hal_py.GatePin pin: The input pin of the the gate.
            :returns: The predecessor endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint or None
        )");

        py_gate.def_property_readonly(
            "unique_successors", [](const Gate& g) { return g.get_unique_successors(); }, R"(
            A list of all unique successor gates of the gate.

            :type: list[hal_py.Gate]
        )");

        py_gate.def("get_unique_successors", &Gate::get_unique_successors, py::arg("filter") = nullptr, R"(
            Get a list of all unique successor gates of the gate.
            A filter can be supplied which filters out all potential values that return false.

            :param lambda filter: An optional filter being evaluated on the gate's output pin as well as the successor endpoint.
            :returns: A list of unique successor gates.
            :rtype: list[hal_py.Gate]
        )");

        py_gate.def_property_readonly(
            "successors", [](const Gate& g) { return g.get_successors(); }, R"(
            A list of all direct successor endpoints of the gate, i.e., all successor endpoints that are connected to an output pin of the gate. 

            :type: list[hal_py.Endpoint]
        )");

        py_gate.def("get_successors", &Gate::get_successors, py::arg("filter") = nullptr, R"(
            Get a list of all direct successor endpoints of the gate, i.e., all successor endpoints that are connected to an output pin of the gate. 
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :returns: A list of successor endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_gate.def(
            "get_successor",
            [](const Gate& self, const GatePin* pin) -> Endpoint* {
                auto res = self.get_successor(pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting successor endpoint:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("pin"),
            R"(
            Get a single direct successor endpoint that is connected to a specific output pin of the gate.
           If there are no or more than one successor, None is returned.

            :param hal_py.GatePin pin: The target output pin of the gate.
            :returns: The successor endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint or None
        )");
    }
}    // namespace hal
