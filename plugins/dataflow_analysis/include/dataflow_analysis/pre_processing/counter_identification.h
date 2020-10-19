#pragma once

#include "hal_core/defines.h"

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct NetlistAbstraction;

        namespace pre_processing
        {
            void identify_counters(NetlistAbstraction& netlist_abstr);
        }    // namespace pre_processing
    }        // namespace dataflow
}