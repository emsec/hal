#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void pin_group_init(py::module& m)
    {
        py::class_<PinGroup<GatePin>, RawPtrWrapper<PinGroup<GatePin>>> py_gate_pin_group(m, "GatePinGroup", R"(
            A group of gate pins made up of a name, the pins, a pin order, and a start index.
        )");

        // TODO implement missing gate pin group bindings

        py::class_<PinGroup<ModulePin>, RawPtrWrapper<PinGroup<ModulePin>>> py_module_pin_group(m, "ModulePinGroup", R"(
            A group of module pins made up of a name, the pins, a pin order, and a start index.
        )");

        py_module_pin_group.def(py::self == py::self, R"(
            Check whether two module pin groups are equal.

            :returns: True if both module pin groups are equal, False otherwise.
            :rtype: bool
        )");

        py_module_pin_group.def(py::self != py::self, R"(
            Check whether two module pin groups are unequal.

            :returns: True if both module pin groups are unequal, False otherwise.
            :rtype: bool
        )");

        py_module_pin_group.def_property_readonly("id", &PinGroup<ModulePin>::get_id, R"(
            The ID of the module pin group. The ID is unique within a module.

            :type: int
        )");

        py_module_pin_group.def("get_id", &PinGroup<ModulePin>::get_id, R"(
            Get the ID of the module pin group. The ID is unique within a module.

            :returns: The ID of the pin.
            :rtype: int
        )");

        py_module_pin_group.def_property_readonly("name", &PinGroup<ModulePin>::get_name, R"(
            The name of the pin group.

            :type: str
        )");

        py_module_pin_group.def("get_name", &PinGroup<ModulePin>::get_name, R"(
            Get the name of the pin group.

            :returns: The name of the pin group.
            :rtype: str
        )");

        py_module_pin_group.def_property_readonly("direction", &PinGroup<ModulePin>::get_direction, R"(
            The direction of the pin group.

            :type: hal_py.PinDirection
        )");

        py_module_pin_group.def("get_direction", &PinGroup<ModulePin>::get_direction, R"(
            Get the direction of the pin group.

            :returns: The direction of the pin.
            :rtype: hal_py.PinDirection
        )");

        py_module_pin_group.def_property_readonly("type", &PinGroup<ModulePin>::get_type, R"(
            The type of the pin group.

            :type: hal_py.PinType
        )");

        py_module_pin_group.def("get_type", &PinGroup<ModulePin>::get_type, R"(
            Get the type of the pin group.

            :returns: The type of the pin.
            :rtype: hal_py.PinType
        )");

        py_module_pin_group.def_property_readonly(
            "pins", [](const PinGroup<ModulePin>& self) -> std::vector<ModulePin*> { return self.get_pins(nullptr); }, R"(
            The (ordered) pins of the pin groups.

            :type: list[hal_py.ModulePin]
        )");

        py_module_pin_group.def("get_pins", &PinGroup<ModulePin>::get_pins, py::arg("filter") = nullptr, R"(
            Get the (ordered) pins of the pin groups.
            The optional filter is evaluated on every pin such that the result only contains pins matching the specified condition.

            :param lambda filter: Filter function to be evaluated on each pin.
            :returns: The ordered pins.
            :rtype: list[hal_py.ModulePin]
        )");

        py_module_pin_group.def(
            "get_pin_at_index",
            [](const PinGroup<ModulePin>& self, u32 index) -> ModulePin* {
                auto res = self.get_pin_at_index(index);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("index"),
            R"(
            Get the pin specified by the given index.

            :param int index: The index of the pin within the pin group.
            :returns: The pin on success, None otherwise.
            :rtype: hal_py.ModulePin or None
        )");

        py_module_pin_group.def(
            "get_index",
            [](const PinGroup<ModulePin>& self, const ModulePin* pin) -> i32 {
                auto res = self.get_index(pin);
                if (res.is_ok())
                {
                    return (i32)res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return -1;
                }
            },
            py::arg("pin"),
            R"(
            Get the index within the pin group of the given pin.

            :param hal_py.ModulePin pin: The pin
            :returns: The index of the pin on success, -1 otherwise.
            :rtype: int
        )");

        py_module_pin_group.def_property_readonly("ascending", &PinGroup<ModulePin>::is_ascending, R"(
            True if the pin order of a pin group comprising n pins is ascending (from 0 to n-1), False if it is descending (from n-1 to 0).

            :type: bool
        )");

        py_module_pin_group.def("is_ascending", &PinGroup<ModulePin>::is_ascending, R"(
            Check whether the pin order of a pin group comprising n pins is ascending (from 0 to n-1) or descending (from n-1 to 0).

            :returns: True for ascending bit order, False otherwise.
            :rtype: bool
        )");

        py_module_pin_group.def_property_readonly("start_index", &PinGroup<ModulePin>::get_start_index, R"(
            The start index of the pin group.
            Commonly, pin groups start at index 0, but this may not always be the case.
            Note that the start index for a pin group comprising n pins is 0 independent of whether it is ascending or descending.

            :type: int
        )");

        py_module_pin_group.def("get_start_index", &PinGroup<ModulePin>::get_start_index, R"(
            Get the start index of the pin group.
            Commonly, pin groups start at index 0, but this may not always be the case.
            Note that the start index for a pin group comprising n pins is 0 independent of whether it is ascending or descending.

            :returns: The start index. 
            :rtype: int
        )");

        py_module_pin_group.def("empty", &PinGroup<ModulePin>::empty, R"(
            Check whether the pin group is empty, i.e., contains no pins.

            :returns: True if the pin group is empty, False otherwise.
            :rtype: bool
        )");

        py_module_pin_group.def("size", &PinGroup<ModulePin>::size, R"(
            Get the size, i.e., the number of pins, of the pin group.

            :returns: The size of the pin group.
            :rtype: int
        )");
    }
}    // namespace hal