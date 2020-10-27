#pragma once

#include "hal_core/netlist/boolean_function.h"

namespace hal
{
    /**
     * Represents the logic value that a signal can take.
     */
    enum SignalValue
    {
        ZERO = BooleanFunction::ZERO, /**< Represents a logical 0. */
        ONE  = BooleanFunction::ONE,  /**< Represents a logical 1. */
        X    = BooleanFunction::X,    /**< Represents an undefined value. */
        Z    = 42                     /**< Represents high impedance (currently not supported by the simulator). */
    };
}    // namespace hal
