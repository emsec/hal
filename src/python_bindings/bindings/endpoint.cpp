#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void endpoint_init(py::module& m)
    {
        py::class_<Endpoint, RawPtrWrapper<Endpoint>> py_endpoint(m, "Endpoint", R"(
            An endpoint comprises the pin of a gate, the respective gate, and the connected net.
        )");

        py_endpoint.def_property_readonly("gate", &Endpoint::get_gate, R"(
            The gate associated with the endpoint.

            :type: hal_py.Gate
        )");

        py_endpoint.def(py::self == py::self, R"(
            Check whether two endpoints are equal.

            :returns: True if both endpoints are equal, false otherwise.
            :rtype: bool
        )");

        py_endpoint.def(py::self != py::self, R"(
            Check whether two endpoints are unequal.

            :returns: True if both endpoints are unequal, false otherwise.
            :rtype: bool
        )");

        py_endpoint.def("get_gate", &Endpoint::get_gate, R"(
            Get the gate associated with the endpoint.

            :returns: The gate.
            :rtype: hal_py.Gate
        )");

        py_endpoint.def_property_readonly("pin", &Endpoint::get_pin, R"(
            The name of the pin associated with the endpoint.

            :type: str
        )");

        py_endpoint.def("get_pin", &Endpoint::get_pin, R"(
            Get the name of the pin associated with the endpoint.

            :returns: The name of the pin.
            :rtype: str
        )");

        py_endpoint.def_property_readonly("net", &Endpoint::get_net, R"(
            The net associated with the endpoint.

            :type: hal_py.Net
        )");

        py_endpoint.def("get_net", &Endpoint::get_net, R"(
            Get the net associated with the endpoint.

            :returns: The net.
            :rtype: hal_py.Net
        )");

        py_endpoint.def_property_readonly("is_source", &Endpoint::is_source_pin, R"(
            True if the pin of the endpoint is a source (output) pin, false otherwise.

            :type: bool
        )");

        py_endpoint.def("is_source_pin", &Endpoint::is_source_pin, R"(
            Checks whether the pin of the endpoint is a source (output) pin.

            :returns: True if the endpoint is an source (output) pin, false otherwise.
            :rtype: bool
        )");

        py_endpoint.def_property_readonly("is_destination", &Endpoint::is_destination_pin, R"(
            True if the pin of the endpoint is a destination (input) pin, false otherwise.

            :type: bool
        )");

        py_endpoint.def("is_destination_pin", &Endpoint::is_destination_pin, R"(
            Checks whether the pin of the endpoint is a destination (input) pin.

            :returns: True if the endpoint is an destination (input) pin, false otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
