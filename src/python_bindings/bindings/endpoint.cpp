#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void endpoint_init(py::module& m)
    {
        py::class_<Endpoint, std::shared_ptr<Endpoint>> py_endpoint(m, "Endpoint");

        py_endpoint.def_property_readonly("gate", &Endpoint::get_gate, R"(
        The gate of the endpoint.

        :type: hal_py.Gate
)");

        py_endpoint.def("get_gate", &Endpoint::get_gate, R"(
        Returns the gate of the endpoint.

        :returns: The gate.
        :rtype: hal_py.Gate
)");

        py_endpoint.def_property_readonly("pin", &Endpoint::get_pin, R"(
        The pin of the endpoint.

        :type: str
)");

        py_endpoint.def("get_pin", &Endpoint::get_pin, R"(
        Returns the pin of the endpoint.

        :returns: The pin.
        :rtype: str
)");

        py_endpoint.def_property_readonly("net", &Endpoint::get_net, R"(
        The net of the endpoint.

        :type: hal_py.Net
)");

        py_endpoint.def("get_net", &Endpoint::get_net, R"(
        Returns the net of the endpoint.

        :returns: The net.
        :rtype: hal_py.Net
)");

        py_endpoint.def_property_readonly("is_source", &Endpoint::is_source_pin, R"(
        True if the pin of the endpoint is a source pin, false otherwise.

        :type: bool
)");

        py_endpoint.def("is_source_pin", &Endpoint::is_source_pin, R"(
        Checks whether the pin of the endpoint is a source pin.

        :returns: The pin type.
        :rtype: bool
)");

        py_endpoint.def_property_readonly("is_destination", &Endpoint::is_destination_pin, R"(
        True if the pin of the endpoint is a destination pin, false otherwise.

        :type: bool
)");

        py_endpoint.def("is_destination_pin", &Endpoint::is_destination_pin, R"(
        Checks whether the pin of the endpoint is a destination pin.

        :returns: The pin type.
        :rtype: bool
)");
    }
}    // namespace hal
