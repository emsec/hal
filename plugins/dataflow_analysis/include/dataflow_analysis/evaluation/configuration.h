#pragma once

#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        namespace evaluation
        {
            struct Configuration
            {
                std::vector<u32> prioritized_sizes;
            };

        }    // namespace evaluation
    }        // namespace dataflow
}