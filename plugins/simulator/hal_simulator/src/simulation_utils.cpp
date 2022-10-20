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
            log_error("netlist_simulator", "unsupported set/reset behavior {}.", enum_to_string(behavior));
            return BooleanFunction::Value::X;
        }

        std::vector<BooleanFunction::Value> int_to_values(u32 integer, u32 len)
        {
            if (len > 32)
            {
                return {};
            }

            std::vector<BooleanFunction::Value> res;
            for (u32 i = 0; i < len; i++)
            {
                res.push_back((BooleanFunction::Value)((integer >> i) & 1));
            }

            return res;
        }

        u32 values_to_int(const std::vector<BooleanFunction::Value>& values)
        {
            u32 len = values.size();

            if (len > 32)
            {
                return 0;
            }

            u32 res = 0;
            for (u32 i = 0; i < len; i++)
            {
                BooleanFunction::Value val = values.at(i);
                if (val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z)
                {
                    return 0;
                }

                res ^= (u32)val << i;
            }

            return res;
        }

        u32 get_int_bus_value(const std::unordered_map<std::string, BooleanFunction::Value>& signal_values, const std::vector<GatePin*>& ordered_pins)
        {
            std::vector<BooleanFunction::Value> ordered_values;
            for (const GatePin* pin : ordered_pins)
            {
                ordered_values.push_back(signal_values.at(pin->get_name()));
            }

            return simulation_utils::values_to_int(ordered_values);
        }
    }    // namespace simulation_utils
}    // namespace hal
