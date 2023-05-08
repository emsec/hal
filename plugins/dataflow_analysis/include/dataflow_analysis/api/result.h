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

#pragma once

#include "dataflow_analysis/common/grouping.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class Gate;
    class Net;
    class Module;

    namespace dataflow
    {
        /**
         * The result of a dataflow analysis run containing the identified groups of sequential gates and their interconnections.
         */
        struct Result
        {
            Result(Netlist* nl, const Grouping& grouping);

            /**
             * Get the netlist on which dataflow analysis has been performed.
             * 
             * @returns The netlist.
             */
            Netlist* get_netlist() const;

            /**
             * Get the groups of sequential gates resulting from dataflow analysis. 
             * 
             * @returns A map from group ID to a set of gates belonging to the respective group.
             */
            const std::unordered_map<u32, std::unordered_set<Gate*>>& get_groups() const;

            /**
             * Get all gates contained in any of the groups groups.
             * 
             * @returns A vector of gates.
             */
            std::vector<Gate*> get_gates() const;

            /**
             * Get the gates of the specified group of sequential gates.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns Ok() and the gates of the group as a set on success, an error otherwise.
             */
            hal::Result<std::unordered_set<Gate*>> get_gates_of_group(const u32 group_id) const;

            /**
             * Get the group ID of the group that contains the given gate. 
             * 
             * @param[in] gate - The gate.
             * @returns Ok() and the group ID on success, an error otherwise.
             */
            hal::Result<u32> get_group_id_of_gate(const Gate* gate) const;

            /**
             * Get the control nets of the group with the given group ID that are connected to a pin of the specified type.
             * 
             * @param[in] group_id - The group ID.
             * @param[in] type - The pin type.
             * @returns Ok() and a set of control nets of the group on success, an error otherwise.
            */
            hal::Result<std::unordered_set<Net*>> get_group_control_nets(const u32 group_id, const PinType type) const;

            /**
             * Get the control nets of the given gate that are connected to a pin of the specified type.
             * 
             * @param[in] gate - The gate.
             * @param[in] type - The pin type.
             * @returns Ok() and a set of control nets of the gate on success, an error otherwise.
            */
            hal::Result<std::unordered_set<Net*>> get_gate_control_nets(const Gate* gate, const PinType type) const;

            /**
             * Get the successor groups of the group with the given ID.
             * 
             * @param[in] group_id - The group ID.
             * @returns Ok() and the successors of the group as a set of group IDs on success, an error otherwise.
             */
            hal::Result<std::unordered_set<u32>> get_group_successors(const u32 group_id) const;

            /**
             * Get the sequential successor gates of the given sequential gate.
             * 
             * @param[in] gate - The gate.
             * @returns Ok() and the successors of the gate as a set of gates on success, an error otherwise.
             */
            hal::Result<std::unordered_set<Gate*>> get_gate_successors(const Gate* gate) const;

            /**
             * Get the predecessor groups of the group with the given ID.
             * 
             * @param[in] group_id - The group ID.
             * @returns Ok() and the predecessors of the group as a set of group IDs on success, an error otherwise.
             */
            hal::Result<std::unordered_set<u32>> get_group_predecessors(const u32 group_id) const;

            /**
             * Get the sequential predecessor gates of the given sequential gate.
             * 
             * @param[in] gate - The gate.
             * @returns Ok() and the predecessors of the gate as a set of gates on success, an error otherwise.
             */
            hal::Result<std::unordered_set<Gate*>> get_gate_predecessors(const Gate* gate) const;

            /**
             * Write the dataflow graph as a DOT graph to the specified location.
             * 
             * @param[in] out_path - The output path.
             * @param[in] group_ids - The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
             * @returns Ok() on success, an error otherwise.
             */
            hal::Result<std::monostate> write_dot(const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids = {}) const;

            /**
             * Write the groups resulting from dataflow analysis to a `.txt` file.
             * 
             * @param[in] out_path - The output path.
             * @param[in] group_ids - The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
             * @returns Ok() on success, an error otherwise.
             */
            hal::Result<std::monostate> write_txt(const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids = {}) const;

            /**
             * Create modules for the dataflow analysis result.
             * 
             * @param[in] group_ids - The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
             * @returns Ok() and a map from group IDs to Modules on success, an error otherwise.
             */
            hal::Result<std::unordered_map<u32, Module*>> create_modules(const std::unordered_set<u32>& group_ids = {}) const;

            /**
             * Get the groups of the dataflow analysis result as a list.
             * 
             * @param[in] group_ids - The group IDs to consider. If no IDs are provided, all groups will be considered. Defaults to an empty set.
             * @returns A vector of groups with each group being a vector of gates.
             */
            std::vector<std::vector<Gate*>> get_groups_as_list(const std::unordered_set<u32>& group_ids = {}) const;

            /**
             * Merge multiple groups specified by ID. 
             * All specified groups are merged into the first group of the provided vector and are subsequently deleted.
             * 
             * @param[in] group_ids - The group IDs of the groups to merge.
             * @returns The ID of the group that all over groups have been merged into on success, an error otherwise.
             */
            hal::Result<u32> merge_groups(const std::vector<u32>& group_ids);

            /**
             * Split a group into multiple smaller groups specified by sets of gates.
             * All gates of the group to split must be contained in the sets exactly once and all gates in the sets must be contained in the group to split.
             * The group that is being split is deleted in the process.
             * 
             * @param[in] group_id - The group ID of the group to split.
             * @param[in] new_groups - A vector of groups specified as unordered sets of gates.
             * @returns The group IDs of the newly created groups in the order of the provided sets.
             */
            hal::Result<std::vector<u32>> split_group(u32 group_id, const std::vector<std::unordered_set<Gate*>>& new_groups);

        private:
            Netlist* m_netlist;

            u32 m_last_id = 0;

            std::unordered_map<u32, std::unordered_set<Gate*>> m_gates_of_group;
            std::unordered_map<const Gate*, u32> m_parent_group_of_gate;

            // gate information
            std::unordered_map<const Gate*, std::unordered_map<PinType, std::unordered_set<Net*>>> m_gate_signals;
            std::unordered_map<const Gate*, std::unordered_set<Gate*>> m_gate_successors;
            std::unordered_map<const Gate*, std::unordered_set<Gate*>> m_gate_predecessors;

            // group information
            std::unordered_map<u32, std::unordered_map<PinType, std::unordered_set<Net*>>> m_group_signals;
            std::unordered_map<u32, std::unordered_set<u32>> m_group_successors;
            std::unordered_map<u32, std::unordered_set<u32>> m_group_predecessors;
        };
    }    // namespace dataflow
}    // namespace hal