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
 * @file grouping.h
 * @brief This file contains the class that holds all information of a dataflow analysis grouping.
 */

#pragma once

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/defines.h"

#include <map>
#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hal
{
    namespace dataflow
    {
        /**
         * @class Grouping
         * @brief Grouping used during dataflow analysis.
         * 
         * This class provides access to all information related to a grouping as an (intermediate) product of dataflow analysis.
         * Each grouping is a collection of groups of, e.g., flip-flops that result from different paths that have been executed on a previous grouping.
         */
        struct Grouping
        {
            /**
             * @brief Construct a new (empty) grouping from a netlist abstraction.
             * 
             * @param[in] na - The netlist abstraction.
             */
            Grouping(const NetlistAbstraction& na);

            /**
             * @brief Construct a new grouping from a netlist abstraction and a vector of groups (as a vector of gates).
             * 
             * @param[in] na - The netlist abstraction.
             * @param[in] groups - A vector of groups.
             */
            Grouping(const NetlistAbstraction& na, const std::vector<std::vector<Gate*>>& groups);

            /**
             * @brief Construct a new grouping as a copy of an existing grouping. 
             */
            Grouping(const Grouping& other);

            /**
             * The netlist abstraction associated with the grouping.
             */
            const NetlistAbstraction& netlist_abstr;

            /**
             * A map from group the an ordered list of control net IDs.
             */
            std::unordered_map<u32, std::vector<u32>> group_control_fingerprint_map;

            /**
             * A map from group to all gates contained in the group.
             */
            std::unordered_map<u32, std::unordered_set<u32>> gates_of_group;

            /**
             * A map from gate ID to the group that this gate belongs to.
             */
            std::unordered_map<u32, u32> parent_group_of_gate;

            /**
             * A map from a group to a flag that determines whether operations on the group are allowed.
             */
            std::map<u32, bool> operations_on_group_allowed;

            /**
             * @brief Check two groupings for equality.
             * 
             * Two groupings are equal if the comprise the same number of groups and if these groups are made up from the same gates. 
             * 
             * @param[in] other - The other grouping.
             * @return `true` if the groupings are equal, `false` otherwise.
             */
            bool operator==(const Grouping& other) const;

            /**
             * @brief Check two groupings for inequality.
             * 
             * Two groupings are equal if the comprise the same number of groups and if these groups are made up from the same gates. 
             * 
             * @param[in] other - The other grouping.
             * @return `true` if the groupings are unequal, `false` otherwise.
             */
            bool operator!=(const Grouping& other) const;

            /**
             * @brief Get the control signals of a group as a map from the control pin type to the connected net IDs.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns The group's control signals.
             */
            std::map<PinType, std::unordered_set<u32>> get_control_signals_of_group(u32 group_id) const;

            /** 
             * @brief Get the successor groups of a group.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns The group's successor groups.
             */
            std::unordered_set<u32> get_successor_groups_of_group(u32 group_id) const;

            /** 
             * @brief Get the predecessor groups of a group.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns The group's predecessor groups.
             */
            std::unordered_set<u32> get_predecessor_groups_of_group(u32 group_id) const;

            /**
             * @brief Get the known successor groups of a group.
             * 
             * The known groups have initially been fed to the dataflow analysis by the user.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns The group's known successor groups.
             */
            std::unordered_set<u32> get_known_successor_groups_of_group(u32 group_id) const;

            /**
             * @brief Get the known predecessor groups of a group.
             * 
             * The known groups have initially been fed to the dataflow analysis by the user.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns The group's known predecessor groups.
             */
            std::unordered_set<u32> get_known_predecessor_groups_of_group(u32 group_id) const;

            /**
             * @brief Get the intersection of the register stages of all gates of the group.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns The register stage intersection of all gates of the group.
             */
            std::set<u32> get_register_stage_intersect_of_group(u32 group_id) const;

            /** 
             * @brief Check if two groups are allowed to be merged.
             * 
             * @param[in] group_1_id - The ID of the first group.
             * @param[in] group_2_id - The ID of the second group.
             * @param[in] enforce_type_consistency - Set `true` to enforce that the gates of both groups must be of the same type, `false` otherwise.
             * @returns `true` if the groups are allowed to be joined, `false` otherwise.
             */
            bool are_groups_allowed_to_merge(u32 group_1_id, u32 group_2_id, bool enforce_type_consistency) const;

            /** 
             * @brief Check if the group is allowed to be split.
             * 
             * @param[in] group_id - The ID of the group.
             * @returns `true` if the group is allowed to be split, `false` otherwise.
             */
            bool is_group_allowed_to_split(u32 group_id) const;

        private:
            /* caches */
            mutable struct
            {
                std::shared_mutex mutex;
                std::unordered_map<u32, std::unordered_set<u32>> suc_cache;
                std::unordered_map<u32, std::unordered_set<u32>> pred_cache;
                std::unordered_map<u32, std::unordered_set<u32>> suc_known_group_cache;
                std::unordered_map<u32, std::unordered_set<u32>> pred_known_group_cache;
                std::set<std::set<u32>> comparison_cache;
            } cache;

            const std::set<std::set<u32>>& get_comparison_data() const;

            std::unordered_set<u32> get_signals_of_group(u32 group_id, const std::unordered_map<u32, std::unordered_set<u32>>& signals) const;
        };
    }    // namespace dataflow
}    // namespace hal
