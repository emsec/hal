#pragma once

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/evaluation/configuration.h"
#include "dataflow_analysis/evaluation/context.h"
#include "dataflow_analysis/evaluation/result.h"
#include "dataflow_analysis/processing/result.h"

namespace hal
{
    namespace evaluation
    {
        evaluation::Result run(const Configuration& config, Context& ctx, const std::shared_ptr<Grouping>& initial_grouping, const processing::Result& result);
    }    // namespace evaluation
}    // namespace hal
