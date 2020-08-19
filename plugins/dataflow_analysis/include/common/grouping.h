#pragma once

#include "common/netlist_abstraction.h"
#include "def.h"

#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hal
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
}    // namespace hal
