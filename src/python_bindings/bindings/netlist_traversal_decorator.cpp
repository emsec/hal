#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_traversal_decorator_init(py::module& m)
    {
        py::class_<NetlistTraversalDecorator> py_netlist_traversal_decorator(
            m, "NetlistTraversalDecorator", R"(A netlist decorator that provides functionality to traverse the associated netlist without making any modifications.)");

        py_netlist_traversal_decorator.def(py::init<Netlist&>(), py::arg("netlist"), R"(
            Construct new NetlistTraversalDecorator object.

            :param hal_py.Netlist netlist: The netlist to operate on.
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates",
            [](NetlistTraversalDecorator& self,
               const Net* net,
               bool successors,
               const std::function<bool(const Gate*)>& target_gate_filter,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates(net, successors, target_gate_filter, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("net"),
            py::arg("successors"),
            py::arg("target_gate_filter"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Traverse over gates that do not meet the ``target_gate_filter`` condition.
            Stop traversal if (1) the ``target_gate_filter`` evaluates to ``True``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            Both the ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates",
            [](NetlistTraversalDecorator& self,
               const Gate* gate,
               bool successors,
               const std::function<bool(const Gate*)>& target_gate_filter,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates(gate, successors, target_gate_filter, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("gate"),
            py::arg("successors"),
            py::arg("target_gate_filter"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Traverse over gates that do not meet the ``target_gate_filter`` condition.
            Stop traversal if (1) the ``target_gate_filter`` evaluates to ``True``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            Both the ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates_until",
            [](NetlistTraversalDecorator& self,
               const Net* net,
               bool successors,
               const std::function<bool(const Gate*)>& target_gate_filter                                 = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until(net, successors, target_gate_filter, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("net"),
            py::arg("successors"),
            py::arg("target_gate_filter")    = nullptr,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if (1) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal) or (2) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            The target_gate_filter may be omitted in which case all traversed gates will be returned.
            Both ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted as well.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates_until",
            [](NetlistTraversalDecorator& self,
               const Gate* gate,
               bool successors,
               const std::function<bool(const Gate*)>& target_gate_filter                                 = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until(gate, successors, target_gate_filter, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("gate"),
            py::arg("successors"),
            py::arg("target_gate_filter")    = nullptr,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if (1) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal) or (2) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            The target_gate_filter may be omitted in which case all traversed gates will be returned.
            Both ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted as well.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates_until_depth",
            [](NetlistTraversalDecorator& self, const Net* net, bool successors, const std::function<bool(const Gate*)>& target_gate_filter = nullptr, u32 depth = 0)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until_depth(net, successors, target_gate_filter, depth);
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
            py::arg("net"),
            py::arg("successors"),
            py::arg("target_gate_filter") = nullptr,
            py::arg("depth")              = 0,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if the specified depth is reached.
            The current depth is counted starting at 1 for the destinations of the provided net. 
            If no depth is provided, all gates between the start net and the global netlist outputs will be traversed.
            The target_gate_filter may be omitted in which case all traversed gates will be returned.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param int depth: The maximum depth for netlist traversal starting from the start net.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates_until_depth",
            [](NetlistTraversalDecorator& self, const Gate* gate, bool successors, const std::function<bool(const Gate*)>& target_gate_filter = nullptr, u32 depth = 0)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until_depth(gate, successors, target_gate_filter, depth);
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
            py::arg("gate"),
            py::arg("successors"),
            py::arg("target_gate_filter") = nullptr,
            py::arg("depth")              = 0,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if the specified depth is reached.
            The current depth is counted starting at 1 for the direct successors/predecessors of the provided gate. 
            If no depth is provided, all gates between the start gate and the global netlist outputs will be traversed.
            The target_gate_filter may be omitted in which case all traversed gates will be returned.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param int depth: The maximum depth for netlist traversal starting from the start gate.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_sequential_gates",
            [](NetlistTraversalDecorator& self, const Net* net, bool successors, const std::set<PinType>& forbidden_pins) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_sequential_gates(net, successors, forbidden_pins, nullptr);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next sequential gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("net"),
            py::arg("successors"),
            py::arg("forbidden_pins"),
            R"(
            Starting from the given net, traverse the netlist and return only the next layer of sequential successor/predecessor gates.
            Traverse over gates that are not sequential until a sequential gate is found.
            Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Sequential gates reached through these pins will not be part of the result. Defaults to an empty set.
            :returns: The next sequential gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_sequential_gates",
            [](NetlistTraversalDecorator& self, const Net* net, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_sequential_gates(net, successors, forbidden_pins, cache);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next sequential gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("net"),
            py::arg("successors"),
            py::arg("forbidden_pins"),
            py::arg("cache"),
            R"(
            Starting from the given net, traverse the netlist and return only the next layer of sequential successor/predecessor gates.
            Traverse over gates that are not sequential until a sequential gate is found.
            Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.
            Provide a cache to speed up traversal when calling this function multiple times on the same netlist using the same forbidden pins.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Sequential gates reached through these pins will not be part of the result.
            :param dict[hal_py.Net, set[hal_py.Gate]] cache: A cache that can be used for better performance on repeated calls.
            :returns: The next sequential gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_sequential_gates",
            [](NetlistTraversalDecorator& self, const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_sequential_gates(gate, successors, forbidden_pins, nullptr);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next sequential gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("successors"),
            py::arg("forbidden_pins"),
            R"(
            Starting from the given gate, traverse the netlist and return only the next layer of sequential successor/predecessor gates.
            Traverse over gates that are not sequential until a sequential gate is found.
            Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Sequential gates reached through these pins will not be part of the result. Defaults to an empty set.
            :returns: The next sequential gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_sequential_gates",
            [](NetlistTraversalDecorator& self, const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_sequential_gates(gate, successors, forbidden_pins, cache);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next sequential gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("successors"),
            py::arg("forbidden_pins"),
            py::arg("cache"),
            R"(
            Starting from the given gate, traverse the netlist and return only the next layer of sequential successor/predecessor gates.
            Traverse over gates that are not sequential until a sequential gate is found.
            Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.
            Provide a cache to speed up traversal when calling this function multiple times on the same netlist using the same forbidden pins.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Sequential gates reached through these pins will not be part of the result.
            :param dict[hal_py.Net, set[hal_py.Gate]] cache: A cache that can be used for better performance on repeated calls.
            :returns: The next sequential gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_sequential_gates_map",
            [](NetlistTraversalDecorator& self, bool successors, const std::set<PinType>& forbidden_pins) -> std::optional<std::map<Gate*, std::set<Gate*>>> {
                auto res = self.get_next_sequential_gates_map(successors, forbidden_pins);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next sequential gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("successors"),
            py::arg("forbidden_pins"),
            R"(
            Get the next sequential gates for all sequential gates in the netlist by traversing through remaining logic (e.g., combinational logic).
            Compute a dict from a sequential gate to all its successors.
            Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.

            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Sequential gates reached through these pins will not be part of the result.
            :returns: A dict from each sequential gate to all its sequential successors on success, ``None`` otherwise.
            :rtype: dict[hal_py.Gate,set[hal_py.Gate]] or None
        )");
    }
}    // namespace hal