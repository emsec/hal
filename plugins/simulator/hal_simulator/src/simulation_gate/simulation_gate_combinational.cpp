#include "netlist_simulator/netlist_simulator.h"

namespace hal
{
    NetlistSimulator::SimulationGateCombinational::SimulationGateCombinational(const Gate* gate) : SimulationGate(gate)
    {
        std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();

        m_output_pins = gate->get_type()->get_output_pins();

        for (const GatePin* pin : m_output_pins)
        {
            const Net* out_net = gate->get_fan_out_net(pin);
            m_output_nets.push_back(out_net);

            // resolve recursion within output functions
            BooleanFunction& func = functions.at(pin->get_name());
            while (true)
            {
                auto vars = func.get_variable_names();
                bool exit = true;
                for (const GatePin* other_pin : m_output_pins)
                {
                    if (const std::string& other_pin_name = other_pin->get_name(); std::find(vars.begin(), vars.end(), other_pin_name) != vars.end())
                    {
                        func = func.substitute(other_pin_name, functions.at(other_pin_name)).get();
                        exit = false;
                    }
                }
                if (exit)
                {
                    break;
                }
            }
            m_functions.emplace(out_net, func);
        }
    }

    bool NetlistSimulator::SimulationGateCombinational::simulate(const Simulation& simulation, const WaveEvent& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        UNUSED(simulation);

        // compute delay, currently just a placeholder
        u64 delay = 0;

        for (auto out_net : m_output_nets)
        {
            BooleanFunction::Value result = m_functions[out_net].evaluate(m_input_values).get();

            new_events[std::make_pair(out_net, event.time + delay)] = result;
        }

        return true;
    }
}    // namespace hal
