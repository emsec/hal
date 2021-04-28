//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/defines.h"

#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hal
{
    namespace dataflow
    {
        struct Grouping
        {
            Grouping(const NetlistAbstraction& na);

            Grouping(const Grouping& other);

            const NetlistAbstraction& netlist_abstr;

            std::unordered_map<u32, std::vector<u32>> group_control_fingerprint_map;

            std::unordered_map<u32, std::unordered_set<u32>> gates_of_group;
            std::unordered_map<u32, u32> parent_group_of_gate;

            bool operator==(const Grouping& other) const;
            bool operator!=(const Grouping& other) const;

            std::unordered_set<u32> get_clock_signals_of_group(u32 group_id);
            std::unordered_set<u32> get_control_signals_of_group(u32 group_id);
            std::unordered_set<u32> get_reset_signals_of_group(u32 group_id);
            std::unordered_set<u32> get_set_signals_of_group(u32 group_id);

            std::unordered_set<u32> get_successor_groups_of_group(u32 group_id);
            std::unordered_set<u32> get_predecessor_groups_of_group(u32 group_id);

            std::set<u32> get_register_stage_intersect_of_group(u32 group_id);

            bool are_groups_allowed_to_merge(u32 group_1_id, u32 group_2_id);

        private:
            /* caches */
            mutable struct
            {
                std::shared_mutex mutex;
                std::unordered_map<u32, std::unordered_set<u32>> suc_cache;
                std::unordered_map<u32, std::unordered_set<u32>> pred_cache;
                std::set<std::set<u32>> comparison_cache;
            } cache;

            const std::set<std::set<u32>>& get_comparison_data() const;

            std::unordered_set<u32> get_signals_of_group(u32 group_id, const std::unordered_map<u32, std::unordered_set<u32>>& signals);
        };
    }    // namespace dataflow
}    // namespace hal
