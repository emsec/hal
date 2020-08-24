#pragma once

#include "hal_core/defines.h"

#include <set>
#include <string>

namespace hal
{
    struct Grouping;
    class Netlist;
    class Gate;

    namespace state_to_module
    {
        bool create_modules(Netlist* nl, const std::shared_ptr<const Grouping>& state);
        std::set<std::set<Gate*>> create_sets(Netlist* nl, const std::shared_ptr<const Grouping>& state);

    }    // namespace state_to_module
}    // namespace hal
