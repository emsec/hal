#pragma once

#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct Grouping;

        namespace merge_states
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<const Grouping>& state1, const std::shared_ptr<const Grouping>& state2, bool delete_from_smaller);

        }    // namespace merge_states
    }        // namespace dataflow
}