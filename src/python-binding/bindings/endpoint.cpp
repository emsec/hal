#include "bindings.h"

void endpoint_init(py::module& m)
{
    py::class_<endpoint, std::shared_ptr<endpoint>> py_endpoint(m, "endpoint");

    py_endpoint.def(py::init<const std::shared_ptr<gate>&, const std::string&, bool>(), py::arg("gate"), py::arg("pin"), py::arg("is_a_destination"), R"(
        Construct a new endpoint.
)");

    py_endpoint.def(py::self < py::self, R"(
        Standard "less than". Required for searching through sets.

        :returns: True if endpoint is less than compare target.
        :rtype: bool
)");

    py_endpoint.def(py::self == py::self, R"(
        Standard "equals". Required for searching through sets.

        :returns: True if endpoint is equal to compare target.
        :rtype: bool
)");

    py_endpoint.def(py::self != py::self, R"(
        Standard "unequal".

        :returns: True if endpoint is unequal to compare target.
        :rtype: bool
)");

    py_endpoint.def_property_readonly("gate", &endpoint::get_gate, R"(
        The gate of the endpoint.

        :type: hal_py.gate
)");

    py_endpoint.def("get_gate", &endpoint::get_gate, R"(
        Returns the gate of the endpoint.

        :returns: The gate.
        :rtype: hal_py.gate
)");

    py_endpoint.def_property_readonly("pin", &endpoint::get_pin, R"(
        The pin of the endpoint.

        :type: str
)");

    py_endpoint.def("get_pin", &endpoint::get_pin, R"(
        Returns the pin of the endpoint.

        :returns: The pin.
        :rtype: str
)");

    py_endpoint.def_property_readonly("is_source", &endpoint::is_source_pin, R"(
        True if the pin of the endpoint is a source pin, false otherwise.

        :type: bool
)");

    py_endpoint.def("is_source_pin", &endpoint::is_source_pin, R"(
        Checks whether the pin of the endpoint is a source pin.

        :returns: The pin type.
        :rtype: bool
)");

    py_endpoint.def_property_readonly("is_destination", &endpoint::is_destination_pin, R"(
        True if the pin of the endpoint is a destination pin, false otherwise.

        :type: bool
)");

    py_endpoint.def("is_destination_pin", &endpoint::is_destination_pin, R"(
        Checks whether the pin of the endpoint is a destination pin.

        :returns: The pin type.
        :rtype: bool
)");
}