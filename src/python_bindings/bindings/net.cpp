#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void net_init(py::module& m)
    {
        py::class_<Net, DataContainer, RawPtrWrapper<Net>> py_net(m, "Net", R"(Net class containing information about a net including its source and destination.)");

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

        py_net.def("add_source", &Net::add_source, py::arg("gate"), py::arg("pin"), R"(
        Add a source-endpoint to this net.

        :param hal_py.Gate gate: The source gate.
        :param str pin: The output pin of the source gate.
        :returns: The new endpoint or *None* on error.
        :rtype: hal_py.Endpoint
)");

        py_net.def("remove_source", py::overload_cast<Gate*, const std::string&>(&Net::remove_source), py::arg("gate"), py::arg("pin"), R"(
        Remove a source-endpoint from this net.

        :param hal_py.Gate gate: The source gate.
        :param str pin: The output pin of the source gate.
        :returns: True on success.
        :rtype: bool
)");

        py_net.def("remove_source", py::overload_cast<Endpoint*>(&Net::remove_source), py::arg("endpoint"), R"(
        Remove a source-endpoint from this net.

        :param endpoint: The source endpoint.
        :type endpoint: hal_py.Endpoint
        :returns: True on success
        :rtype: bool
)");

        py_net.def("is_a_source", py::overload_cast<Gate*, const std::string&>(&Net::is_a_source, py::const_), py::arg("gate"), py::arg("pin"), R"(
        Check whether an endpoint, given by its gate and pin type, is a source-endpoint of this net.

        :param hal_py.Gate gate: The source gate.
        :param str pin: The source output pin.
        :returns: True if the gate's pin is a source of this net.
        :rtype: bool
)");

        py_net.def("is_a_source", py::overload_cast<Endpoint*>(&Net::is_a_source, py::const_), py::arg("endpoint"), R"(
        Check whether an endpoint is a source-endpoint of this net.

        :param hal_py.Endpoint Endpoint: The endpoint.
        :returns: True if the endpoint is a source of this net.
        :rtype: bool
)");

        py_net.def_property_readonly("num_of_sources", &Net::get_num_of_sources, R"(
        The number of sources of the net.

        :type: int
)");

        py_net.def("get_num_of_sources", &Net::get_num_of_sources, R"(
        Get the number of sources.
        Faster than len(:func:`get_sources()`).

        :returns: The number of sources of this net.
        :rtype: int
)");

        py_net.def_property_readonly(
            "sources", [](Net* n) { return n->get_sources(); }, R"(
        The list of sources of the net.

        :type: list[hal_py.Endpoint]
)");

        py_net.def("get_sources", &Net::get_sources, py::arg("filter") = nullptr, R"(
        Get a list of sources of the net.
        A filter can be supplied which filters out all potential values that return false.

        :param lambda filter: A filter for endpoints. Leave empty for no filtering.
        :returns: A list of source-endpoints.
        :rtype: list[hal_py.Endpoint]
)");

        py_net.def("get_source", &Net::get_source, R"(
        .. deprecated:: 2.0
            Use :func:`get_sources` instead.

        Get the (first) source-endpoint of the net.
        If there was no source assigned, the gate element of the returned endpoint is *None*.
        If the net is multi-driven a warning is printed.

        :returns: The (first) source endpoint.
        :rtype: hal_py.Endpoint

)");

        py_net.def("add_destination", &Net::add_destination, py::arg("gate"), py::arg("pin"), R"(
        Add a destination-endpoint to this net.

        :param hal_py.Gate gate: The destination gate.
        :param str pin: The input pin of the destination gate.
        :returns: The new endpoint or *None* on error.
        :rtype: hal_py.Endpoint
)");

        py_net.def("remove_destination", py::overload_cast<Gate*, const std::string&>(&Net::remove_destination), py::arg("gate"), py::arg("pin"), R"(
        Remove a destination from this net.

        :param hal_py.Gate gate: The destination gate.
        :param str pin: The input pin of the destination gate.
        :returns: True on success
        :rtype: bool
)");

        py_net.def("remove_destination", py::overload_cast<Endpoint*>(&Net::remove_destination), py::arg("endpoint"), R"(
        Remove a destination-endpoint from this net.
        If the endpoint is no destination-endpoint of the net this function aborts.

        :param hal_py.Endpoint endpoint: The destination endpoint.
        :returns: True on success
        :rtype: bool
)");

        py_net.def("is_a_destination", py::overload_cast<Gate*, const std::string&>(&Net::is_a_destination, py::const_), py::arg("gate"), py::arg("pin"), R"(
        Check whether an endpoint, given by its gate and pin type, is a destination-endpoint of this net.

        :param hal_py.Gate gate: The destination gate.
        :param str pin: The destination input pin.
        :returns: True if the gate's pin is a destination of this net.
        :rtype: bool
)");

        py_net.def("is_a_destination", py::overload_cast<Endpoint*>(&Net::is_a_destination, py::const_), py::arg("endpoint"), R"(
        Check whether an endpoint is a destination-endpoint of this net.

        :param hal_py.Endpoint endpoint: The endpoint.
        :returns: True if the endpoint is a destination of this net.
        :rtype: bool
)");

        py_net.def_property_readonly("num_of_destinations", &Net::get_num_of_destinations, R"(
        The number of destinations of the net.

        :type: int
)");

        py_net.def("get_num_of_destinations", &Net::get_num_of_destinations, R"(
        Get the number of destinations.
        Faster than len(:func:`get_destinations()`).

        :returns: The number of destinations of this net.
        :rtype: int
)");

        py_net.def_property_readonly(
            "destinations", [](Net* n) { return n->get_destinations(); }, R"(
        The list of destinations of the net.

        :type: list[hal_py.Endpoint]
)");

        py_net.def("get_destinations", &Net::get_destinations, py::arg("filter") = nullptr, R"(
        Get the list of destinations of the net.

        :param filter: A filter for endpoints. Leave empty for no filtering.
        :returns: A list of destination-endpoints.
        :rtype: list[hal_py.Endpoint]
)");

        py_net.def("is_unrouted", &Net::is_unrouted, R"(
        Check whether the net is unrouted, i.e. it has no source or no destination.

        :returns: True if the net is unrouted.
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
