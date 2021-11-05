#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void pin_group_init(py::module& m)
    {
        py::class_<PinGroup<GatePin>, RawPtrWrapper<PinGroup<GatePin>>> py_gate_pin_group(m, "GatePinGroup", R"(
            A group of gate pins made up of a name, the pins, a pin order, and a start index.
        )");

        py::class_<PinGroup<ModulePin>, RawPtrWrapper<PinGroup<ModulePin>>> py_module_pin_group(m, "ModulePinGroup", R"(
            A group of module pins made up of a name, the pins, a pin order, and a start index.
        )");
    }
}    // namespace hal