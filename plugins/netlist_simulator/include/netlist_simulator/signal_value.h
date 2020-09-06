#pragma once

#include "hal_core/netlist/boolean_function.h"

namespace hal
{
    enum SignalValue
    {
        ZERO = BooleanFunction::ZERO,
        ONE  = BooleanFunction::ONE,
        X    = BooleanFunction::X,
        Z    = 42
    };
}
