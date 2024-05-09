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
            py::arg("target_gate_filter"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
            Continues traversal independent of whatever ``target_gate_filter`` evaluates to.
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
            py::arg("target_gate_filter"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the ``target_gate_filter`` evaluates to ``True``.
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
            "get_next_sequential_gates",
            [](NetlistTraversalDecorator& self, const Net* net, bool successors, const std::set<PinType>& forbidden_input_pins = {}, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_sequential_gates(net, successors, forbidden_input_pins, cache);
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
            py::arg("forbidden_input_pins") = std::set<PinType>(),
            py::arg("cache")                = nullptr,
            R"(

            :param hal_py.Net net: Start net.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.

            :rtype: set[hal_py.Gate] or None
        )");

        py_netlist_traversal_decorator.def(
            "get_next_sequential_gates",
            [](NetlistTraversalDecorator& self, const Gate* gate, bool successors, const std::set<PinType>& forbidden_input_pins = {}, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr)
                -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_sequential_gates(gate, successors, forbidden_input_pins, cache);
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
            py::arg("forbidden_input_pins") = std::set<PinType>(),
            py::arg("cache")                = nullptr,
            R"(

            :param hal_py.Gate gate: Start gate.
            :param bool successors: Set ``True`` to get successors, set ``False`` to get predecessors.

            :rtype: set[hal_py.Gate] or None
        )");
    }
}    // namespace hal