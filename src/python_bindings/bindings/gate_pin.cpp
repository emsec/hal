#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_pin_init(py::module& m)
    {
        py::class_<GatePin, RawPtrWrapper<GatePin>> py_gate_pin(m, "GatePin", R"(
            The pin of a gate type. Each pin has a name, a direction, and a type. 
        )");

        py_gate_pin.def("get_name", &GatePin::get_name, R"(
            Get the name of the pin.

            :returns: The name of the pin.
            :rtype: str
        )");

        py_gate_pin.def("get_direction", &GatePin::get_direction, R"(
            Get the direction of the pin.

            :returns: The direction of the pin.
            :rtype: hal_py.PinDirection
        )");

        py_gate_pin.def("get_type", &GatePin::get_type, R"(
            Get the type of the pin.

            :returns: The type of the pin.
            :rtype: hal_py.PinType
        )");

        py_gate_pin.def("get_group", &GatePin::get_group, R"(
            Get the group of the pin as well as the index of the pin within the group.

            :returns: The group and the index of the pin.
            :rtype: tuple(hal_py.PinGroup,int)
        )");
    }
}    // namespace hal