#pragma once

#include "hal_core/defines.h"

namespace hal
{
    /* forward declaration */
    struct NetlistAbstraction;

    namespace pre_processing
    {
        void identify_register_stages(NetlistAbstraction& netlist_abstr);
    }    // namespace pre_processing
}    // namespace hal
