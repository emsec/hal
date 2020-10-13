#pragma once

#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct Grouping;

        namespace split_by_successors_predecessors
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool successors);

        }    // namespace split_by_successors_predecessors
    }        // namespace dataflow
}