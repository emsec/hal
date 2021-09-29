#include "netlist_simulator/simulation_utils.h"

namespace hal
{
    namespace simulation_utils
    {
        BooleanFunction::Value toggle(BooleanFunction::Value v)
        {
            if (v == 0 || v == 1)
            {
                return static_cast<BooleanFunction::Value>(1 - v);
            }
            return v;
        }

        BooleanFunction::Value process_clear_preset_behavior(AsyncSetResetBehavior behavior, BooleanFunction::Value previous_output)
        {
            if (behavior == AsyncSetResetBehavior::N)
            {
                return previous_output;
            }
            else if (behavior == AsyncSetResetBehavior::X)
            {
                return BooleanFunction::Value::X;
            }
            else if (behavior == AsyncSetResetBehavior::L)
            {
                return BooleanFunction::Value::ZERO;
            }
            else if (behavior == AsyncSetResetBehavior::H)
            {
                return BooleanFunction::Value::ONE;
            }
            else if (behavior == AsyncSetResetBehavior::T)
            {
                return toggle(previous_output);
            }
            log_error("netlist_simulator", "unsupported set/reset behavior {}.", behavior);
            return BooleanFunction::Value::X;
        }
    }    // namespace simulation_utils
}    // namespace hal