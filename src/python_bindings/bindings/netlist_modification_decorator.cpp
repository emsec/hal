#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_modification_decorator_init(py::module& m)
    {
        py::class_<NetlistModificationDecorator> py_netlist_modification_decorator(
            m, "NetlistModificationDecorator", R"(A netlist decorator that provides functionality to modify the associated netlist.)");

        py_netlist_modification_decorator.def(py::init<Netlist&>(), py::arg("netlist"), R"(
            Construct new NetlistModificationDecorator object.

            :param hal_py.Netlist netlist: The netlist to operate on.
        )");

        py_netlist_modification_decorator.def(
            "delete_modules",
            [](NetlistModificationDecorator& self, const std::function<bool(const Module*)>& filter = nullptr) -> bool {
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

        py_netlist_modification_decorator.def(
            "replace_gate",
            [](NetlistModificationDecorator& self, Gate* gate, GateType* target_type, const std::map<GatePin*, GatePin*>& pin_map) -> Gate* {
                auto res = self.replace_gate(gate, target_type, pin_map);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while replacing gate:\n{}", res.get_error().get());
                    return nullptr;
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
            :returns: The new gate on success, None otherwise.
            :rtype: hal_py.Gate or None
        )");

        py_netlist_modification_decorator.def(
            "connect_gates",
            [](NetlistModificationDecorator& self, Gate* src_gate, GatePin* src_pin, Gate* dst_gate, GatePin* dst_pin) -> Net* {
                auto res = self.connect_gates(src_gate, src_pin, dst_gate, dst_pin);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while connecting gates:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("src_gate"),
            py::arg("src_pin"),
            py::arg("dst_gate"),
            py::arg("dst_pin"),
            R"(
            Connects two gates through the specified pins.
            If both pins are not yet connected to a net, a new net is created to connect both pins.
            If one of the pins is already connected to a net, that net is connected to the other pin.
            If both pins are already connected to a net, an error is returned.

            :param hal_py.Gate src_gate: The source gate.
            :param hal_py.GatePin src_pin: The output pin of the source gate.
            :param hal_py.Gate dst_gate: The destination gate.
            :param hal_py.GatePin dst_pin: The input pin of the destination gate.
            :returns: The connecting net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");

        py_netlist_modification_decorator.def(
            "connect_nets",
            [](NetlistModificationDecorator& self, Net* master_net, Net* slave_net) -> Net* {
                auto res = self.connect_nets(master_net, slave_net);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while connecting nets:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("master_net"),
            py::arg("slave_net"),
            R"(
            Connects (and thereby merges) two nets.
            All properties of the slave net are transfered to the master net and the slave net is subsequently deleted.

            :param hal_py.Net master_net: The net that receives all properties from the slave net. 
            :param hal_py.Net slave_net: The net that transfers all properties to the master net and is subsequently deleted.
            :returns: The merged net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");
    }
}    // namespace hal