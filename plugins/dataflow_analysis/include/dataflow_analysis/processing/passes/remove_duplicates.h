#pragma once

#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct Grouping;

        namespace remove_duplicates
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool delete_from_smaller);

        }    // namespace remove_duplicates
    }        // namespace dataflow
}