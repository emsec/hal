#pragma once

#include "hal_core/defines.h"

namespace hal
{
    /* forward declaration */
    struct Grouping;

    namespace group_by_control_signals
    {
        std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool clock, bool clock_enable, bool reset);
        std::shared_ptr<Grouping> pure_control_signals_process(const std::shared_ptr<Grouping>& state, bool clock, bool clock_enable, bool reset);

    }    // namespace group_by_control_signals
}    // namespace hal
