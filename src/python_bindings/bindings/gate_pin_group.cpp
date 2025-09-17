#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_pin_group_init(py::module& m)
    {
        py::class_<PinGroup<GatePin>, RawPtrWrapper<PinGroup<GatePin>>> py_gate_pin_group(m, "GatePinGroup", R"(
            A group of gate pins made up of a name, the pins, a pin order, and a start index.
        )");

        py_gate_pin_group.def(py::self == py::self, R"(
            Check whether two gate pin groups are equal.

            :returns: True if both gate pin groups are equal, False otherwise.
            :rtype: bool
        )");

        py_gate_pin_group.def(py::self != py::self, R"(
            Check whether two gate pin groups are unequal.

            :returns: True if both gate pin groups are unequal, False otherwise.
            :rtype: bool
        )");

        py_gate_pin_group.def("__hash__", &PinGroup<GatePin>::get_hash, R"(
            Python requires hash for set and dict container.

            :returns: The hash.
            :rtype: Py_hash_t
        )");

        py_gate_pin_group.def_property_readonly("id", &PinGroup<GatePin>::get_id, R"(
            The ID of the gate pin group. The ID is unique within a gate type.

            :type: int
        )");

        py_gate_pin_group.def("get_id", &PinGroup<GatePin>::get_id, R"(
            Get the ID of the gate pin group. The ID is unique within a gate type.

            :returns: The ID of the pin.
            :rtype: int
        )");

        py_gate_pin_group.def_property_readonly("name", &PinGroup<GatePin>::get_name, R"(
            The name of the pin group.

            :type: str
        )");

        py_gate_pin_group.def("get_name", &PinGroup<GatePin>::get_name, R"(
            Get the name of the pin group.

            :returns: The name of the pin group.
            :rtype: str
        )");

        py_gate_pin_group.def_property_readonly("direction", &PinGroup<GatePin>::get_direction, R"(
            The direction of the pin group.

            :type: hal_py.PinDirection
        )");

        py_gate_pin_group.def("get_direction", &PinGroup<GatePin>::get_direction, R"(
            Get the direction of the pin group.

            :returns: The direction of the pin.
            :rtype: hal_py.PinDirection
        )");

        py_gate_pin_group.def_property_readonly("type", &PinGroup<GatePin>::get_type, R"(
            The type of the pin group.

            :type: hal_py.PinType
        )");

        py_gate_pin_group.def("get_type", &PinGroup<GatePin>::get_type, R"(
            Get the type of the pin group.

            :returns: The type of the pin.
            :rtype: hal_py.PinType
        )");

        py_gate_pin_group.def_property_readonly("pins", [](const PinGroup<GatePin>& self) -> std::vector<GatePin*> { return self.get_pins(nullptr); }, R"(
            The (ordered) pins of the pin groups.

            :type: list[hal_py.GatePin]
        )");

        py_gate_pin_group.def("get_pins", &PinGroup<GatePin>::get_pins, py::arg("filter") = nullptr, R"(
            Get the (ordered) pins of the pin groups.
            The optional filter is evaluated on every pin such that the result only contains pins matching the specified condition.

            :param lambda filter: Filter function to be evaluated on each pin.
            :returns: The ordered pins.
            :rtype: list[hal_py.GatePin]
        )");

        py_gate_pin_group.def(
            "get_pin_at_index",
            [](const PinGroup<GatePin>& self, u32 index) -> GatePin* {
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
            :rtype: hal_py.GatePin or None
        )");

        py_gate_pin_group.def(
            "get_index",
            [](const PinGroup<GatePin>& self, const GatePin* pin) -> i32 {
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

            :param hal_py.GatePin pin: The pin
            :returns: The index of the pin on success, -1 otherwise.
            :rtype: int
        )");

        py_gate_pin_group.def("contains_pin", &PinGroup<GatePin>::contains_pin, py::arg("pin"), R"(
            Check whether the pin group contains the given pin.

            :param hal_py.GatePin pin: The pin to check.
            :returns: True if the pin group contains the pin, True otherwise.
            :rtype: bool
        )");

        py_gate_pin_group.def_property_readonly("ascending", &PinGroup<GatePin>::is_ascending, R"(
            ``True`` if the pin order of a pin group comprising n pins is ascending, e.g., from index ``0`` to ``n-1``, ``False`` otherwise.

            :type: bool
        )");

        py_gate_pin_group.def("is_ascending", &PinGroup<GatePin>::is_ascending, R"(
            Check whether the pin order of a pin group comprising n pins is ascending, e.g., from index ``0`` to ``n-1``.

            :returns: ``True`` for ascending bit order, ``False`` otherwise.
            :rtype: bool
        )");

        py_gate_pin_group.def_property_readonly("descending", &PinGroup<GatePin>::is_descending, R"(
            ``True`` if the pin order of a pin group comprising n pins is descending, e.g., from index ``n-1`` to ``0``, ``False`` otherwise.

            :type: bool
        )");

        py_gate_pin_group.def("is_descending", &PinGroup<GatePin>::is_descending, R"(
            Check whether the pin order of a pin group comprising n pins is descending, e.g., from index ``n-1`` to ``0``.

            :returns: ``True`` for descending bit order, ``False`` otherwise.
            :rtype: bool
        )");

        py_gate_pin_group.def_property_readonly("lowest_index", &PinGroup<GatePin>::get_lowest_index, R"(
            The numerically lowest index of the pin group.

            :type: int
        )");

        py_gate_pin_group.def("get_lowest_index", &PinGroup<GatePin>::get_lowest_index, R"(
            Get the numerically lowest index of the pin group.

            :returns: The lowest index. 
            :rtype: int
        )");

        py_gate_pin_group.def_property_readonly("highest_index", &PinGroup<GatePin>::get_highest_index, R"(
            The numerically highest index of the pin group.

            :type: int
        )");

        py_gate_pin_group.def("get_highest_index", &PinGroup<GatePin>::get_highest_index, R"(
            Get the numerically highest index of the pin group.

            :returns: The highest index. 
            :rtype: int
        )");

        py_gate_pin_group.def_property_readonly("start_index", &PinGroup<GatePin>::get_start_index, R"(
            The start index of the pin group.
            For ascending pin groups, this index equals the lowest index of the pin group.
            For descending pin groups, it is equal to the highest index of the pin group.

            :type: int
        )");

        py_gate_pin_group.def("get_start_index", &PinGroup<GatePin>::get_start_index, R"(
            Get the start index of the pin group.
            For ascending pin groups, this index equals the lowest index of the pin group.
            For descending pin groups, it is equal to the highest index of the pin group.

            :returns: The start index. 
            :rtype: int
        )");

        py_gate_pin_group.def_property_readonly("ordered", &PinGroup<GatePin>::is_ordered, R"(
            ``True`` if the pin group is inherently ordered, ``False`` otherwise.

            :type: bool
        )");

        py_gate_pin_group.def("is_ordered", &PinGroup<GatePin>::is_ordered, R"(
            Check whether the pin group features an inherent order.

            :returns: ``True`` if the pin group is inherently ordered, ``False`` otherwise.
            :rtype: bool
        )");

        py_gate_pin_group.def("empty", &PinGroup<GatePin>::empty, R"(
            Check whether the pin group is empty, i.e., contains no pins.

            :returns: True if the pin group is empty, False otherwise.
            :rtype: bool
        )");

        py_gate_pin_group.def("size", &PinGroup<GatePin>::size, R"(
            Get the size, i.e., the number of pins, of the pin group.

            :returns: The size of the pin group.
            :rtype: int
        )");
    }
}    // namespace hal