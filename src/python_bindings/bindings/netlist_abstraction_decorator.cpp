#include "hal_core/netlist/netlist.h"
#include "hal_core/python_bindings/python_bindings.h"
#include "hal_core/utilities/result.h"

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace py = pybind11;

namespace hal
{
    void netlist_abstraction_decorator_init(py::module& m)
    {
        // Binding NetlistAbstraction struct
        py::class_<NetlistAbstraction> py_netlist_abstraction(m, "NetlistAbstraction", R"(
            Represents a netlist abstraction operating on a subset of the netlist, abstracting away the connections between them.
        )");

        // Bind create function
        py_netlist_abstraction.def_static(
            "create",
            [](const Netlist* netlist,
               const std::vector<Gate*>& gates,
               bool include_all_netlist_gates,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::optional<NetlistAbstraction> {
                auto res = NetlistAbstraction::create(netlist, gates, include_all_netlist_gates, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error creating NetlistAbstraction:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("netlist"),
            py::arg("gates"),
            py::arg("include_all_netlist_gates") = false,
            py::arg("exit_endpoint_filter")      = nullptr,
            py::arg("entry_endpoint_filter")     = nullptr,
            R"(
            Creates a NetlistAbstraction from a set of gates.

            :param hal_py.Netlist netlist: The netlist to abstract.
            :param list[hal_py.Gate] gates: The gates to include in the abstraction.
            :param bool include_all_netlist_gates: If set, all gates in the netlist have edges to the nearest gates in the abstraction. Defaults to False.
            :param function exit_endpoint_filter: Condition to stop traversal on a fan-in/out endpoint. Defaults to None.
            :param function entry_endpoint_filter: Condition to stop traversal on a successor/predecessor endpoint. Defaults to None.
            :returns: The NetlistAbstraction on success, None otherwise.
            :rtype: NetlistAbstraction or None
            )");

        // Bind get_predecessors functions with overloads
        py_netlist_abstraction.def(
            "get_predecessors",
            [](const NetlistAbstraction& self, const Gate* gate) -> std::optional<std::vector<Endpoint*>> {
                auto res = self.get_predecessors(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting predecessors of gate:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Gets the predecessors of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get predecessors for.
            :returns: A list of predecessor endpoints on success, None otherwise.
            :rtype: list[hal_py.Endpoint] or None
            )");

        py_netlist_abstraction.def(
            "get_predecessors",
            [](const NetlistAbstraction& self, const Endpoint* endpoint) -> std::optional<std::vector<Endpoint*>> {
                auto res = self.get_predecessors(endpoint);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting predecessors of endpoint:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            R"(
            Gets the predecessors of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get predecessors for.
            :returns: A list of predecessor endpoints on success, None otherwise.
            :rtype: list[hal_py.Endpoint] or None
            )");

        // Bind get_unique_predecessors functions with overloads
        py_netlist_abstraction.def(
            "get_unique_predecessors",
            [](const NetlistAbstraction& self, const Gate* gate) -> std::optional<std::vector<Gate*>> {
                auto res = self.get_unique_predecessors(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting unique predecessors of gate:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Gets the unique predecessor gates of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get unique predecessors for.
            :returns: A list of unique predecessor gates on success, None otherwise.
            :rtype: list[hal_py.Gate] or None
            )");

        py_netlist_abstraction.def(
            "get_unique_predecessors",
            [](const NetlistAbstraction& self, const Endpoint* endpoint) -> std::optional<std::vector<Gate*>> {
                auto res = self.get_unique_predecessors(endpoint);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting unique predecessors of endpoint:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            R"(
            Gets the unique predecessor gates of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get unique predecessors for.
            :returns: A list of unique predecessor gates on success, None otherwise.
            :rtype: list[hal_py.Gate] or None
            )");

        // Bind get_successors functions with overloads
        py_netlist_abstraction.def(
            "get_successors",
            [](const NetlistAbstraction& self, const Gate* gate) -> std::optional<std::vector<Endpoint*>> {
                auto res = self.get_successors(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting successors of gate:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Gets the successors of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get successors for.
            :returns: A list of successor endpoints on success, None otherwise.
            :rtype: list[hal_py.Endpoint] or None
            )");

        py_netlist_abstraction.def(
            "get_successors",
            [](const NetlistAbstraction& self, const Endpoint* endpoint) -> std::optional<std::vector<Endpoint*>> {
                auto res = self.get_successors(endpoint);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting successors of endpoint:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            R"(
            Gets the successors of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get successors for.
            :returns: A list of successor endpoints on success, None otherwise.
            :rtype: list[hal_py.Endpoint] or None
            )");

        // Bind get_unique_successors functions with overloads
        py_netlist_abstraction.def(
            "get_unique_successors",
            [](const NetlistAbstraction& self, const Gate* gate) -> std::optional<std::vector<Gate*>> {
                auto res = self.get_unique_successors(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting unique successors of gate:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Gets the unique successor gates of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get unique successors for.
            :returns: A list of unique successor gates on success, None otherwise.
            :rtype: list[hal_py.Gate] or None
            )");

        py_netlist_abstraction.def(
            "get_unique_successors",
            [](const NetlistAbstraction& self, const Endpoint* endpoint) -> std::optional<std::vector<Gate*>> {
                auto res = self.get_unique_successors(endpoint);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting unique successors of endpoint:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            R"(
            Gets the unique successor gates of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get unique successors for.
            :returns: A list of unique successor gates on success, None otherwise.
            :rtype: list[hal_py.Gate] or None
            )");

        // Bind get_global_input_predecessors function
        py_netlist_abstraction.def(
            "get_global_input_predecessors",
            [](const NetlistAbstraction& self, const Endpoint* endpoint) -> std::optional<std::vector<Net*>> {
                auto res = self.get_global_input_predecessors(endpoint);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting global input predecessors of endpoint:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            R"(
            Gets the global input nets that are predecessors of an endpoint.

            :param hal_py.Endpoint endpoint: The endpoint to get global input predecessors for.
            :returns: A list of global input nets on success, None otherwise.
            :rtype: list[hal_py.Net] or None
            )");

        // Bind get_global_output_successors function
        py_netlist_abstraction.def(
            "get_global_output_successors",
            [](const NetlistAbstraction& self, const Endpoint* endpoint) -> std::optional<std::vector<Net*>> {
                auto res = self.get_global_output_successors(endpoint);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error getting global output successors of endpoint:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            R"(
            Gets the global output nets that are successors of an endpoint.

            :param hal_py.Endpoint endpoint: The endpoint to get global output successors for.
            :returns: A list of global output nets on success, None otherwise.
            :rtype: list[hal_py.Net] or None
            )");

        // Binding NetlistAbstractionDecorator class
        py::class_<NetlistAbstractionDecorator> py_netlist_abstraction_decorator(m, "NetlistAbstractionDecorator", R"(
            A netlist decorator that provides additional methods for analyzing netlist abstractions.
        )");

        py_netlist_abstraction_decorator.def(py::init<const NetlistAbstraction&>(),
                                             py::arg("abstraction"),
                                             R"(
            Constructs a new `NetlistAbstractionDecorator` object.

            :param hal_py.NetlistAbstraction abstraction: The netlist abstraction to operate on.
        )");

        // Bind the first overloaded get_shortest_path_distance method
        py_netlist_abstraction_decorator.def(
            "get_shortest_path_distance",
            [](const NetlistAbstractionDecorator& self,
               Endpoint* start,
               const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
               const PinDirection& direction,
               const bool directed                                                          = true,
               const std::function<bool(const Endpoint*, const u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32)>& entry_endpoint_filter = nullptr) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_filter, direction, directed, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to get shortest path distance: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_filter"),
            py::arg("direction"),
            py::arg("directed")              = true,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Finds the length of the shortest path connecting the start endpoint to a target matching the given filter.
                If there is no such path, an empty optional is returned.
                Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.

                :param hal_py.Endpoint start: The starting endpoint.
                :param callable target_filter: A filter function to determine the target endpoints.
                :param hal_py.PinDirection direction: The direction to search in (`PinDirection::input`, `PinDirection::output`, or `PinDirection::inout`).
                :param bool directed: Defines whether we are searching on a directed or undirected graph representation of the netlist. Defaults to `True`.
                :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
                :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
                :returns: The shortest distance.
                :rtype: int or None
            )");

        // Bind the second overloaded get_shortest_path_distance method
        py_netlist_abstraction_decorator.def(
            "get_shortest_path_distance",
            [](const NetlistAbstractionDecorator& self,
               const Gate* start,
               const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
               const PinDirection& direction,
               const bool directed                                                          = true,
               const std::function<bool(const Endpoint*, const u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32)>& entry_endpoint_filter = nullptr) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_filter, direction, directed, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to get shortest path distance: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_filter"),
            py::arg("direction"),
            py::arg("directed")              = true,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Finds the length of the shortest path connecting the start gate to a target matching the given filter.
                If there is no such path, an empty optional is returned.
                Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.

                :param hal_py.Gate start: The starting gate.
                :param callable target_filter: A filter function to determine the target endpoints.
                :param hal_py.PinDirection direction: The direction to search in (`PinDirection::input`, `PinDirection::output`, or `PinDirection::inout`).
                :param bool directed: Defines whether we are searching on a directed or undirected graph representation of the netlist. Defaults to `True`.
                :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
                :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
                :returns: The shortest distance.
                :rtype: int or None
            )");

        // Bind the third overloaded get_shortest_path_distance method
        py_netlist_abstraction_decorator.def(
            "get_shortest_path_distance",
            [](const NetlistAbstractionDecorator& self,
               const Gate* start,
               const Gate* target_gate,
               const PinDirection& direction,
               const bool directed                                                          = true,
               const std::function<bool(const Endpoint*, const u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32)>& entry_endpoint_filter = nullptr) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_gate, direction, directed, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to get shortest path distance: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_gate"),
            py::arg("direction"),
            py::arg("directed")              = true,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Finds the length of the shortest path connecting the start gate to a target matching the given filter.
                If there is no such path, an empty optional is returned.
                Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.

                :param hal_py.Gate start: The starting gate.
                :param hal_py.Gate target_gate: The target gate.
                :param hal_py.PinDirection direction: The direction to search in (`PinDirection::input`, `PinDirection::output`, or `PinDirection::inout`).
                :param bool directed: Defines whether we are searching on a directed or undirected graph representation of the netlist. Defaults to `True`.
                :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
                :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
                :returns: The shortest distance.
                :rtype: int or None
            )");

        // Bind the first overloaded get_next_matching_gates method
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates",
            [](const NetlistAbstractionDecorator& self,
               Endpoint* endpoint,
               const std::function<bool(const Gate*)>& target_gate_filter,
               const PinDirection& direction,
               const bool directed                                                          = false,
               bool continue_on_match                                                       = false,
               const std::function<bool(const Endpoint*, const u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates(endpoint, target_gate_filter, direction, directed, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to get next matching gates: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            py::arg("target_gate_filter"),
            py::arg("direction"),
            py::arg("directed")              = false,
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Starting from the given endpoint, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
                Traverse over gates that do not meet the `target_gate_filter` condition.
                Stop traversal if (1) `continue_on_match` is `false` and the `target_gate_filter` evaluates to `true`, 
                (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or 
                (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
                Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.

                :param hal_py.Endpoint endpoint: The starting endpoint.
                :param callable target_gate_filter: Filter condition that must be met for the target gates.
                :param hal_py.PinDirection direction: The direction to search in (`PinDirection::input` or `PinDirection::output`).
                :param bool directed: Defines whether we are searching on a directed or undirected graph representation of the netlist. Defaults to `False`.
                :param bool continue_on_match: Set `true` to continue even if `target_gate_filter` evaluates to `true`, `false` otherwise. Defaults to `False`.
                :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
                :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
                :returns: A set of gates fulfilling the `target_gate_filter` condition.
                :rtype: set[hal_py.Gate] or None
            )");

        // Bind the second overloaded get_next_matching_gates method
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates",
            [](const NetlistAbstractionDecorator& self,
               const Gate* gate,
               const std::function<bool(const Gate*)>& target_gate_filter,
               const PinDirection& direction,
               const bool directed                                                          = false,
               bool continue_on_match                                                       = false,
               const std::function<bool(const Endpoint*, const u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates(gate, target_gate_filter, direction, directed, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to get next matching gates: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("target_gate_filter"),
            py::arg("direction"),
            py::arg("directed")              = false,
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Starting from the given gate, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
                Traverse over gates that do not meet the `target_gate_filter` condition.
                Stop traversal if (1) `continue_on_match` is `false` and the `target_gate_filter` evaluates to `true`, 
                (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or 
                (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
                Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.

                :param hal_py.Gate gate: The starting gate.
                :param callable target_gate_filter: Filter condition that must be met for the target gates.
                :param hal_py.PinDirection direction: The direction to search in (`PinDirection::input` or `PinDirection::output`).
                :param bool directed: Defines whether we are searching on a directed or undirected graph representation of the netlist. Defaults to `False`.
                :param bool continue_on_match: Set `true` to continue even if `target_gate_filter` evaluates to `true`, `false` otherwise. Defaults to `False`.
                :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
                :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
                :returns: A set of gates fulfilling the `target_gate_filter` condition.
                :rtype: set[hal_py.Gate] or None
            )");

        // Bind the first overloaded get_next_matching_gates_until method
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates_until",
            [](const NetlistAbstractionDecorator& self,
               Endpoint* endpoint,
               const std::function<bool(const Gate*)>& target_gate_filter,
               const PinDirection& direction,
               const bool directed                                                          = false,
               bool continue_on_mismatch                                                    = false,
               const std::function<bool(const Endpoint*, const u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until(endpoint, target_gate_filter, direction, directed, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to get next matching gates until: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("endpoint"),
            py::arg("target_gate_filter"),
            py::arg("direction"),
            py::arg("directed")              = false,
            py::arg("continue_on_mismatch")  = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Starting from the given endpoint, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
                Continue traversal regardless of whether `target_gate_filter` evaluates to `true` or `false`.
                Stop traversal if (1) `continue_on_mismatch` is `false` and the `target_gate_filter` evaluates to `false`, 
                (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or 
                (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
                Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.

                :param hal_py.Endpoint endpoint: The starting endpoint.
                :param callable target_gate_filter: Filter condition that must be met for the target gates.
                :param hal_py.PinDirection direction: The direction to search in (`PinDirection::input` or `PinDirection::output`).
                :param bool directed: Defines whether we are searching on a directed or undirected graph representation of the netlist. Defaults to `False`.
                :param bool continue_on_mismatch: Set `true` to continue even if `target_gate_filter` evaluates to `false`, `false` otherwise. Defaults to `False`.
                :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
                :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
                :returns: A set of gates fulfilling the `target_gate_filter` condition.
                :rtype: set[hal_py.Gate] or None
            )");

        // Bind the second overloaded get_next_matching_gates_until method
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates_until",
            [](const NetlistAbstractionDecorator& self,
               const Gate* gate,
               const std::function<bool(const Gate*)>& target_gate_filter,
               const PinDirection& direction,
               const bool directed                                                          = false,
               bool continue_on_mismatch                                                    = false,
               const std::function<bool(const Endpoint*, const u32)>& exit_endpoint_filter  = nullptr,
               const std::function<bool(const Endpoint*, const u32)>& entry_endpoint_filter = nullptr) -> std::optional<std::set<Gate*>> {
                auto res = self.get_next_matching_gates_until(gate, target_gate_filter, direction, directed, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Failed to get next matching gates until: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("target_gate_filter"),
            py::arg("direction"),
            py::arg("directed")              = false,
            py::arg("continue_on_mismatch")  = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
                Starting from the given gate, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
                Continue traversal regardless of whether `target_gate_filter` evaluates to `true` or `false`.
                Stop traversal if (1) `continue_on_mismatch` is `false` and the `target_gate_filter` evaluates to `false`, 
                (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or 
                (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
                Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.

                :param hal_py.Gate gate: The starting gate.
                :param callable target_gate_filter: Filter condition that must be met for the target gates.
                :param hal_py.PinDirection direction: The direction to search in (`PinDirection::input` or `PinDirection::output`).
                :param bool directed: Defines whether we are searching on a directed or undirected graph representation of the netlist. Defaults to `False`.
                :param bool continue_on_mismatch: Set `true` to continue even if `target_gate_filter` evaluates to `false`, `false` otherwise. Defaults to `False`.
                :param callable exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
                :param callable entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
                :returns: A set of gates fulfilling the `target_gate_filter` condition.
                :rtype: set[hal_py.Gate] or None
            )");
    }
}    // namespace hal
