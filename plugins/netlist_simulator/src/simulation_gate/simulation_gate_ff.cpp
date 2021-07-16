#include "netlist_simulator/netlist_simulator.h"
#include "netlist_simulator/simulation_utils.h"

namespace hal
{
    bool NetlistSimulator::SimulationGateFF::simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        // is the event triggering a clock pin?
        // if so remember to process the flipflop later!
        if (std::find(clock_nets.begin(), clock_nets.end(), event.affected_net) != clock_nets.end())
        {
            // return true if the event was completely handled
            // -> true if the gate is NOT clocked at this point
            return (clock_func.evaluate(input_values) != BooleanFunction::ONE);
        }
        else    // not a clock pin -> only check for asynchronous signals
        {
            BooleanFunction::Value async_set   = preset_func.evaluate(input_values);
            BooleanFunction::Value async_reset = clear_func.evaluate(input_values);

            // check whether an asynchronous set or reset ist triggered
            if (async_set == BooleanFunction::ONE || async_reset == BooleanFunction::ONE)
            {
                BooleanFunction::Value result     = BooleanFunction::Value::X;
                BooleanFunction::Value inv_result = BooleanFunction::Value::X;

                if (async_set == BooleanFunction::ONE && async_reset == BooleanFunction::ONE)
                {
                    // both signals set? -> evaluate special behavior
                    BooleanFunction::Value old_output = BooleanFunction::Value::X;
                    if (!state_output_nets.empty())
                    {
                        const Net* out_net = state_output_nets[0];
                        if (auto it = simulation.m_events.find(out_net); it != simulation.m_events.end())
                        {
                            old_output = it->second.back().new_value;
                        }
                    }
                    BooleanFunction::Value old_output_inv = BooleanFunction::Value::X;
                    if (!state_inverted_output_nets.empty())
                    {
                        auto out_net = state_inverted_output_nets[0];
                        if (auto it = simulation.m_events.find(out_net); it != simulation.m_events.end())
                        {
                            old_output_inv = it->second.back().new_value;
                        }
                    }
                    result     = simulation_utils::process_clear_preset_behavior(sr_behavior_out, old_output);
                    inv_result = simulation_utils::process_clear_preset_behavior(sr_behavior_out_inverted, old_output_inv);
                }
                else if (async_set == BooleanFunction::ONE)
                {
                    // only asynch set is 1
                    result     = BooleanFunction::Value::ONE;
                    inv_result = simulation_utils::toggle(result);
                }
                else if (async_reset == BooleanFunction::ONE)
                {
                    // only asynch reset is 1
                    result     = BooleanFunction::Value::ZERO;
                    inv_result = simulation_utils::toggle(result);
                }

                // generate events
                for (auto out_net : state_output_nets)
                {
                    new_events[std::make_pair(out_net, event.time + delay)] = result;
                }
                for (auto out_net : state_inverted_output_nets)
                {
                    new_events[std::make_pair(out_net, event.time + delay)] = inv_result;
                }
            }
        }

        return true;
    }

    void NetlistSimulator::SimulationGateFF::clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        // compute output
        BooleanFunction::Value result     = next_state_func.evaluate(input_values);
        BooleanFunction::Value inv_result = simulation_utils::toggle(result);

        // generate events
        for (const Net* out_net : state_output_nets)
        {
            new_events[std::make_pair(out_net, current_time + delay)] = result;
        }
        for (const Net* out_net : state_inverted_output_nets)
        {
            new_events[std::make_pair(out_net, current_time + delay)] = inv_result;
        }
    }
}    // namespace hal