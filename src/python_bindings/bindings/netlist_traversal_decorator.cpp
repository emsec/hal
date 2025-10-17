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
               bool continue_on_match                                                                     = false,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates(net, successors, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Traverse over gates that do not meet the ``target_gate_filter`` condition.
            Stop traversal if (1) ``continue_on_match`` is ``False`` the ``target_gate_filter`` evaluates to ``True``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            Both the ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param bool continue_on_match: Set ``True`` to continue even if ``target_gate_filter`` evaluated to ``True``, ``False`` otherwise. Defaults to ``False``.
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
               bool continue_on_match                                                                     = false,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates(gate, successors, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Traverse over gates that do not meet the ``target_gate_filter`` condition.
            Stop traversal if (1) ``continue_on_match`` is ``False`` the ``target_gate_filter`` evaluates to ``True``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            Both the ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param bool continue_on_match: Set ``True`` to continue even if ``target_gate_filter`` evaluated to ``True``, ``False`` otherwise. Defaults to ``False``.
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
               const std::function<bool(const Gate*)>& target_gate_filter,
               bool continue_on_mismatch                                                                  = false,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until(net, successors, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("continue_on_mismatch")  = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if (1) ``continue_on_mismatch`` is ``False`` the ``target_gate_filter`` evaluates to ``False``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            Both ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param bool continue_on_mismatch: Set ``True`` to continue even if ``target_gate_filter`` evaluated to ``False``, ``False`` otherwise. Defaults to ``False``.
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
               const std::function<bool(const Gate*)>& target_gate_filter,
               bool continue_on_mismatch                                                                  = false,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until(gate, successors, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
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
            py::arg("continue_on_mismatch")  = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if (1) ``continue_on_mismatch`` is ``False`` the ``target_gate_filter`` evaluates to ``False``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
            Both ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :param bool continue_on_mismatch: Set ``True`` to continue even if ``target_gate_filter`` evaluated to ``False``, ``False`` otherwise. Defaults to ``False``.
            :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates_until_depth",
            [](NetlistTraversalDecorator& self, const Net* net, bool successors, u32 max_depth, const std::function<bool(const Gate*)>& target_gate_filter = nullptr)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until_depth(net, successors, max_depth, target_gate_filter);
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
            py::arg("max_depth"),
            py::arg("target_gate_filter") = nullptr,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if the specified depth is reached.
            The current depth is counted starting at 1 for the destinations of the provided net. 
            For a ``max_depth`` of ``0``, all gates between the start net and the global netlist outputs will be traversed.
            The target_gate_filter may be omitted in which case all traversed gates will be returned.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param int max_depth: The maximum depth for netlist traversal starting from the start net.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_matching_gates_until_depth",
            [](NetlistTraversalDecorator& self, const Gate* gate, bool successors, u32 max_depth, const std::function<bool(const Gate*)>& target_gate_filter = nullptr)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until_depth(gate, successors, max_depth, target_gate_filter);
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
            py::arg("max_depth"),
            py::arg("target_gate_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continue traversal independent of whatever ``target_gate_filter`` evaluates to.
            Stop traversal if the specified depth is reached.
            The current depth is counted starting at 1 for the direct successors/predecessors of the provided gate. 
            For a ``max_depth`` of ``0``, all gates between the start gate and the global netlist outputs will be traversed.
            The target_gate_filter may be omitted in which case all traversed gates will be returned.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param int max_depth: The maximum depth for netlist traversal starting from the start gate.
            :param lambda target_gate_filter: Filter condition that must be met for the target gates.
            :returns: The next gates fulfilling the target gate filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        // Binding for get_next_matching_endpoints starting from Endpoint*
        py_netlist_traversal_decorator.def(
            "get_next_matching_endpoints",
            [](NetlistTraversalDecorator& self,
               Endpoint* endpoint,
               bool successors,
               const std::function<bool(const Endpoint*)>& target_endpoint_filter,
               bool continue_on_match                                                                     = false,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Endpoint*>> {
                auto res = self.get_next_matching_endpoints(endpoint, successors, target_endpoint_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next endpoints:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            py::arg("successors"),
            py::arg("target_endpoint_filter"),
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given endpoint, traverse the netlist and return only the successor/predecessor endpoints for which the ``target_endpoint_filter`` evaluates to ``True``.
            Traverse over endpoints that do not meet the ``target_endpoint_filter`` condition.
            Stop traversal if (1) ``continue_on_match`` is ``False`` and the ``target_endpoint_filter`` evaluates to ``True``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current endpoint during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next endpoint during traversal).
            Both the ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Endpoint endpoint: Start endpoint.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param callable target_endpoint_filter: Filter condition that must be met for the target endpoints.
            :param bool continue_on_match: Set ``True`` to continue even if ``target_endpoint_filter`` evaluated to ``True``, ``False`` otherwise. Defaults to ``False``.
            :param callable exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param callable entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next endpoints fulfilling the target endpoint filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Endpoint] or None
        )");

        // Binding for get_next_matching_endpoints starting from Net*
        py_netlist_traversal_decorator.def(
            "get_next_matching_endpoints",
            [](NetlistTraversalDecorator& self,
               const Net* net,
               bool successors,
               const std::function<bool(const Endpoint*)>& target_endpoint_filter,
               bool continue_on_match                                                                     = false,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Endpoint*>> {
                auto res = self.get_next_matching_endpoints(net, successors, target_endpoint_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next endpoints:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("net"),
            py::arg("successors"),
            py::arg("target_endpoint_filter"),
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor endpoints for which the ``target_endpoint_filter`` evaluates to ``True``.
            Traverse over endpoints that do not meet the ``target_endpoint_filter`` condition.
            Stop traversal if (1) ``continue_on_match`` is ``False`` and the ``target_endpoint_filter`` evaluates to ``True``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current endpoint during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next endpoint during traversal).
            Both the ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param callable target_endpoint_filter: Filter condition that must be met for the target endpoints.
            :param bool continue_on_match: Set ``True`` to continue even if ``target_endpoint_filter`` evaluated to ``True``, ``False`` otherwise. Defaults to ``False``.
            :param callable exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param callable entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next endpoints fulfilling the target endpoint filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Endpoint] or None
        )");

        // Binding for get_next_matching_endpoints starting from Gate*
        py_netlist_traversal_decorator.def(
            "get_next_matching_endpoints",
            [](NetlistTraversalDecorator& self,
               const Gate* gate,
               bool successors,
               const std::function<bool(const Endpoint*)>& target_endpoint_filter,
               bool continue_on_match                                                                     = false,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Endpoint*>> {
                auto res = self.get_next_matching_endpoints(gate, successors, target_endpoint_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next endpoints:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("successors"),
            py::arg("target_endpoint_filter"),
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor endpoints for which the ``target_endpoint_filter`` evaluates to ``True``.
            Traverse over endpoints that do not meet the ``target_endpoint_filter`` condition.
            Stop traversal if (1) ``continue_on_match`` is ``False`` and the ``target_endpoint_filter`` evaluates to ``True``, (2) the ``exit_endpoint_filter`` evaluates to ``False`` on a fan-in/out endpoint (i.e., when exiting the current endpoint during traversal), or (3) the ``entry_endpoint_filter`` evaluates to ``False`` on a successor/predecessor endpoint (i.e., when entering the next endpoint during traversal).
            Both the ``entry_endpoint_filter`` and the ``exit_endpoint_filter`` may be omitted.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param callable target_endpoint_filter: Filter condition that must be met for the target endpoints.
            :param bool continue_on_match: Set ``True`` to continue even if ``target_endpoint_filter`` evaluated to ``True``, ``False`` otherwise. Defaults to ``False``.
            :param callable exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param callable entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The next endpoints fulfilling the target endpoint filter condition on success, ``None`` otherwise.
            :rtype: set[hal_py.Endpoint] or None
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
            :param set[hal_py.PinType] forbidden_pins: Sequential gates reached through these pins will not be part of the result.
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

        py_netlist_traversal_decorator.def(
            "get_next_combinational_gates",
            [](NetlistTraversalDecorator& self, const Net* net, bool successors, const std::set<PinType>& forbidden_pins) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_combinational_gates(net, successors, forbidden_pins, nullptr);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next combinational gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("net"),
            py::arg("successors"),
            py::arg("forbidden_pins"),
            R"(
            Starting from the given net, traverse the netlist and return all combinational successor/predecessor gates.
            Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
            All combinational gates found during traversal are added to the result.
            Forbidden pins can be provided to, e.g., avoid the inclusion of logic in front of flip-flop control inputs.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Netlist traversal stops at these pins.
            :returns: The next combinational gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_combinational_gates",
            [](NetlistTraversalDecorator& self, const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_combinational_gates(gate, successors, forbidden_pins, nullptr);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting next combinational gates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("successors"),
            py::arg("forbidden_pins"),
            R"(
            Starting from the given gate, traverse the netlist and return all combinational successor/predecessor gates.
            Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
            All combinational gates found during traversal are added to the result.
            Forbidden pins can be provided to, e.g., avoid the inclusion of logic in front of flip-flop control inputs.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Netlist traversal stops at these pins.
            :returns: The next combinational gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_shortest_path_distance",
            [](NetlistTraversalDecorator& self,
               Endpoint* start,
               const std::function<bool(const Endpoint*)>& target_filter,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter = nullptr) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_filter, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting shortest path distance:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_filter"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Find the length of the shortest path connecting the start endpoint to a target matching the given filter.
            If there is no such path, ``None`` is returned.
            Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.

            :param hal_py.Endpoint start: The starting endpoint.
            :param callable target_filter: A filter function to determine the target endpoints.
            :param hal_py.PinDirection direction: The direction to search in (``hal_py.PinDirection.input``, ``hal_py.PinDirection.output``, or ``hal_py.PinDirection.inout``).
            :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: The shortest distance between the start and target endpoints on success, ``None`` otherwise.
            :rtype: int or None
        )");

        // Binding for get_shortest_path_distance starting from Gate* with target_filter
        py_netlist_traversal_decorator.def(
            "get_shortest_path_distance",
            [](NetlistTraversalDecorator& self,
               const Gate* start,
               const std::function<bool(const Endpoint*)>& target_filter,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter = nullptr) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_filter, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting shortest path distance:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_filter"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Find the length of the shortest path connecting the start gate to a target matching the given filter.
            If there is no such path, ``None`` is returned.
            Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.

            :param hal_py.Gate start: The starting gate.
            :param callable target_filter: A filter function to determine the target endpoints.
            :param hal_py.PinDirection direction: The direction to search in (``hal_py.PinDirection.input``, ``hal_py.PinDirection.output``, or ``hal_py.PinDirection.inout``).
            :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: The shortest distance between the start gate and target endpoints on success, ``None`` otherwise.
            :rtype: int or None
        )");

        py_netlist_traversal_decorator.def(
            "get_shortest_path_distance",
            [](NetlistTraversalDecorator& self,
               const Gate* start_gate,
               const Gate* end_gate,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter = nullptr) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start_gate, end_gate, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting shortest path distance:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start_gate"),
            py::arg("end_gate"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Find the length of the shortest path (i.e., the smallest number of gates) that connects the start gate with the end gate.
                If there is no such path, ``None`` is returned.
                Computing only the shortest distance to a gate is faster than computing the shortest path, since it does not have to keep track of the path to reach each gate.

                :param hal_py.Gate start_gate: The gate to start from.
                :param hal_py.Gate end_gate: The gate to connect to.
                :param hal_py.PinDirection direction: The direction to search in. Can be ``hal_py.PinDirection.input``, ``hal_py.PinDirection.output``, or ``hal_py.PinDirection.inout`` to search both directions and return the shorter one.
                :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
                :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
                :returns: The shortest distance between the start and end gate on success, ``None`` otherwise.
                :rtype: int or None
            )");

        // Binding for get_shortest_path
        py_netlist_traversal_decorator.def(
            "get_shortest_path",
            [](NetlistTraversalDecorator& self,
               const Gate* start_gate,
               const Gate* end_gate,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::vector<Gate*>> {
                auto res = self.get_shortest_path(start_gate, end_gate, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting shortest path:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start_gate"),
            py::arg("end_gate"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Find the shortest path (i.e., the smallest number of gates) that connects the start gate with the end gate.
                The gate where the search started from will be the first in the result list, the end gate will be the last.
                If there is no such path, ``None`` is returned. If there is more than one path with the same length, only the first one is returned.

                :param hal_py.Gate start_gate: The gate to start from.
                :param hal_py.Gate end_gate: The gate to connect to.
                :param hal_py.PinDirection direction: The direction to search in. Can be ``hal_py.PinDirection.input``, ``hal_py.PinDirection.output``, or ``hal_py.PinDirection.inout`` to search both directions and return the shorter one.
                :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
                :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
                :returns: A list of gates that connect the start with end gate on success, ``None`` otherwise.
                :rtype: list[hal_py.Gate] or None
            )");

        py_netlist_traversal_decorator.def(
            "get_combinational_subgraphs",
            [](NetlistTraversalDecorator& self) -> std::optional<std::vector<std::vector<Gate*>>> {
                auto res = self.get_combinational_subgraphs();
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting combinational subgraphs:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            R"(
                Find all combinational subgraphs in the netlist.
                A combinational subgraph is defined as a connected set of gates that are all of combinational type

                :rtype: list[list[hal_py.Gate]] or None
            )");

        py_netlist_traversal_decorator.def(
            "get_matching_subgraphs",
            [](NetlistTraversalDecorator& self,
               const std::function<bool(const Gate*)>& gate_filter,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) -> std::optional<std::vector<std::vector<Gate*>>> {
                auto res = self.get_matching_subgraphs(gate_filter, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting matching subgraphs:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate_filter"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Find all subgraphs in the netlist where all gates match the given filter.
                A subgraph is defined as a connected set of gates.
                A connection between gates has to be established through pins that do not get filtered out by the `exit_endpoint_filter` and the `entry_endpoint_filter`.

                :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
                :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
                :param callable gate_filter: Filter condition that must be met for all gates in the subgraph.
                :returns: A list of subgraphs where each subgraph is represented as a list of gates on success, ``None`` otherwise.
                :rtype: list[list[hal_py.Gate]] or None
            )");
    }
}    // namespace hal