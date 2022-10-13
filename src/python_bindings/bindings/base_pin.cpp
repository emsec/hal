#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void base_pin_init(py::module& m)
    {
        py::class_<BasePin<GatePin>, RawPtrWrapper<BasePin<GatePin>>> py_gate_base_pin(m, "GateBasePin", R"(
            Base class for gate pins.
        )");

        py_gate_base_pin.def(py::self == py::self, R"(
            Check whether two pins are equal.

            :returns: True if both pins are equal, False otherwise.
            :rtype: bool
        )");

        py_gate_base_pin.def(py::self != py::self, R"(
            Check whether two pins are unequal.

            :returns: True if both pins are unequal, False otherwise.
            :rtype: bool
        )");

        py_gate_base_pin.def_property_readonly("id", &BasePin<GatePin>::get_id, R"(
            The ID of the gate pin. The ID is unique within a gate type.

            :type: int
        )");

        py_gate_base_pin.def("get_id", &BasePin<GatePin>::get_id, R"(
            Get the ID of the gate pin. The ID is unique within a gate type.

            :returns: The ID of the pin.
            :rtype: int
        )");

        py_gate_base_pin.def_property_readonly("name", &BasePin<GatePin>::get_name, R"(
            The name of the pin.

            :type: str
        )");

        py_gate_base_pin.def("get_name", &BasePin<GatePin>::get_name, R"(
            Get the name of the pin.

            :returns: The name of the pin.
            :rtype: str
        )");

        py_gate_base_pin.def_property_readonly("direction", &BasePin<GatePin>::get_direction, R"(
            The direction of the pin.

            :type: hal_py.PinDirection
        )");

        py_gate_base_pin.def("get_direction", &BasePin<GatePin>::get_direction, R"(
            Get the direction of the pin.

            :returns: The direction of the pin.
            :rtype: hal_py.PinDirection
        )");

        py_gate_base_pin.def_property_readonly("type", &BasePin<GatePin>::get_type, R"(
            The type of the pin.

            :type: hal_py.PinType
        )");

        py_gate_base_pin.def("get_type", &BasePin<GatePin>::get_type, R"(
            Get the type of the pin.

            :returns: The type of the pin.
            :rtype: hal_py.PinType
        )");

        py_gate_base_pin.def_property_readonly("group", &BasePin<GatePin>::get_group, R"(
            A tuple comprising the group of the pin as well as the index of the pin within the group.

            :type: tuple(hal_py.GatePinGroup,int)
        )");

        py_gate_base_pin.def("get_group", &BasePin<GatePin>::get_group, R"(
            Get the group of the pin as well as the index of the pin within the group.

            :returns: The group and the index of the pin.
            :rtype: tuple(hal_py.GatePinGroup,int)
        )");

        py::class_<BasePin<ModulePin>, RawPtrWrapper<BasePin<ModulePin>>> py_module_base_pin(m, "ModuleBasePin", R"(
            Base class for module pins.
        )");

        py_module_base_pin.def(py::self == py::self, R"(
            Check whether two pins are equal.

            :returns: True if both pins are equal, False otherwise.
            :rtype: bool
        )");

        py_module_base_pin.def(py::self != py::self, R"(
            Check whether two pins are unequal.

            :returns: True if both pins are unequal, False otherwise.
            :rtype: bool
        )");

        py_module_base_pin.def_property_readonly("id", &BasePin<ModulePin>::get_id, R"(
            The ID of the module pin. The ID is unique within a module.

            :type: int
        )");

        py_module_base_pin.def("get_id", &BasePin<ModulePin>::get_id, R"(
            Get the ID of the module pin. The ID is unique within a module.

            :returns: The ID of the pin.
            :rtype: int
        )");

        py_module_base_pin.def_property_readonly("name", &BasePin<ModulePin>::get_name, R"(
            The name of the pin.

            :type: str
        )");

        py_module_base_pin.def("get_name", &BasePin<ModulePin>::get_name, R"(
            Get the name of the pin.

            :returns: The name of the pin.
            :rtype: str
        )");

        py_module_base_pin.def_property_readonly("direction", &BasePin<ModulePin>::get_direction, R"(
            The direction of the pin.

            :type: hal_py.PinDirection
        )");

        py_module_base_pin.def("get_direction", &BasePin<ModulePin>::get_direction, R"(
            Get the direction of the pin.

            :returns: The direction of the pin.
            :rtype: hal_py.PinDirection
        )");

        py_module_base_pin.def_property_readonly("type", &BasePin<ModulePin>::get_type, R"(
            The type of the pin.

            :type: hal_py.PinType
        )");

        py_module_base_pin.def("get_type", &BasePin<ModulePin>::get_type, R"(
            Get the type of the pin.

            :returns: The type of the pin.
            :rtype: hal_py.PinType
        )");

        py_module_base_pin.def_property_readonly("group", &BasePin<ModulePin>::get_group, R"(
            A tuple comprising the group of the pin as well as the index of the pin within the group.

            :type: tuple(hal_py.ModulePinGroup,int)
        )");

        py_module_base_pin.def("get_group", &BasePin<ModulePin>::get_group, R"(
            Get the group of the pin as well as the index of the pin within the group.

            :returns: The group and the index of the pin.
            :rtype: tuple(hal_py.ModulePinGroup,int)
        )");
    }
}    // namespace hal