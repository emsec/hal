#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void net_init(py::module& m)
    {
        py::class_<Net, DataContainer, RawPtrWrapper<Net>> py_net(m, "Net", R"(Net class containing information about a net including its source and destination.)");

        py_net.def(py::self == py::self, R"(
            Check whether two nets are equal.

            :returns: True if both nets are equal, false otherwise.
            :rtype: bool
        )");

        py_net.def(py::self != py::self, R"(
            Check whether two nets are unequal.

            :returns: True if both nets are unequal, false otherwise.
            :rtype: bool
        )");

        py_net.def("__hash__", &Net::get_hash, R"(
            Python requires hash for set and dict container.

            :returns: The hash.
            :rtype: Py_hash_t
        )");

        py_net.def_property_readonly("id", &Net::get_id, R"(
            The unique id of the net.

            :type: int
        )");

        py_net.def("get_id", &Net::get_id, R"(
            Get the unique id of the net.

            :returns: The unique id.
            :rtype: int
        )");

        py_net.def_property_readonly(
            "netlist", [](Net* net) { return RawPtrWrapper<Netlist>(net->get_netlist()); }, R"(
            The netlist this net is associated with.

            :type: hal_py.Netlist
        )");

        py_net.def(
            "get_netlist", [](Net* net) { return RawPtrWrapper<Netlist>(net->get_netlist()); }, R"(
            Get the netlist this net is associated with.

            :returns: The netlist.
            :rtype: hal_py.Netlist
        )");

        py_net.def_property("name", &Net::get_name, &Net::set_name, R"(
            The name of the net.

            :type: str
        )");

        py_net.def("get_name", &Net::get_name, R"(
            Get the name of the net.

            :returns: The name.
            :rtype: str
        )");

        py_net.def("set_name", &Net::set_name, py::arg("name"), R"(
            Set the name of the net.

            :param str name: The new name.
        )");

        py_net.def("get_grouping", &Net::get_grouping, R"(
            Get the grouping in which this net is contained.
            If no grouping contains this gate, *None* is returned.

            :returns: The grouping.
            :rtype: hal_py.Grouping
        )");

        py_net.def(
            "add_source",
            [](Net& self, Gate* gate, GatePin* pin) -> Endpoint* {
                auto res = self.add_source(gate, pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while adding source:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("gate"),
            py::arg("pin"),
            R"(
            Add a source endpoint to the net.

            :param hal_py.Gate gate: The source gate.
            :param hal_py.GatePin pin: The output pin of the source gate.
            :returns: The endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint
        )");

        py_net.def(
            "remove_source",
            [](Net& self, Gate* gate, const GatePin* pin) -> bool {
                auto res = self.remove_source(gate, pin);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while removing source:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("gate"),
            py::arg("pin"),
            R"(
            Remove a source endpoint specified by a gate and a pin from the net.

            :param hal_py.Gate gate: The source gate.
            :param hal_py.GatePin pin: The output pin of the source gate.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def(
            "remove_source",
            [](Net& self, Endpoint* ep) -> bool {
                auto res = self.remove_source(ep);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while removing source:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("endpoint"),
            R"(
            Remove a source endpoint from the net.

            :param hal_py.Endpoint endpoint: The source endpoint.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_source", py::overload_cast<Gate*, const GatePin*>(&Net::is_a_source, py::const_), py::arg("gate"), py::arg("pin"), R"(
            Check whether a tuple of a gate and an output pin is a source endpoint of the net.

            :param hal_py.Gate gate: The gate.
            :param hal_py.GatePin pin: The pin of the gate.
            :returns: True if it is a source of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_source", py::overload_cast<Endpoint*>(&Net::is_a_source, py::const_), py::arg("endpoint"), R"(
            Check whether an endpoint is a source of the net.

            :param hal_py.Endpoint Endpoint: The endpoint.
            :returns: True if it is a source of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def_property_readonly("num_of_sources", &Net::get_num_of_sources, R"(
            The number of sources of the net.

            :type: int
        )");

        py_net.def("get_num_of_sources", &Net::get_num_of_sources, R"(
            Get the number of sources of the net.

            :returns: The number of sources.
            :rtype: int
        )");

        py_net.def_property_readonly(
            "sources", [](Net* n) { return n->get_sources(); }, R"(
            The list of sources of the net.

            :type: list[hal_py.Endpoint]
        )");

        py_net.def("get_sources", &Net::get_sources, py::arg("filter") = nullptr, R"(
            Get a list of sources of the net.
            The optional filter is evaluated on every candidate endpoint such that the result only contains endpoints matching the specified condition.

            :param lambda filter: The optional filter function to be evaluated on each endpoint.
            :returns: A list of source endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_net.def(
            "add_destination",
            [](Net& self, Gate* gate, GatePin* pin) -> Endpoint* {
                auto res = self.add_destination(gate, pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while adding destination:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("gate"),
            py::arg("pin"),
            R"(
            Add a destination endpoint to the net.

            :param hal_py.Gate gate: The destination gate.
            :param hal_py.GatePin pin: The input pin of the destination gate.
            :returns: The endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint
        )");

        py_net.def(
            "remove_destination",
            [](Net& self, Gate* gate, const GatePin* pin) -> bool {
                auto res = self.remove_destination(gate, pin);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while removing destination:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("gate"),
            py::arg("pin"),
            R"(
            Remove a destination endpoint specified by a gate and a pin from the net.

            :param hal_py.Gate gate: The destination gate.
            :param hal_py.GatePin pin: The input pin of the destination gate.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def(
            "remove_destination",
            [](Net& self, Endpoint* ep) -> bool {
                auto res = self.remove_destination(ep);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while removing destination:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("endpoint"),
            R"(
            Remove a destination endpoint from the net.

            :param hal_py.Endpoint endpoint: The destination endpoint.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_destination", py::overload_cast<Gate*, const GatePin*>(&Net::is_a_destination, py::const_), py::arg("gate"), py::arg("pin"), R"(
            Check whether a tuple of a gate and an output pin is a destination endpoint of the net.

            :param hal_py.Gate gate: The gate.
            :param hal_py.GatePin pin: The pin of the gate.
            :returns: True if it is a destination of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_destination", py::overload_cast<Endpoint*>(&Net::is_a_destination, py::const_), py::arg("endpoint"), R"(
            Check whether an endpoint is a destination of the net.

            :param hal_py.Endpoint Endpoint: The endpoint.
            :returns: True if it is a destination of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def_property_readonly("num_of_destinations", &Net::get_num_of_destinations, R"(
            The number of destinations of the net.

            :type: int
        )");

        py_net.def("get_num_of_destinations", &Net::get_num_of_destinations, R"(
            Get the number of destinations of the net.

            :returns: The number of destinations.
            :rtype: int
        )");

        py_net.def_property_readonly(
            "destinations", [](Net* n) { return n->get_destinations(); }, R"(
            The list of destinations of the net.

            :type: list[hal_py.Endpoint]
        )");

        py_net.def("get_destinations", &Net::get_destinations, py::arg("filter") = nullptr, R"(
            Get a list of destinations of the net.
            The optional filter is evaluated on every candidate endpoint such that the result only contains endpoints matching the specified condition.

            :param lambda filter: The optional filter function to be evaluated on each endpoint.
            :returns: A list of destination endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_net.def("is_unrouted", &Net::is_unrouted, R"(
            Check whether the net is unrouted, i.e., it has no source or no destination.

            :returns: True if the net is unrouted, false otherwise.
            :rtype: bool
        )");

        py_net.def("is_gnd_net", &Net::is_gnd_net, R"(
            Check whether the net is connected to GND.

            :returns: True if the net is connected to GND, false otherwise.
            :rtype: bool
        )");

        py_net.def("is_vcc_net", &Net::is_vcc_net, R"(
            Check whether the net is connected to VCC.

            :returns: True if the net is connected to VCC, false otherwise.
            :rtype: bool
        )");

        py_net.def("mark_global_input_net", &Net::mark_global_input_net, R"(
            Mark this net as a global input net.

            :returns: True on success.
            :rtype: bool
        )");

        py_net.def("mark_global_output_net", &Net::mark_global_output_net, R"(
            Mark this net as a global output net.

            :returns: True on success.
            :rtype: bool
        )");

        py_net.def("unmark_global_input_net", &Net::unmark_global_input_net, R"(
            Unmark this net as a global input net.

            :returns: True on success.
            :rtype: bool
        )");

        py_net.def("unmark_global_output_net", &Net::unmark_global_output_net, R"(
            Unmark this net as a global output net.

            :returns: True on success.
            :rtype: bool
        )");

        py_net.def("is_global_input_net", &Net::is_global_input_net, R"(
            Checks whether this net is a global input net.

            :returns: True if the net is a global input net.
            :rtype: bool
        )");

        py_net.def("is_global_output_net", &Net::is_global_output_net, R"(
            Checks whether this net is a global output net.

            :returns: True if the net is a global output net.
            :rtype: bool
        )");
    }
}    // namespace hal
