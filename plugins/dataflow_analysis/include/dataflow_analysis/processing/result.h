#pragma once

#include "dataflow_analysis/common/grouping.h"
#include "hal_core/defines.h"
#include "dataflow_analysis/processing/pass_collection.h"

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
