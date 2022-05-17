#include "netlist_simulator_controller/simulation_input.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"
#include <stdio.h>

namespace hal {

    bool SimulationInput::contains_gate(const Gate* g) const
    {
        return (mSimulationSet.find(g) != mSimulationSet.end());
    }

    void SimulationInput::add_gates(const std::vector<Gate*>& gates)
    {
        mSimulationSet.insert(gates.begin(), gates.end());

        compute_input_nets();
        compute_output_nets();
        compute_partial_nets();
    }

    const std::unordered_set<const Gate*>& SimulationInput::get_gates() const
    {
        return mSimulationSet;
    }

    bool SimulationInput::is_clock(const Net* n) const
    {
        for (const Clock& c: m_clocks)
            if (c.clock_net == n) return true;
        return false;
    }

    void SimulationInput::add_clock(const Clock& clk)
    {
        m_clocks.push_back(clk);
    }

    void SimulationInput::clear()
    {
        mSimulationSet.clear();
        m_clocks.clear();
        m_input_nets.clear();
        m_output_nets.clear();
        m_partial_nets.clear();
    }

    bool SimulationInput::is_ready() const
    {
        return has_gates() && (!m_clocks.empty() || mNoClockUsed) && !m_input_nets.empty();
    }

    bool SimulationInput::has_gates() const
    {
        return !mSimulationSet.empty();
    }

    const std::unordered_set<const Net *>& SimulationInput::get_input_nets() const
    {
        return m_input_nets;
    }

    const std::vector<const Net*>& SimulationInput::get_output_nets() const
    {
        return m_output_nets;
    }

    const std::vector<const Net*>& SimulationInput::get_partial_netlist_nets() const
    {
        return m_partial_nets;
    }

    bool SimulationInput::is_input_net(const Net* n) const
    {
        return (m_input_nets.find(n) != m_input_nets.end());
    }

    void SimulationInput::set_no_clock_used()
    {
        mNoClockUsed = true;
    }

    void SimulationInput::dump(std::string filename) const
    {
        FILE* of = stderr;
        if (!filename.empty())
            if (!(of = fopen(filename.c_str(),"w")))
            {
                log_warning("SimulationInput", "cannot open simulation input dump file '{}'", filename);
                return;
            }

        fprintf(of, "Gates:______________________________________\n");
        for (const Gate* g: mSimulationSet)
        {
            fprintf(of, "  %4d <%s>\n", g->get_id(), g->get_name().c_str());
        }
        fprintf(of, "Clocks:_____________________________________\n");
        for (const Clock& clk: m_clocks)
        {
            fprintf(of, "  %4d <%s> \t period: %u \n", clk.clock_net->get_id(), clk.clock_net->get_name().c_str(), (unsigned int) clk.period());
        }
        fprintf(of, "Input nets:_________________________________\n");
        for (const Net* n: m_input_nets)
        {
            fprintf(of, "  %4d <%s>\n", n->get_id(), n->get_name().c_str());
        }
        fprintf(of, "Output nets:________________________________\n");
        for (const Net* n: m_output_nets)
        {
            fprintf(of, "  %4d <%s>\n", n->get_id(), n->get_name().c_str());
        }
        fprintf(of, "Partial netlist nets:________________________________\n");
        for (const Net* n: m_partial_nets)
        {
            fprintf(of, "  %4d <%s>\n", n->get_id(), n->get_name().c_str());
        }
        if (filename.empty())
            fflush(of);
        else
            fclose(of);
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
                    m_input_nets.insert(net);
                }
                else    // ... or has a source outside of the simulation set
                {
                    for (auto src : net->get_sources())
                    {
                        if (!contains_gate(src->get_gate()))
                        {
                            m_input_nets.insert(net);
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

    void SimulationInput::compute_partial_nets()
    {
        m_partial_nets.clear();
        std::unordered_set<const Net*> found;
        for (const Gate* g : mSimulationSet)
        {
            for (const Net* n : g->get_fan_in_nets())
            {
                if (found.find(n) == found.end())
                {
                    found.insert(n);
                    m_partial_nets.push_back(n);
                }
            }
            for (const Net* n : g->get_fan_out_nets())
            {
                if (found.find(n) == found.end())
                {
                    found.insert(n);
                    m_partial_nets.push_back(n);
                }
            }
        }
    }

}
