#include "netlist_simulator/netlist_simulator.h"

namespace hal
{
    NetlistSimulator::SimulationGate::SimulationGate(const Gate* gate)
    {
        m_gate = gate;

        for (const std::string& pin : gate->get_type()->get_input_pins())
        {
            m_input_values[pin] = BooleanFunction::Value::X;
            m_input_pins.push_back(pin);
            m_input_nets.push_back(gate->get_fan_in_net(pin));
        }
    }
}    // namespace hal