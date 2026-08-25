#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_traversal_decorator_init(py::module& m)
    {
        py::class_<NetlistTraversalDecorator> py_netlist_traversal_decorator(
            m, "NetlistTraversalDecorator", R"(A netlist decorator that provides functionality to traverse the associated netlist without making any modifications.)");

        py_netlist_traversal_decorator.def(py::init<Netlist&>(), py::arg("netlist"), py::keep_alive<1, 2>(), R"(
            Construct new NetlistTraversalDecorator object.

            :param hal_py.Netlist netlist: The netlist to operate on.
        )");

        py::enum_<TraversalDirection>(m, "TraversalDirection", R"(The direction in which a netlist is traversed.)")
            .value("forward", TraversalDirection::forward, R"(Follow the fan-out, i.e., towards the successors of a gate.)")
            .value("backward", TraversalDirection::backward, R"(Follow the fan-in, i.e., towards the predecessors of a gate.)")
            .value("both", TraversalDirection::both, R"(Follow both directions.)")
            .export_values();

        py::enum_<TraversalStop>(m, "TraversalStop", R"(Where a traversal stops relative to the gates it is looking for.)")
            .value("at_match", TraversalStop::at_match, R"(Stop at a gate the filter accepts, so the collected gates bound the search.)")
            .value("at_mismatch", TraversalStop::at_mismatch, R"(Stop at a gate the filter rejects, so the collected gates form a connected region.)")
            .value("never", TraversalStop::never, R"(Do not stop at a gate; bound the traversal with a depth or the endpoint filters.)")
            .export_values();

        py::class_<TraversalCache> py_traversal_cache(m, "TraversalCache", R"(
            A reusable store for the results of one specific traversal, handed to ``NetlistTraversalDecorator.get_gates``.

            The traversal a cache belongs to is sealed in when it is created and the cache can only ever be used for exactly that traversal, which is what makes reuse sound. Create one with ``NetlistTraversalDecorator.make_traversal_cache`` and drop it when the netlist is modified.
        )");

        py_netlist_traversal_decorator.def(
            "get_common_inputs",
            [](NetlistTraversalDecorator& self, const std::vector<Gate*>& gates, u32 threshold = 0) -> std::optional<std::vector<Net*>> {
                auto res = self.get_common_inputs(gates, threshold);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("gates"),
            py::arg("threshold") = 0,
            borrowed(),
            R"(
            Get the nets that are inputs to at least ``threshold`` of the given gates.

            Shared inputs across a group of gates typically indicate a shared control signal, so this is a cheap way to test whether a set of gates belongs together. Nets driven by GND or VCC gates do not count.

            :param list[hal_py.Gate] gates: The gates to inspect.
            :param int threshold: The number of gates a net has to feed. ``0`` to require all of them. Defaults to ``0``.
            :returns: The common input nets on success, ``None`` otherwise.
            :rtype: list[hal_py.Net] or None
        )");

        py_netlist_traversal_decorator.def(
            "make_traversal_cache",
            [](NetlistTraversalDecorator& self,
               TraversalDirection direction,
               const std::function<bool(const Gate*)>& match,
               TraversalStop stop,
               const std::function<bool(const Endpoint*)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*)>& entry_endpoint_filter = nullptr) -> std::unique_ptr<TraversalCache> {
                return std::make_unique<TraversalCache>(self.make_traversal_cache(direction, match, stop, exit_endpoint_filter, entry_endpoint_filter));
            },
            py::arg("direction"),
            py::arg("match"),
            py::arg("stop"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Create a cache for one specific traversal, to be handed to ``get_gates`` in place of the traversal's parameters.

            The direction must be ``TraversalDirection.forward`` or ``backward``. The endpoint filters receive no depth and there is no depth limit, as either would make the cached answers depend on how a net was reached.

            :param hal_py.TraversalDirection direction: The direction to traverse in.
            :param lambda match: The condition a gate has to meet to be collected.
            :param hal_py.TraversalStop stop: Where to stop traversing, relative to the gates that ``match`` accepts.
            :param lambda exit_endpoint_filter: Condition that has to hold to leave a gate.
            :param lambda entry_endpoint_filter: Condition that has to hold to enter a gate.
            :returns: The cache.
            :rtype: hal_py.TraversalCache
        )");

        py_netlist_traversal_decorator.def(
            "get_gates",
            [](NetlistTraversalDecorator& self, const Gate* gate, TraversalCache& cache) -> std::optional<std::set<Gate*>> {
                auto res = self.get_gates(gate, cache);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("gate"),
            py::arg("cache"),
            borrowed(),
            R"(
            Traverse the netlist from the given gate, sharing results through the cache: what an earlier call worked out is not walked again.

            :param hal_py.Gate gate: The gate to start from.
            :param hal_py.TraversalCache cache: The cache holding the traversal and its results.
            :returns: The collected gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_gates",
            [](NetlistTraversalDecorator& self, const Net* net, TraversalCache& cache) -> std::optional<std::set<Gate*>> {
                auto res = self.get_gates(net, cache);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("net"),
            py::arg("cache"),
            borrowed(),
            R"(
            Traverse the netlist from the given net, sharing results through the cache: what an earlier call worked out is not walked again.

            :param hal_py.Net net: The net to start from.
            :param hal_py.TraversalCache cache: The cache holding the traversal and its results.
            :returns: The collected gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_gates",
            [](NetlistTraversalDecorator& self,
               const Gate* gate,
               TraversalDirection direction,
               const std::function<bool(const Gate*)>& match,
               TraversalStop stop,
               u32 max_depth                                                          = 0,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_gates(gate, direction, match, stop, max_depth, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("gate"),
            py::arg("direction"),
            py::arg("match"),
            py::arg("stop"),
            py::arg("max_depth")             = 0,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            borrowed(),
            R"(
            Traverse the netlist from the given gate, collecting the gates that ``match`` accepts.

            :param hal_py.Gate gate: The gate to start from.
            :param hal_py.TraversalDirection direction: The direction to traverse in.
            :param lambda match: The condition a gate has to meet to be collected.
            :param hal_py.TraversalStop stop: Where to stop traversing, relative to the gates that ``match`` accepts.
            :param int max_depth: The maximum number of gates to traverse through. ``0`` for no limit.
            :param lambda exit_endpoint_filter: Condition that has to hold to leave a gate.
            :param lambda entry_endpoint_filter: Condition that has to hold to enter a gate.
            :returns: The collected gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
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
            borrowed(), R"(
            Starting from the given net, traverse the netlist and return all combinational successor/predecessor gates.
            Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
            All combinational gates found during traversal are added to the result.
            Forbidden pins can be provided to, e.g., avoid the inclusion of logic in front of flip-flop control inputs.

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Traversal stops at pins of these types, i.e., gates reached through such a pin are not part of the result. Defaults to an empty set.
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
            borrowed(), R"(
            Starting from the given gate, traverse the netlist and return all combinational successor/predecessor gates.
            Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
            All combinational gates found during traversal are added to the result.
            Forbidden pins can be provided to, e.g., avoid the inclusion of logic in front of flip-flop control inputs.

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.
            :param set[hal_py.PinType] forbidden_pins: Traversal stops at pins of these types, i.e., gates reached through such a pin are not part of the result. Defaults to an empty set.
            :returns: The next combinational gates on success, ``None`` otherwise.
            :rtype: set[hal_py.Gate] or None
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
                :param hal_py.PinDirection direction: The direction to search in. Can be ``hal_py.PinDirection.input``, ``hal_py.PinDirection.output`` or ``hal_py.PinDirection.inout`` to search both directions and return the shorter one.
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
            borrowed(), R"(
                Find the shortest path (i.e., the smallest number of gates) that connects the start gate with the end gate.
                The gate where the search started from will be the first in the result list, the end gate will be the last.
                If there is no such path, ``None`` is returned. If there is more than one path with the same length, only the first one is returned.

                :param hal_py.Gate start_gate: The gate to start from.
                :param hal_py.Gate end_gate: The gate to connect to.
                :param hal_py.PinDirection direction: The direction to search in. Can be ``hal_py.PinDirection.input``, ``hal_py.PinDirection.output`` or ``hal_py.PinDirection.inout`` to search both directions and return the shorter one.
                :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
                :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
                :returns: A list of gates that connect the start with end gate on success, ``None`` otherwise.
                :rtype: list[hal_py.Gate] or None
            )");

        py_netlist_traversal_decorator.def(
            "get_shortest_path",
            [](NetlistTraversalDecorator& self,
               const Gate* start_gate,
               const Module* end_module,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::vector<Gate*>> {
                auto res = self.get_shortest_path(start_gate, end_module, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("start_gate"),
            py::arg("end_module"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            borrowed(),
            R"(
            Find the shortest path that connects the start gate with any gate of the given module.

            :param hal_py.Gate start_gate: The gate to start from.
            :param hal_py.Module end_module: The module to connect to. Gates of its submodules count as belonging to it.
            :param hal_py.PinDirection direction: The direction to search in.
            :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The path on success, ``None`` otherwise.
            :rtype: list[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_shortest_path",
            [](NetlistTraversalDecorator& self,
               const Module* start_module,
               const Module* end_module,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::vector<std::vector<Gate*>>> {
                auto res = self.get_shortest_path(start_module, end_module, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("start_module"),
            py::arg("end_module"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            borrowed(),
            R"(
            Find every shortest path that connects the start module with the end module.

            :param hal_py.Module start_module: The module to start from. Gates of its submodules count as belonging to it.
            :param hal_py.Module end_module: The module to connect to. Gates of its submodules count as belonging to it.
            :param hal_py.PinDirection direction: The direction to search in.
            :param lambda exit_endpoint_filter: Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
            :param lambda entry_endpoint_filter: Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
            :returns: The shortest paths on success, ``None`` otherwise.
            :rtype: list[list[hal_py.Gate]] or None
        )");

py_netlist_traversal_decorator.def(
            "get_gate_chain",
            [](NetlistTraversalDecorator& self, Gate* start_gate, const std::vector<const GatePin*>& input_pins = {}, const std::vector<const GatePin*>& output_pins = {}, const std::function<bool(const Gate*)>& filter = nullptr)
                -> std::vector<Gate*> {
                auto res = self.get_gate_chain(start_gate, input_pins, output_pins, filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while detecting gate chain:\n{}", res.get_error().get());
                    return {};
                }
            },
            py::arg("start_gate"),
            py::arg("input_pins")  = std::vector<GatePin*>(),
            py::arg("output_pins") = std::vector<GatePin*>(),
            py::arg("filter")      = nullptr,
            borrowed(),
            R"(
            Find a sequence of identical gates that are connected via the specified input and output pins.
            The start gate may be any gate within a such a sequence, it is not required to be the first or the last gate.
            If input and/or output pins are specified, the gates must be connected through one of the input pins and/or one of the output pins.
            The optional filter is evaluated on every gate such that the result only contains gates matching the specified condition.

            :param hal_py.Gate start_gate: The gate at which to start the chain detection.
            :param list[hal_py.GatePin] input_pins: The input pins through which the gates must be connected. Defaults to an empty list.
            :param set[hal_py.GatePin] output_pins: The output pins through which the gates must be connected. Defaults to an empty list.
            :param lambda filter: An optional filter function to be evaluated on each gate.
            :returns: A list of gates that form a chain on success, an empty list on error.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_traversal_decorator.def(
            "get_complex_gate_chain",
            [](NetlistTraversalDecorator& self, Gate* start_gate,
               const std::vector<GateType*>& chain_types,
               const std::map<GateType*, std::vector<const GatePin*>>& input_pins,
               const std::map<GateType*, std::vector<const GatePin*>>& output_pins,
               const std::function<bool(const Gate*)>& filter = nullptr) -> std::vector<Gate*> {
                auto res = self.get_complex_gate_chain(start_gate, chain_types, input_pins, output_pins, filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while detecting complex gate chain:\n{}", res.get_error().get());
                    return {};
                }
            },
            py::arg("start_gate"),
            py::arg("chain_types"),
            py::arg("input_pins"),
            py::arg("output_pins"),
            py::arg("filter") = nullptr,
            borrowed(),
            R"(
            Find a sequence of gates (of the specified sequence of gate types) that are connected via the specified input and output pins.
            The start gate may be any gate within a such a sequence, it is not required to be the first or the last gate.
            However, the start gate must be of the first gate type within the repeating sequence.
            If input and/or output pins are specified for a gate type, the gates must be connected through one of the input pins and/or one of the output pins.
            The optional filter is evaluated on every gate such that the result only contains gates matching the specified condition.

            :param hal_py.Gate start_gate: The gate at which to start the chain detection.
            :param list[hal_py.GateType] chain_types: The sequence of gate types that is expected to make up the gate chain.
            :param dict[hal_py.GateType,set[str]] input_pins: The input pins (of every gate type of the sequence) through which the gates must be connected.
            :param dict[hal_py.GateType,set[str]] output_pins: The output pins (of every gate type of the sequence) through which the gates must be connected.
            :param lambda filter: An optional filter function to be evaluated on each gate.
            :returns: A list of gates that form a chain on success, an empty list on error.
            :rtype: list[hal_py.Gate]
        )");

    }
}    // namespace hal