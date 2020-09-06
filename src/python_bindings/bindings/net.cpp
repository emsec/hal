#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void net_init(py::module& m)
    {
        py::class_<Net, RawPtrWrapper<Net>> py_net(m, "Net", R"(Net class containing information about a net including its source and destination.)");

        py_net.def_property_readonly("id", &Net::get_id, R"(
        The unique id of the net.

        :type: int
)");

        py_net.def("get_id", &Net::get_id, R"(
        Get the unique id of the Net.

        :returns: The net's id.
        :rtype: int
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

        py_net.def("add_source", &Net::add_source, py::arg("gate"), py::arg("pin"), R"(
        Add a source to this net.

        :param hal_py.Gate gate: The source gate.
        :param str pin: The input pin of the gate.
        :returns: True on succes.
        :rtype: bool
)");

        py_net.def("remove_source", py::overload_cast<Gate*, const std::string&>(&Net::remove_source), py::arg("gate"), py::arg("pin"), R"(
        Removes the source of the net.
        :param hal_py.get_gate() gate: The source gate.
        :param str pin: The pin of the source gate.
        :returns: True on succes.
        :rtype: bool
)");

        py_net.def("remove_source", py::overload_cast<Endpoint*>(&Net::remove_source), py::arg("endpoint"), R"(
        Remove a source endpoint from this net.

        :param endpoint: The source endpoint.
        :type endpoint: hal_py.Endpoint
        :returns: True on succes
        :rtype: bool
)");

        py_net.def("is_a_source", py::overload_cast<Gate*, const std::string&>(&Net::is_a_source, py::const_), py::arg("gate"), py::arg("pin"), R"(
        Check whether a gate's input pin is a source of this net.

        :param gate: The source gate.
        :type gate: hal_py.get_gate()
        :param str pin: The input pin of the gate. Leave empty if the pin does not matter.
        :returns: True if the input's pin is a source.
        :rtype: bool
)");

        py_net.def("is_a_source", py::overload_cast<Endpoint*>(&Net::is_a_source, py::const_), py::arg("endpoint"), R"(
        Check whether a gate's input pin is a source of this net.

        :param Endpoint: The input endpoint.
        :type Endpoint: hal_py.Endpoint
        :returns: True if the input's pin is a source.
        :rtype: bool
)");

        py_net.def_property_readonly("num_of_sources", &Net::get_num_of_sources, R"(
        The number of sources of the net.

        :type: int
)");

        py_net.def("get_num_of_sources", &Net::get_num_of_sources, R"(
        Get the number of sources.

        :returns: The number of sources of this net.
        :rtype: int
)");

        py_net.def_property_readonly(
            "sources", [](Net* n) { return n->get_sources(); }, R"(
        Get the vector of sources of the net.

        :type: set[hal_py.Net]
)");

        py_net.def("get_sources", &Net::get_sources, py::arg("filter") = nullptr, R"(
        Get the vector of sources of the net.

        :param filter: a filter for endpoints.
        :returns: A list of source endpoints.
        :rtype: list[hal_py.Endpoint]
)");

        py_net.def("get_source", &Net::get_source, R"(
        Get the (first) source of the net specified by type.
        If there is no source, the gate of the returned endpoint is null.

        :param str gate_type: The desired source gate type.
        :returns: The source endpoint.
        :rtype: hal_py.Endpoint
)");

        py_net.def("add_destination", &Net::add_destination, py::arg("gate"), py::arg("pin"), R"(
        Add a destination to this net.

        :param gate: The destination gate.
        :type gate: hal_py.Gate
        :param str pin: The input pin of the gate.
        :returns: True on succes
        :rtype: bool
)");

        py_net.def("remove_destination", py::overload_cast<Gate*, const std::string&>(&Net::remove_destination), py::arg("gate"), py::arg("pin"), R"(
        Remove a destination from this net.

        :param gate: The destination gate.
        :type gate: hal_py.Gate
        :param str pin: The input pin of the gate. Leave empty if the pin does not matter.
        :returns: True on succes
        :rtype: bool
)");

        py_net.def("remove_destination", py::overload_cast<Endpoint*>(&Net::remove_destination), py::arg("endpoint"), R"(
        Remove a destination endpoint from this net.

        :param endpoint: The destination endpoint.
        :type endpoint: hal_py.Endpoint
        :returns: True on succes
        :rtype: bool
)");

        py_net.def("is_a_destination", py::overload_cast<Gate*, const std::string&>(&Net::is_a_destination, py::const_), py::arg("gate"), py::arg("pin"), R"(
        Check whether a gate's input pin is a destination of this net.

        :param gate: The destination gate.
        :type gate: hal_py.get_gate()
        :param str pin: The input pin of the gate. Leave empty if the pin does not matter.
        :returns: True if the input's pin is a destination.
        :rtype: bool
)");

        py_net.def("is_a_destination", py::overload_cast<Endpoint*>(&Net::is_a_destination, py::const_), py::arg("endpoint"), R"(
        Check whether a gate's input pin is a destination of this net.

        :param Endpoint: The input endpoint.
        :type Endpoint: hal_py.Endpoint
        :returns: True if the input's pin is a destination.
        :rtype: bool
)");

        py_net.def_property_readonly("num_of_destinations", &Net::get_num_of_destinations, R"(
        The number of destinations of the net.

        :type: int
)");

        py_net.def("get_num_of_destinations", &Net::get_num_of_destinations, R"(
        Get the number of destinations.

        :returns: The number of destinations of this net.
        :rtype: int
)");

        py_net.def_property_readonly(
            "destinations", [](Net* n) { return n->get_destinations(); }, R"(
        Get the vector of destinations of the net.

        :type: set[hal_py.Net]
)");

        py_net.def("get_destinations", &Net::get_destinations, py::arg("filter") = nullptr, R"(
        Get the vector of destinations of the net.

        :param filter: a filter for endpoints.
        :returns: A list of destination endpoints.
        :rtype: list[hal_py.Endpoint]
)");

        py_net.def("is_unrouted", &Net::is_unrouted, R"(
        Check whether the net is routed, i.e. it has no source or no destinations.

        :returns: True if the Net is unrouted.
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
