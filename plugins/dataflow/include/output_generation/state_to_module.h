#pragma once

#include "def.h"

#include <set>
#include <string>

namespace hal
{
    struct Grouping;
    class Netlist;
    class Gate;

    namespace state_to_module
    {
        bool create_modules(const std::shared_ptr<Netlist>& nl, const std::shared_ptr<const Grouping>& state);
        std::set<std::set<std::shared_ptr<Gate>>> create_sets(const std::shared_ptr<Netlist>& nl, const std::shared_ptr<const Grouping>& state);

    }    // namespace state_to_module
}    // namespace hal
