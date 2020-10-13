#pragma once

#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct Grouping;

        namespace merge_successor_predecessor_groupings
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool delete_from_smaller);

        }    // namespace merge_successor_predecessor_groupings
    }        // namespace dataflow
}