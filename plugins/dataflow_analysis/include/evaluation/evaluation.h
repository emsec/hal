#pragma once

#include "common/netlist_abstraction.h"
#include "evaluation/configuration.h"
#include "evaluation/context.h"
#include "evaluation/result.h"
#include "processing/result.h"

namespace hal
{
    namespace evaluation
    {
        evaluation::Result run(const Configuration& config, Context& ctx, const std::shared_ptr<Grouping>& initial_grouping, const processing::Result& result);
    }    // namespace evaluation
}    // namespace hal
