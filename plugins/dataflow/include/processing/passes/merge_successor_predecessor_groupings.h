#pragma once

#include "def.h"

namespace hal
{
    /* forward declaration */
    struct Grouping;

    namespace merge_successor_predecessor_groupings
    {
        std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool delete_from_smaller);

    }    // namespace merge_successor_predecessor_groupings
}    // namespace hal