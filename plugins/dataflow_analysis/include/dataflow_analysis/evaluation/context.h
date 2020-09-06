#pragma once

#include "dataflow_analysis/common/grouping.h"
#include "hal_core/defines.h"

namespace hal
{
    namespace evaluation
    {
        struct Context
        {
            Context(){
                // phase   = 0;
            };

            // u32 phase;
            // std::shared_ptr<grouping> control_state;
            std::vector<std::shared_ptr<Grouping>> partial_results;
        };

    }    // namespace evaluation
}    // namespace hal
