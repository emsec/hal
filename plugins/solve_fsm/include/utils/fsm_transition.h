#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/net.h"

#include <iostream>
#include <map>

#include "z3++.h"

namespace hal {
    struct FsmTransition
    {
        FsmTransition(const z3::expr& start, const z3::expr& end, const std::map<u32, u8>& inputs);

        FsmTransition merge(const FsmTransition& other) const;
        std::string to_string() const;
        std::string to_dot_string(const Netlist* nl) const;

        z3::expr starting_state_expr;
        z3::expr end_state_expr;
        u64 starting_state;
        u64 end_state;

        // vector of all input mappings that lead to this transition
        std::vector<std::map<u32, u8>> input_ids_to_values;
    };
}  // namespace hal
