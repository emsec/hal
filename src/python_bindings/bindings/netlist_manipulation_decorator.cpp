#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_manipulation_decorator_init(py::module& m)
    {
        py::class_<NetlistManipulationDecorator> py_netlist_manipulation_decorator(m, "NetlistManipulationDecorator", R"()");

        py_netlist_manipulation_decorator.def(py::init<Netlist&>(), py::arg("netlist"), R"(
            Construct new NetlistDecorator object.

            :param hal_py.Netlist netlist: The netlist to operate on.
        )");

        py_netlist_manipulation_decorator.def(
            "delete_modules",
            [](NetlistManipulationDecorator& self, const std::function<bool(const Module*)>& filter = nullptr) -> bool {
                auto res = self.delete_modules(filter);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while deleting modules:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("filter") = nullptr,
            R"(
            Delete all modules in the  netlist except the top module.
            An optional filter can be specified to delete only modules fulfilling a certain condition.

            :param lambda filter: An optional filter to be applied to the modules before deletion.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_netlist_manipulation_decorator.def(
            "replace_gate",
            [](NetlistManipulationDecorator& self, Gate* gate, GateType* target_type, std::map<GatePin*, GatePin*> pin_map) -> bool {
                auto res = self.replace_gate(gate, target_type, pin_map);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while replacing gate:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("gate"),
            py::arg("target_type"),
            py::arg("pin_map"),
            R"(
            Replace the given gate with a gate of the specified gate type.
            A map from old to new pins must be provided in order to correctly connect the gates inputs and outputs.
            A pin can be omitted if no connection at that pin is desired.

            :param hal_py.Gate gate: The gate to be replaced.
            :param hal_py.GateType target_type: The gate type of the replacement gate.
            :param dict[hal_py.GatePin,hal_py.GatePin] pin_map: A dict from old to new pins.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal