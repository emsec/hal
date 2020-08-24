#pragma once

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/defines.h"
#include "dataflow_analysis/processing/configuration.h"
#include "dataflow_analysis/processing/result.h"

namespace hal
{
    namespace processing
    {
        processing::Result run(const processing::Configuration& config, const std::shared_ptr<Grouping>& initial_grouping);
    }    // namespace processing
}    // namespace hal
