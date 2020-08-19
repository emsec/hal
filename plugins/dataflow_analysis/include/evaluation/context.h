#pragma once

#include "common/grouping.h"
#include "def.h"

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