#pragma once

#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct Grouping;

        namespace group_by_control_signals
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool clock, bool clock_enable, bool reset, bool set);
            std::shared_ptr<Grouping> pure_control_signals_process(const std::shared_ptr<Grouping>& state, bool clock, bool clock_enable, bool reset, bool set);

        }    // namespace group_by_control_signals
    }        // namespace dataflow
}