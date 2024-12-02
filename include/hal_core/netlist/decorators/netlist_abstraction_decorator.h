// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
 * @file netlist_abstraction_decorator.h
 * @brief This file contains the declarations for the NetlistAbstraction struct and the NetlistAbstractionDecorator class. 
*/
#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/result.h"

#include <optional>

namespace hal
{
    /**
     * @struct NetlistAbstraction
     * @brief Represents a netlist abstraction operating on a subset of the netlist, abstracting away the connections between them.
     *
     * This struct holds an abstraction of a netlist by focusing on a subset of gates and their connections.
     * It provides methods to retrieve predecessors and successors within the abstraction.
     */
    struct NETLIST_API NetlistAbstraction
    {
    public:
        NetlistAbstraction(NetlistAbstraction&& other) = default;

        /**
         * @brief Creates a `NetlistAbstraction` from a set of gates.
         *
         * @param[in] netlist - The netlist to abstract.
         * @param[in] gates - The gates to include in the abstraction.
         * @param[in] include_all_netlist_gates - If this flag is set, for all gates in the netlist, edges are created to the nearest gates that are part of the abstraction, otherwise this is only done for gates part of the abstraction.
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         */
        static Result<std::shared_ptr<NetlistAbstraction>> create(const Netlist* netlist,
                                                                  const std::vector<Gate*>& gates,
                                                                  const bool include_all_netlist_gates                                                       = false,
                                                                  const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                                  const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr);

        /**
         * @brief Gets the predecessors of a gate within the abstraction.
         *
         * @param[in] gate - The gate to get predecessors for.
         * @returns A vector of predecessor endpoints.
         */
        Result<std::vector<Endpoint*>> get_predecessors(const Gate* gate) const;

        /**
         * @brief Gets the predecessors of an endpoint within the abstraction.
         *
         * @param[in] endpoint - The endpoint to get predecessors for.
         * @returns A vector of predecessor endpoints.
         */
        Result<std::vector<Endpoint*>> get_predecessors(const Endpoint* endpoint) const;

        /**
         * @brief Gets the unique predecessor gates of a gate within the abstraction.
         *
         * @param[in] gate - The gate to get unique predecessors for.
         * @returns A vector of unique predecessor gates.
         */
        Result<std::vector<Gate*>> get_unique_predecessors(const Gate* gate) const;

        /**
         * @brief Gets the unique predecessor gates of an endpoint within the abstraction.
         *
         * @param[in] endpoint - The endpoint to get unique predecessors for.
         * @returns A vector of unique predecessor gates.
         */
        Result<std::vector<Gate*>> get_unique_predecessors(const Endpoint* endpoint) const;

        /**
         * @brief Gets the successors of a gate within the abstraction.
         *
         * @param[in] gate - The gate to get successors for.
         * @returns A vector of successor endpoints.
         */
        Result<std::vector<Endpoint*>> get_successors(const Gate* gate) const;

        /**
         * @brief Gets the successors of an endpoint within the abstraction.
         *
         * @param[in] endpoint - The endpoint to get successors for.
         * @returns A vector of successor endpoints.
         */
        Result<std::vector<Endpoint*>> get_successors(const Endpoint* endpoint) const;

        /**
         * @brief Gets the unique successor gates of a gate within the abstraction.
         *
         * @param[in] gate - The gate to get unique successors for.
         * @returns A vector of unique successor gates.
         */
        Result<std::vector<Gate*>> get_unique_successors(const Gate* gate) const;

        /**
         * @brief Gets the unique successor gates of an endpoint within the abstraction.
         *
         * @param[in] endpoint - The endpoint to get unique successors for.
         * @returns A vector of unique successor gates.
         */
        Result<std::vector<Gate*>> get_unique_successors(const Endpoint* endpoint) const;

        /**
         * @brief Gets the global input nets that are predecessors of an endpoint.
         *
         * @param[in] endpoint - The endpoint to get global input predecessors for.
         * @returns A vector of global input nets.
         */
        Result<std::vector<Net*>> get_global_input_predecessors(const Endpoint* endpoint) const;

        /**
         * @brief Gets the global output nets that are successors of an endpoint.
         *
         * @param[in] endpoint - The endpoint to get global output successors for.
         * @returns A vector of global output nets.
         */
        Result<std::vector<Net*>> get_global_output_successors(const Endpoint* endpoint) const;

    private:
        NetlistAbstraction() = default;

        /**
         * @brief Maps endpoints to their successor endpoints within the abstraction.
         */
        std::unordered_map<const Endpoint*, std::vector<Endpoint*>> m_successors;

        /**
         * @brief Maps endpoints to their predecessor endpoints within the abstraction.
         */
        std::unordered_map<const Endpoint*, std::vector<Endpoint*>> m_predecessors;

        /**
         * @brief Maps endpoints to their global output successor nets.
         */
        std::unordered_map<const Endpoint*, std::vector<Net*>> m_global_output_successors;

        /**
         * @brief Maps endpoints to their global input predecessor nets.
         */
        std::unordered_map<const Endpoint*, std::vector<Net*>> m_global_input_predecessors;
    };

    /**
     * @class NetlistAbstractionDecorator
     * @brief A netlist decorator that provides additional methods for analyzing netlist abstractions.
     *
     * This class operates on a `NetlistAbstraction` and provides methods such as finding the shortest path distance between gates or endpoints.
     */
    class NETLIST_API NetlistAbstractionDecorator
    {
    public:
        /**
         * @brief Constructs a new `NetlistAbstractionDecorator` object.
         * 
         * @param[in] abstraction - The netlist abstraction to operate on.
         */
        NetlistAbstractionDecorator(const NetlistAbstraction& abstraction);

        /**
         * @brief Finds the length of the shortest path connecting the start endpoint to a target matching the given filter.
         * If there is no such path, an empty optional is returned.
         * Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.
         *
         * @param[in] start - The starting endpoint.
         * @param[in] target_filter - A filter function to determine the target endpoints.
         * @param[in] direction - The direction to search in (`PinDirection::input`, `PinDirection::output`, or `PinDirection::inout`).
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and an optional unsigned integer representing the shortest distance on success, an error otherwise.
         */
        Result<std::optional<u32>> get_shortest_path_distance(Endpoint* start,
                                                              const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                              const PinDirection& direction,
                                                              const bool directed                                                                        = true,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Finds the length of the shortest path connecting the start gate to a target matching the given filter.
         * If there is no such path, an empty optional is returned.
         * Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.
         *
         * @param[in] start - The starting gate.
         * @param[in] target_filter - A filter function to determine the target endpoints.
         * @param[in] direction - The direction to search in (`PinDirection::input`, `PinDirection::output`, or `PinDirection::inout`).
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and an optional unsigned integer representing the shortest distance on success, an error otherwise.
         */
        Result<std::optional<u32>> get_shortest_path_distance(const Gate* start,
                                                              const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                              const PinDirection& direction,
                                                              const bool directed                                                                        = true,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Finds the length of the shortest path connecting the start gate to a target matching the given filter.
         * If there is no such path, an empty optional is returned.
         * Computing only the shortest distance is faster than computing the actual path, as it does not keep track of the path to each gate.
         *
         * @param[in] start - The starting gate.
         * @param[in] target_gate - The target gate.
         * @param[in] direction - The direction to search in (`PinDirection::input`, `PinDirection::output`, or `PinDirection::inout`).
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and an optional unsigned integer representing the shortest distance on success, an error otherwise.
         */
        Result<std::optional<u32>> get_shortest_path_distance(const Gate* start,
                                                              const Gate* target_gate,
                                                              const PinDirection& direction,
                                                              const bool directed                                                                        = true,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Starting from the given endpoint, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Traverse over gates that do not meet the `target_gate_filter` condition.
         * Stop traversal if (1) `continue_on_match` is `false` and the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] endpoint - The starting endpoint.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] direction - The direction to search in (`PinDirection::input` or `PinDirection::output`).
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] continue_on_match - Set `true` to continue even if `target_gate_filter` evaluates to `true`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and a set of gates fulfilling the `target_gate_filter` condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates(Endpoint* endpoint,
                                                        const std::function<bool(const Gate*)>& target_gate_filter,
                                                        const PinDirection& direction,
                                                        const bool directed                                                                        = true,
                                                        bool continue_on_match                                                                     = false,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Starting from the given gate, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Traverse over gates that do not meet the `target_gate_filter` condition.
         * Stop traversal if (1) `continue_on_match` is `false` and the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] gate - The starting gate.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] direction - The direction to search in (`PinDirection::input` or `PinDirection::output`).
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] continue_on_match - Set `true` to continue even if `target_gate_filter` evaluates to `true`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and a set of gates fulfilling the `target_gate_filter` condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates(const Gate* gate,
                                                        const std::function<bool(const Gate*)>& target_gate_filter,
                                                        const PinDirection& direction,
                                                        const bool directed                                                                        = true,
                                                        bool continue_on_match                                                                     = false,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Starting from the given endpoint, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Continue traversal regardless of whether `target_gate_filter` evaluates to `true` or `false`.
         * Stop traversal if (1) `continue_on_mismatch` is `false` and the `target_gate_filter` evaluates to `false`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
         * Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] endpoint - The starting endpoint.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] direction - The direction to search in (`PinDirection::input` or `PinDirection::output`).
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] continue_on_mismatch - Set `true` to continue even if `target_gate_filter` evaluates to `false`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and a set of gates fulfilling the `target_gate_filter` condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_until(Endpoint* endpoint,
                                                              const std::function<bool(const Gate*)>& target_gate_filter,
                                                              const PinDirection& direction,
                                                              const bool directed                                                                        = true,
                                                              bool continue_on_mismatch                                                                  = false,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Starting from the given gate, traverse the netlist abstraction and return the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Continue traversal regardless of whether `target_gate_filter` evaluates to `true` or `false`.
         * Stop traversal if (1) `continue_on_mismatch` is `false` and the `target_gate_filter` evaluates to `false`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint, or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint.
         * Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] gate - The starting gate.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] direction - The direction to search in (`PinDirection::input` or `PinDirection::output`).
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] continue_on_mismatch - Set `true` to continue even if `target_gate_filter` evaluates to `false`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and a set of gates fulfilling the `target_gate_filter` condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_until(const Gate* gate,
                                                              const std::function<bool(const Gate*)>& target_gate_filter,
                                                              const PinDirection& direction,
                                                              const bool directed                                                                        = true,
                                                              bool continue_on_mismatch                                                                  = false,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

    private:
        /**
         * @brief Internal method to find the shortest path distance from a set of starting endpoints.
         *
         * @param[in] start - The starting endpoints.
         * @param[in] target_filter - A filter function to determine the target endpoints.
         * @param[in] direction - The direction to search in.
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and an optional unsigned integer representing the shortest distance on success, an error otherwise.
         */
        Result<std::optional<u32>> get_shortest_path_distance_internal(const std::vector<Endpoint*>& start,
                                                                       const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                                       const PinDirection& direction,
                                                                       const bool directed                                                                  = true,
                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Internal method to traverse the netlist abstraction and return matching gates based on the provided filters.
         *
         * @param[in] start - The starting endpoints.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] direction - The direction to search in.
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] continue_on_match - Determines whether to continue traversal after a match.
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and a set of gates fulfilling the `target_gate_filter` condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_internal(const std::vector<Endpoint*>& start,
                                                                 const std::function<bool(const Gate*)>& target_gate_filter,
                                                                 const PinDirection& direction,
                                                                 const bool directed                                                                  = true,
                                                                 bool continue_on_match                                                               = false,
                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief Internal method to traverse the netlist abstraction and return matching gates until certain conditions are met.
         *
         * @param[in] start - The starting endpoints.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] direction - The direction to search in.
         * @param[in] directed - Defines whether we are searching on a directed or undirected graph represenation of the netlist. 
         * @param[in] continue_on_mismatch - Determines whether to continue traversal after a mismatch.
         * @param[in] exit_endpoint_filter - Filter condition to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition to stop traversal on a successor/predecessor endpoint.
         * @returns OK() and a set of gates fulfilling the `target_gate_filter` condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_until_internal(const std::vector<Endpoint*>& start,
                                                                       const std::function<bool(const Gate*)>& target_gate_filter,
                                                                       const PinDirection& direction,
                                                                       const bool directed                                                                        = true,
                                                                       bool continue_on_mismatch                                                                  = false,
                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * @brief The netlist abstraction to operate on.
         */
        const NetlistAbstraction& m_abstraction;
    };

}    // namespace hal