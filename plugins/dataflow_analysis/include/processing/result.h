#pragma once

#include "common/grouping.h"
#include "def.h"
#include "processing/pass_collection.h"

#include <map>

namespace hal
{
    namespace processing
    {
        struct Result
        {
            std::vector<std::shared_ptr<Grouping>> unique_groupings;
            std::map<std::shared_ptr<Grouping>, std::vector<std::vector<pass_id>>> pass_combinations_leading_to_grouping;
            std::map<std::vector<pass_id>, std::shared_ptr<Grouping>> groupings;
        };

    }    // namespace processing
}    // namespace hal