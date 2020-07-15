#pragma once

#include "common/netlist_abstraction.h"
#include "def.h"
#include "processing/configuration.h"
#include "processing/result.h"

namespace hal
{
    namespace processing
    {
        processing::Result run(const processing::Configuration& config, const std::shared_ptr<Grouping>& initial_grouping);
    }    // namespace processing
}    // namespace hal