#pragma once

#include "hal_core/defines.h"

#include <vector>

namespace hal
{
    /* Forward declaration */
    class Gate;
    class Netlist;

    namespace machine_learning
    {
        enum GraphDirection
        {
            directed_forward,
            directed_backward,
            bidirectional,
        };
    }    // namespace machine_learning
}    // namespace hal