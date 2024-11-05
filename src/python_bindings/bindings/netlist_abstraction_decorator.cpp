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

        py_netlist_abstraction.def(py::init<const Netlist*, const std::vector<Gate*>&, bool, const std::function<bool(const Endpoint*, u32)>&, const std::function<bool(const Endpoint*, u32)>&>(),
                                   py::arg("netlist"),
                                   py::arg("gates"),
                                   py::arg("include_all_netlist_gates") = false,
                                   py::arg("exit_endpoint_filter")      = nullptr,
                                   py::arg("entry_endpoint_filter")     = nullptr,
                                   R"(
            Constructs a `NetlistAbstraction` from a set of gates.

            :param hal_py.Netlist netlist: The netlist to abstract.
            :param list[hal_py.Gate] gates: The gates to include in the abstraction.
            :param bool include_all_netlist_gates: If this flag is set, for all gates in the netlist, edges are created to the nearest gates that are part of the abstraction, otherwise this is only done for gates part of the abstraction.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
        )");

        // Binding methods for NetlistAbstraction

        // get_predecessors for Gate*
        py_netlist_abstraction.def("get_predecessors",
                                   py::overload_cast<const Gate*>(&NetlistAbstraction::get_predecessors, py::const_),
                                   py::arg("gate"),
                                   R"(
            Gets the predecessors of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get predecessors for.
            :returns: A list of predecessor endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        // get_predecessors for Endpoint*
        py_netlist_abstraction.def("get_predecessors",
                                   py::overload_cast<const Endpoint*>(&NetlistAbstraction::get_predecessors, py::const_),
                                   py::arg("endpoint"),
                                   R"(
            Gets the predecessors of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get predecessors for.
            :returns: A list of predecessor endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        // get_unique_predecessors for Gate*
        py_netlist_abstraction.def("get_unique_predecessors",
                                   py::overload_cast<const Gate*>(&NetlistAbstraction::get_unique_predecessors, py::const_),
                                   py::arg("gate"),
                                   R"(
            Gets the unique predecessor gates of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get unique predecessors for.
            :returns: A list of unique predecessor gates.
            :rtype: list[hal_py.Gate]
        )");

        // get_unique_predecessors for Endpoint*
        py_netlist_abstraction.def("get_unique_predecessors",
                                   py::overload_cast<const Endpoint*>(&NetlistAbstraction::get_unique_predecessors, py::const_),
                                   py::arg("endpoint"),
                                   R"(
            Gets the unique predecessor gates of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get unique predecessors for.
            :returns: A list of unique predecessor gates.
            :rtype: list[hal_py.Gate]
        )");

        // get_successors for Gate*
        py_netlist_abstraction.def("get_successors",
                                   py::overload_cast<const Gate*>(&NetlistAbstraction::get_successors, py::const_),
                                   py::arg("gate"),
                                   R"(
            Gets the successors of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get successors for.
            :returns: A list of successor endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        // get_successors for Endpoint*
        py_netlist_abstraction.def("get_successors",
                                   py::overload_cast<const Endpoint*>(&NetlistAbstraction::get_successors, py::const_),
                                   py::arg("endpoint"),
                                   R"(
            Gets the successors of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get successors for.
            :returns: A list of successor endpoints.
            :rtype: list[hal_py.Endpoint]
        )");

        // get_unique_successors for Gate*
        py_netlist_abstraction.def("get_unique_successors",
                                   py::overload_cast<const Gate*>(&NetlistAbstraction::get_unique_successors, py::const_),
                                   py::arg("gate"),
                                   R"(
            Gets the unique successor gates of a gate within the abstraction.

            :param hal_py.Gate gate: The gate to get unique successors for.
            :returns: A list of unique successor gates.
            :rtype: list[hal_py.Gate]
        )");

        // get_unique_successors for Endpoint*
        py_netlist_abstraction.def("get_unique_successors",
                                   py::overload_cast<const Endpoint*>(&NetlistAbstraction::get_unique_successors, py::const_),
                                   py::arg("endpoint"),
                                   R"(
            Gets the unique successor gates of an endpoint within the abstraction.

            :param hal_py.Endpoint endpoint: The endpoint to get unique successors for.
            :returns: A list of unique successor gates.
            :rtype: list[hal_py.Gate]
        )");

        // get_global_input_predecessors
        py_netlist_abstraction.def("get_global_input_predecessors",
                                   &NetlistAbstraction::get_global_input_predecessors,
                                   py::arg("endpoint"),
                                   R"(
            Gets the global input nets that are predecessors of an endpoint.

            :param hal_py.Endpoint endpoint: The endpoint to get global input predecessors for.
            :returns: A list of global input nets.
            :rtype: list[hal_py.Net]
        )");

        // get_global_output_successors
        py_netlist_abstraction.def("get_global_output_successors",
                                   &NetlistAbstraction::get_global_output_successors,
                                   py::arg("endpoint"),
                                   R"(
            Gets the global output nets that are successors of an endpoint.

            :param hal_py.Endpoint endpoint: The endpoint to get global output successors for.
            :returns: A list of global output nets.
            :rtype: list[hal_py.Net]
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

        // get_shortest_path_distance with Endpoint*
        py_netlist_abstraction_decorator.def(
            "get_shortest_path_distance",
            [](const NetlistAbstractionDecorator& self,
               Endpoint* start,
               const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_filter, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in get_shortest_path_distance: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_filter"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Finds the length of the shortest path connecting the start endpoint to a target matching the given filter.
            If there is no such path, returns None.

            :param hal_py.Endpoint start: The starting endpoint.
            :param function target_filter: Filter function to determine the target endpoints.
            :param hal_py.PinDirection direction: The direction to search in.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: An optional integer representing the shortest distance on success.
            :rtype: int or None
        )");

        // get_shortest_path_distance with Gate*
        py_netlist_abstraction_decorator.def(
            "get_shortest_path_distance",
            [](const NetlistAbstractionDecorator& self,
               const Gate* start,
               const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_filter, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in get_shortest_path_distance: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_filter"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Finds the length of the shortest path connecting the start gate to a target matching the given filter.
            If there is no such path, returns None.

            :param hal_py.Gate start: The starting gate.
            :param function target_filter: Filter function to determine the target endpoints.
            :param hal_py.PinDirection direction: The direction to search in.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: An optional integer representing the shortest distance on success.
            :rtype: int or None
        )");

        // get_shortest_path_distance with Gate* and target Gate*
        py_netlist_abstraction_decorator.def(
            "get_shortest_path_distance",
            [](const NetlistAbstractionDecorator& self,
               const Gate* start,
               const Gate* target_gate,
               const PinDirection& direction,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::optional<u32> {
                auto res = self.get_shortest_path_distance(start, target_gate, direction, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in get_shortest_path_distance: {}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("start"),
            py::arg("target_gate"),
            py::arg("direction"),
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Finds the length of the shortest path connecting the start gate to the target gate.
            If there is no such path, returns None.

            :param hal_py.Gate start: The starting gate.
            :param hal_py.Gate target_gate: The target gate.
            :param hal_py.PinDirection direction: The direction to search in.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: An optional integer representing the shortest distance on success.
            :rtype: int or None
        )");

        // get_next_matching_gates with Endpoint*
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates",
            [](const NetlistAbstractionDecorator& self,
               Endpoint* endpoint,
               const PinDirection& direction,
               const std::function<bool(const Gate*)>& target_gate_filter,
               bool continue_on_match,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::set<Gate*> {
                auto res = self.get_next_matching_gates(endpoint, direction, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in get_next_matching_gates: {}", res.get_error().get());
                    return std::set<Gate*>();
                }
            },
            py::arg("endpoint"),
            py::arg("direction"),
            py::arg("target_gate_filter"),
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given endpoint, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `True`.

            :param hal_py.Endpoint endpoint: The starting endpoint.
            :param hal_py.PinDirection direction: The direction to search in.
            :param function target_gate_filter: Filter function that must be met for the target gates.
            :param bool continue_on_match: Set to True to continue even if `target_gate_filter` evaluates to True.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: A set of gates fulfilling the `target_gate_filter` condition.
            :rtype: set[hal_py.Gate]
        )");

        // get_next_matching_gates with Gate*
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates",
            [](const NetlistAbstractionDecorator& self,
               const Gate* gate,
               const PinDirection& direction,
               const std::function<bool(const Gate*)>& target_gate_filter,
               bool continue_on_match,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::set<Gate*> {
                auto res = self.get_next_matching_gates(gate, direction, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in get_next_matching_gates: {}", res.get_error().get());
                    return std::set<Gate*>();
                }
            },
            py::arg("gate"),
            py::arg("direction"),
            py::arg("target_gate_filter"),
            py::arg("continue_on_match")     = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `True`.

            :param hal_py.Gate gate: The starting gate.
            :param hal_py.PinDirection direction: The direction to search in.
            :param function target_gate_filter: Filter function that must be met for the target gates.
            :param bool continue_on_match: Set to True to continue even if `target_gate_filter` evaluates to True.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: A set of gates fulfilling the `target_gate_filter` condition.
            :rtype: set[hal_py.Gate]
        )");

        // get_next_matching_gates_until with Endpoint*
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates_until",
            [](const NetlistAbstractionDecorator& self,
               Endpoint* endpoint,
               const PinDirection& direction,
               const std::function<bool(const Gate*)>& target_gate_filter,
               bool continue_on_mismatch,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::set<Gate*> {
                auto res = self.get_next_matching_gates_until(endpoint, direction, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in get_next_matching_gates_until: {}", res.get_error().get());
                    return std::set<Gate*>();
                }
            },
            py::arg("endpoint"),
            py::arg("direction"),
            py::arg("target_gate_filter"),
            py::arg("continue_on_mismatch")  = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given endpoint, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `True`.
            Continue traversal regardless of whether `target_gate_filter` evaluates to `True` or `False`.

            :param hal_py.Endpoint endpoint: The starting endpoint.
            :param hal_py.PinDirection direction: The direction to search in.
            :param function target_gate_filter: Filter function that must be met for the target gates.
            :param bool continue_on_mismatch: Set to True to continue even if `target_gate_filter` evaluates to False.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: A set of gates fulfilling the `target_gate_filter` condition.
            :rtype: set[hal_py.Gate]
        )");

        // get_next_matching_gates_until with Gate*
        py_netlist_abstraction_decorator.def(
            "get_next_matching_gates_until",
            [](const NetlistAbstractionDecorator& self,
               const Gate* gate,
               const PinDirection& direction,
               const std::function<bool(const Gate*)>& target_gate_filter,
               bool continue_on_mismatch,
               const std::function<bool(const Endpoint*, u32)>& exit_endpoint_filter,
               const std::function<bool(const Endpoint*, u32)>& entry_endpoint_filter) -> std::set<Gate*> {
                auto res = self.get_next_matching_gates_until(gate, direction, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in get_next_matching_gates_until: {}", res.get_error().get());
                    return std::set<Gate*>();
                }
            },
            py::arg("gate"),
            py::arg("direction"),
            py::arg("target_gate_filter"),
            py::arg("continue_on_mismatch")  = false,
            py::arg("exit_endpoint_filter")  = nullptr,
            py::arg("entry_endpoint_filter") = nullptr,
            R"(
            Starting from the given gate, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `True`.
            Continue traversal regardless of whether `target_gate_filter` evaluates to `True` or `False`.

            :param hal_py.Gate gate: The starting gate.
            :param hal_py.PinDirection direction: The direction to search in.
            :param function target_gate_filter: Filter function that must be met for the target gates.
            :param bool continue_on_mismatch: Set to True to continue even if `target_gate_filter` evaluates to False.
            :param function exit_endpoint_filter: Filter condition to stop traversal on a fan-in/out endpoint.
            :param function entry_endpoint_filter: Filter condition to stop traversal on a successor/predecessor endpoint.
            :returns: A set of gates fulfilling the `target_gate_filter` condition.
            :rtype: set[hal_py.Gate]
        )");
    }    // namespace hal
