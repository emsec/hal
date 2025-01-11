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

        py_net.def_property_readonly("netlist", [](Net* net) { return RawPtrWrapper<Netlist>(net->get_netlist()); }, R"(
            The netlist this net is associated with.

            :type: hal_py.Netlist
        )");

        py_net.def("get_netlist", [](Net* net) { return RawPtrWrapper<Netlist>(net->get_netlist()); }, R"(
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

        py_net.def("add_source", py::overload_cast<Gate*, const std::string&>(&Net::add_source), py::arg("gate"), py::arg("pin_name"), R"(
            Add a source endpoint to the net.
            The endpoint is specified by a tuple of a gate and the name of an output pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param str pin_name: The name of an output pin of the gate.
            :returns: The endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint or None
        )");

        py_net.def("add_source", py::overload_cast<Gate*, GatePin*>(&Net::add_source), py::arg("gate"), py::arg("pin"), R"(
            Add a source endpoint to the net.
            The endpoint is specified by a tuple of a gate and an output pin of that gate. 

            :param hal_py.Gate gate: The gate.
            :param hal_py.GatePin pin: The output pin of the gate.
            :returns: The endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint or None
        )");

        py_net.def("remove_source", py::overload_cast<Gate*, const std::string&>(&Net::remove_source), py::arg("gate"), py::arg("pin_name"), R"(
            Remove a source endpoint from the net.
            The endpoint is specified by a tuple of a gate and the name of an output pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param str pin_name: The name of an output pin of the gate.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("remove_source", py::overload_cast<Gate*, const GatePin*>(&Net::remove_source), py::arg("gate"), py::arg("pin"), R"(
            Remove a source endpoint from the net.
            The endpoint is specified by a tuple of a gate and an output pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param hal_py.GatePin pin: The output pin of the gate.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("remove_source", py::overload_cast<Endpoint*>(&Net::remove_source), py::arg("ep"), R"(
            Remove a source endpoint from the net.

            :param hal_py.Endpoint ep: The endpoint.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_source", py::overload_cast<const Gate*>(&Net::is_a_source, py::const_), py::arg("gate"), R"(
            Check whether a gate is a source of the net independent of the pin.

            :param hal_py.Gate gate: The gate.
            :returns: True if the gate is a source of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_source", py::overload_cast<const Gate*, const std::string&>(&Net::is_a_source, py::const_), py::arg("gate"), py::arg("pin_name"), R"(
            Check whether an endpoint is a source of the net.
            The endpoint is specified by a tuple of a gate and the name of an output pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param str pin_name: The name of an output pin of the gate.
            :returns: True if the endpoint is a source of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_source", py::overload_cast<const Gate*, const GatePin*>(&Net::is_a_source, py::const_), py::arg("gate"), py::arg("pin"), R"(
            Check whether an endpoint is a source of the net.
            The endpoint is specified by a tuple of a gate and the name of an output pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param hal_py.GatePin pin: The output pin of the gate.
            :returns: True if the endpoint is a source of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_source", py::overload_cast<const Endpoint*>(&Net::is_a_source, py::const_), py::arg("ep"), R"(
            Check whether an endpoint is a source of the net.

            :param hal_py.Endpoint ep: The endpoint.
            :returns: True if the endpoint is a source of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def_property_readonly("num_of_sources", [](Net* n) { return n->get_num_of_sources(); }, R"(
            The number of sources of the net.

            :type: int
        )");

        py_net.def("get_num_of_sources", &Net::get_num_of_sources, py::arg("filter") = nullptr, R"(
            Get the number of sources of the net.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :returns: The number of sources.
            :rtype: int
        )");

        py_net.def_property_readonly("sources", [](Net* n) { return n->get_sources(); }, R"(
            A list of sources of the net.

            :type: list[hal_py.Endpoint]
        )");

        py_net.def("get_sources", &Net::get_sources, py::arg("filter") = nullptr, R"(
            Get a list of sources of the net.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param lambda filter: An optional filter.
            :returns: A list of source endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_net.def("add_destination", py::overload_cast<Gate*, const std::string&>(&Net::add_destination), py::arg("gate"), py::arg("pin_name"), R"(
            Add a destination endpoint to the net.
            The endpoint is specified by a tuple of a gate and the name of an input pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param str pin_name: The name of an input pin of the gate.
            :returns: The endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint or None
        )");

        py_net.def("add_destination", py::overload_cast<Gate*, GatePin*>(&Net::add_destination), py::arg("gate"), py::arg("pin"), R"(
            Add a destination endpoint to the net.
            The endpoint is specified by a tuple of a gate and an input pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param str pin_name: The input pin of the gate.
            :returns: The endpoint on success, None otherwise.
            :rtype: hal_py.Endpoint or None
        )");

        py_net.def("remove_destination", py::overload_cast<Gate*, const std::string&>(&Net::remove_destination), py::arg("gate"), py::arg("pin_name"), R"(
            Remove a destination endpoint from the net.
            The endpoint is specified by a tuple of a gate and the name of an input pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param str pin_name: The name of an input pin of the gate.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("remove_destination", py::overload_cast<Gate*, const GatePin*>(&Net::remove_destination), py::arg("gate"), py::arg("pin"), R"(
            Remove a destination endpoint from the net.
            The endpoint is specified by a tuple of a gate and the name of an input pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param hal_py.GatePin pin: The input pin of the gate.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("remove_destination", py::overload_cast<Endpoint*>(&Net::remove_destination), py::arg("ep"), R"(
            Remove a destination endpoint from the net.

            :param hal_py.Endpoint ep: The destination endpoint.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_destination", py::overload_cast<const Gate*>(&Net::is_a_destination, py::const_), py::arg("gate"), R"(
            Check whether a gate is a destination of the net independent of the pin.

            :param hal_py.Gate gate: The gate.
            :returns: True if the gate is a destination of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_destination", py::overload_cast<const Gate*, const std::string&>(&Net::is_a_destination, py::const_), py::arg("gate"), py::arg("pin_name"), R"(
            Check whether an endpoint is a destination of the net.
            The endpoint is specified by a tuple of a gate and the name of an input pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param str pin_name: The name of an input pin of the gate.
            :returns: True if the endpoint is a destination of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_destination", py::overload_cast<const Gate*, const GatePin*>(&Net::is_a_destination, py::const_), py::arg("gate"), py::arg("pin"), R"(
            Check whether an endpoint is a destination of the net.
            The endpoint is specified by a tuple of a gate and an input pin of that gate.

            :param hal_py.Gate gate: The gate.
            :param hal_py.GatePin pin: The input pin of the gate.
            :returns: True if the endpoint is a destination of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_a_destination", py::overload_cast<const Endpoint*>(&Net::is_a_destination, py::const_), py::arg("ep"), R"(
            Check whether an endpoint is a destination of the net.

            :param hal_py.Endpoint ep: The endpoint.
            :returns: True if the endpoint is a destination of the net, False otherwise.
            :rtype: bool
        )");

        py_net.def_property_readonly("num_of_destinations", [](Net* n) { return n->get_num_of_destinations(); }, R"(
            The number of destinations of the net.

            :type: int
        )");

        py_net.def("get_num_of_destinations", &Net::get_num_of_destinations, py::arg("filter") = nullptr, R"(
            Get the number of destinations of the net.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param filter: An optional filter.
            :returns: The number of destinations.
            :rtype: int
        )");

        py_net.def_property_readonly("destinations", [](Net* n) { return n->get_destinations(); }, R"(
            A list of destinations of the net.

            :type: list[hal_py.Endpoint]
        )");

        py_net.def("get_destinations", &Net::get_destinations, py::arg("filter") = nullptr, R"(
            Get a vector of destinations of the net.
            The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.

            :param filter: An optional filter.
            :returns: A list of destination endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        py_net.def("is_unrouted", &Net::is_unrouted, R"(
            Check whether the net is unrouted, i.e., it has no source or no destination.

            :returns: True if the net is unrouted, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_gnd_net", &Net::is_gnd_net, R"(
            Check whether the net is connected to GND.

            :returns: True if the net is connected to GND, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_vcc_net", &Net::is_vcc_net, R"(
            Check whether the net is connected to VCC.

            :returns: True if the net is connected to VCC, False otherwise.
            :rtype: bool
        )");

        py_net.def("mark_global_input_net", &Net::mark_global_input_net, R"(
            Mark this net as a global input net.

            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("mark_global_output_net", &Net::mark_global_output_net, R"(
            Mark this net as a global output net.

            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("unmark_global_input_net", &Net::unmark_global_input_net, R"(
            Unmark this net as a global input net.

            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("unmark_global_output_net", &Net::unmark_global_output_net, R"(
            Unmark this net as a global output net.

            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_global_input_net", &Net::is_global_input_net, R"(
            Checks whether this net is a global input net.

            :returns: True if the net is a global input net, False otherwise.
            :rtype: bool
        )");

        py_net.def("is_global_output_net", &Net::is_global_output_net, R"(
            Checks whether this net is a global output net.

            :returns: True if the net is a global output net, False otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
