#pragma once

#include "hal_core/defines.h"

namespace hal
{
    /* forward declaration */
    struct Grouping;

    namespace group_by_successors_predecessors
    {
        std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool successors);

    }    // namespace group_by_successors_predecessors
}    // namespace hal
