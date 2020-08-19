#pragma once

#include "netlist/boolean_function.h"

namespace hal
{
    enum SignalValue
    {
        ZERO = BooleanFunction::value::ZERO,
        ONE  = BooleanFunction::value::ONE,
        X    = BooleanFunction::value::X,
        Z    = 42
    };
}
