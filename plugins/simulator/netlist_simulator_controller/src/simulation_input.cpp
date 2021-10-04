#include "netlist_simulator_controller/simulation_input.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal {

    bool SimulationInput::contains_gate(const Gate* g) const
    {
        return (mSimulationSet.find(g) != mSimulationSet.end());
    }

    const std::unordered_set<const Gate*>& SimulationInput::gates() const
    {
        return mSimulationSet;
    }

    void SimulationInput::add_gates(const std::vector<Gate*>& gates)
    {
        mSimulationSet.insert(gates.begin(), gates.end());

        compute_input_nets();
        compute_output_nets();
    }

    const std::unordered_set<const Gate*>& SimulationInput::get_gates() const
    {
        return mSimulationSet;
    }

    void SimulationInput::add_clock_frequency(const Net* clock_net, u64 frequency, bool start_at_zero)
    {
        u64 period = 1'000'000'000'000ul / frequency;
        add_clock_period(clock_net, period, start_at_zero);
    }

    void SimulationInput::add_clock_period(const Net* clock_net, u64 period, bool start_at_zero)
    {
        Clock c;
        c.clock_net     = clock_net;
        c.switch_time   = period / 2;
        c.start_at_zero = start_at_zero;
        m_clocks.push_back(c);
    }

    void SimulationInput::clear()
    {
        mSimulationSet.clear();
        m_clocks.clear();
        m_input_nets.clear();
        m_output_nets.clear();
    }

    void SimulationInput::compute_input_nets()
    {
        m_input_nets.clear();
        for (auto gate : mSimulationSet)
        {
            for (auto net : gate->get_fan_in_nets())
            {
                // "input net" is either a global input...
                if (net->is_global_input_net())
                {
                    m_input_nets.push_back(net);
                }
                else    // ... or has a source outside of the simulation set
                {
                    for (auto src : net->get_sources())
                    {
                        if (!contains_gate(src->get_gate()))
                        {
                            m_input_nets.push_back(net);
                            break;
                        }
                    }
                }
            }
        }
    }

    void SimulationInput::compute_output_nets()
    {
        m_output_nets.clear();
        for (auto gate : mSimulationSet)
        {
            for (auto net : gate->get_fan_out_nets())
            {
                // "output net" is either a global output...
                if (net->is_global_output_net())
                {
                    m_output_nets.push_back(net);
                }
                else    // ... or has a destination outside of the simulation set
                {
                    for (auto dst : net->get_destinations())
                    {
                        if (!contains_gate(dst->get_gate()))
                        {
                            m_output_nets.push_back(net);
                            break;
                        }
                    }
                }
            }
        }
    }

}
