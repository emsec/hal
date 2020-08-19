#pragma once

#include "def.h"

namespace hal
{
    /* forward declaration */
    struct Grouping;

    namespace split_by_successors_predecessors
    {
        std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool successors);

    }    // namespace split_by_successors_predecessors
}    // namespace hal