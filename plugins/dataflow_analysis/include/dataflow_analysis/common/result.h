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
            hal::Result<std::unordered_set<Net*>> get_control_nets_of_group(const u32 group_id, const PinType type) const;

            /**
             * Get the successor groups of the group with the given ID.
             * 
             * @param[in] group_id - The group ID.
             * @returns Ok() and the successors of the group as a set of group IDs on success, an error otherwise.
             */
            hal::Result<std::unordered_set<u32>> get_group_successors(const u32 group_id) const;

            /**
             * Get the predecessor groups of the group with the given ID.
             * 
             * @param[in] group_id - The group ID.
             * @returns Ok() and the predecessors of the group as a set of group IDs on success, an error otherwise.
             */
            hal::Result<std::unordered_set<u32>> get_group_predecessors(const u32 group_id) const;

        private:
            Netlist* m_netlist;

            std::unordered_map<u32, std::unordered_set<Gate*>> m_gates_of_group;
            std::unordered_map<const Gate*, u32> m_parent_group_of_gate;

            std::unordered_map<u32, std::unordered_map<PinType, std::unordered_set<Net*>>> m_group_signals;

            std::unordered_map<u32, std::unordered_set<u32>> m_group_successors;
            std::unordered_map<u32, std::unordered_set<u32>> m_group_predecessors;
        };
    }    // namespace dataflow
}    // namespace hal