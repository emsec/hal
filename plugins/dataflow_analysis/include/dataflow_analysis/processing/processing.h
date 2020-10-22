#pragma once

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/processing/configuration.h"
#include "dataflow_analysis/processing/result.h"
#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        namespace processing
        {
            processing::Result run(const processing::Configuration& config, const std::shared_ptr<Grouping>& initial_grouping);
        }    // namespace processing
    }        // namespace dataflow
}    // namespace hal