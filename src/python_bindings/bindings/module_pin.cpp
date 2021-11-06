#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void module_pin_init(py::module& m)
    {
        py::class_<ModulePin, BasePin<ModulePin>, RawPtrWrapper<ModulePin>> py_module_pin(m, "ModulePin", R"(
            The pin of a module. Each pin has a name, a direction, and a type and is associated with a net. 
        )");

        py_module_pin.def("get_net", &ModulePin::get_net, R"(
            Get the net passing through the pin.

            :returns: The net of the pin.
            :rtype: hal_py.Net
        )");
    }
}    // namespace hal