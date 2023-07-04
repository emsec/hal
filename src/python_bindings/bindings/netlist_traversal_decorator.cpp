#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_traversal_decorator_init(py::module& m)
    {
        py::class_<NetlistTraversalDecorator> py_netlist_traversal_decorator(m, "NetlistTraversalDecorator", R"()");

        py_netlist_traversal_decorator.def(py::init<Netlist&>(), py::arg("netlist"), R"(
            Construct new NetlistTraversalDecorator object.

            :param hal_py.Netlist netlist: The netlist to operate on.
        )");

        py_netlist_traversal_decorator.def(
            "get_next_gates",
            [](NetlistTraversalDecorator& self,
               std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache,
               const Net* net,
               bool successors,
               const std::function<bool(const Gate*)>& filter,
               const std::set<PinType>& forbidden_pins = {}) -> std::optional<std::unordered_set<Gate*>> {
                auto res = self.get_next_gates(cache, net, successors, filter, forbidden_pins);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("cache"),
            py::arg("net"),
            py::arg("successors"),
            py::arg("filter"),
            py::arg("forbidden_pins") = std::set<PinType>(),
            R"(
            Starting from the given net, get the successor/predecessor gates for which the filter evaluates to ``True``.
            Does not continue traversal beyond gates fulfilling the filter condition, i.e., only the first layer of successors/predecessors is returned.

            :param dict[hal_py.Net,set[hal_py.Gate]] cache: Gate cache to speed up traversal for parts of the netlist that have been traversed before.  
            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda filter: Filter condition that must be met for the target gates.
            :param set[hal_py.PinType] forbidden_pins: Types of pins through which propagation shall not continue. Defaults to en empty set.
            :returns: The next gates fulfilling the filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_gates",
            [](NetlistTraversalDecorator& self,
               std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache,
               const Gate* gate,
               bool successors,
               const std::function<bool(const Gate*)>& filter,
               const std::set<PinType>& forbidden_pins = {}) -> std::optional<std::unordered_set<Gate*>> {
                auto res = self.get_next_gates(cache, gate, successors, filter, forbidden_pins);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("cache"),
            py::arg("gate"),
            py::arg("successors"),
            py::arg("filter"),
            py::arg("forbidden_pins") = std::set<PinType>(),
            R"(
            Starting from the given gate, get the successor/predecessor gates for which the filter evaluates to ``True``.
            Does not continue traversal beyond gates fulfilling the filter condition, i.e., only the first layer of successors/predecessors is returned.

            :param dict[hal_py.Net,set[hal_py.Gate]] cache: Gate cache to speed up traversal for parts of the netlist that have been traversed before.
            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda filter: Filter condition that must be met for the target gates.
            :param set[hal_py.PinType] forbidden_pins: Types of pins through which propagation shall not continue. Defaults to en empty set.
            :returns: The next gates fulfilling the filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");
    }
}    // namespace hal