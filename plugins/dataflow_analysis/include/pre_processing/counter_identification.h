#pragma once

#include "def.h"

namespace hal
{
    /* forward declaration */
    struct NetlistAbstraction;

    namespace pre_processing
    {
        void identify_counters(NetlistAbstraction& netlist_abstr);
    }    // namespace pre_processing
}    // namespace hal