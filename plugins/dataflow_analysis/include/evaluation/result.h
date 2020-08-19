#pragma once

#include "common/grouping.h"

#include <unordered_map>

namespace hal
{
    namespace evaluation
    {
        struct Result
        {
            std::shared_ptr<Grouping> merged_result;
            bool is_final_result;
        };

    }    // namespace evaluation
}    // namespace hal