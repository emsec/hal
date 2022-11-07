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

            :returns: True if both endpoints are equal, False otherwise.
            :rtype: bool
        )");

        py_endpoint.def(py::self != py::self, R"(
            Check whether two endpoints are unequal.

            :returns: True if both endpoints are unequal, False otherwise.
            :rtype: bool
        )");

        py_endpoint.def("get_gate", &Endpoint::get_gate, R"(
            Get the gate associated with the endpoint.

            :returns: The gate.
            :rtype: hal_py.Gate
        )");

        py_endpoint.def_property_readonly("pin", &Endpoint::get_pin, R"(
            The pin associated with the endpoint.

            :type: hal_py.GatePin
        )");

        py_endpoint.def("get_pin", &Endpoint::get_pin, R"(
            Get pin associated with the endpoint.

            :returns: The pin.
            :rtype: hal_py.GatePin
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

        py_endpoint.def_property_readonly("source_pin", &Endpoint::is_source_pin, R"(
            True if the pin of the endpoint is a source (output) pin, False otherwise.

            :type: bool
        )");

        py_endpoint.def("is_source_pin", &Endpoint::is_source_pin, R"(
            Checks whether the pin of the endpoint is a source (output) pin.

            :returns: True if the endpoint is an source (output) pin, False otherwise.
            :rtype: bool
        )");

        py_endpoint.def_property_readonly("destination_pin", &Endpoint::is_destination_pin, R"(
            True if the pin of the endpoint is a destination (input) pin, False otherwise.

            :type: bool
        )");

        py_endpoint.def("is_destination_pin", &Endpoint::is_destination_pin, R"(
            Checks whether the pin of the endpoint is a destination (input) pin.

            :returns: True if the endpoint is an destination (input) pin, False otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
