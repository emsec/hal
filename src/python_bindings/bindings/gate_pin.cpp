#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_pin_init(py::module& m)
    {
        py::class_<GatePin, BasePin<GatePin>, RawPtrWrapper<GatePin>> py_gate_pin(m, "GatePin", R"(
            The pin of a gate type. Each pin has a name, a direction, and a type. 
        )");
    }
}    // namespace hal