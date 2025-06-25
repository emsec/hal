#include "netlist_simulator_controller/simulation_input.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/module.h"
#include <stdio.h>

namespace hal {

    std::vector<const Net *> SimulationInput::NetGroup::get_nets() const
    {
        std::vector<const Net *> retval;
        if (gate)
        {
            for (GatePin* gp : gate_pin_group->get_pins())
            {
                Net* n = nullptr;
                if (gp->get_direction() == PinDirection::input || gp->get_direction() == PinDirection::inout)
                    n = gate->get_fan_in_net(gp);
                else
                    n = gate->get_fan_out_net(gp);
                if (n) retval.push_back(n);
            }
        }
        else
        {
            for (ModulePin* mp : module_pin_group->get_pins())
                retval.push_back(mp->get_net());
        }
        return retval;
    }

    std::string SimulationInput::NetGroup::get_name() const
    {
        if (gate)
            return gate_pin_group->get_name();
        return module_pin_group->get_name();
    }

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

    bool SimulationInput::is_no_clock_used() const
    {
        return mNoClockUsed;
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
                    int number_sources = 0;
                    for (auto src : net->get_sources())
                    {
                        ++number_sources;
                        if (!contains_gate(src->get_gate()))
                        {
                            m_input_nets.insert(net);
                            break;
                        }
                    }
                    if (!number_sources)
                        m_input_nets.insert(net);
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
                    int number_destinations = 0;
                    for (auto dst : net->get_destinations())
                    {
                        ++number_destinations;
                        if (!contains_gate(dst->get_gate()))
                        {
                            m_output_nets.push_back(net);
                            break;
                        }
                    }
                    if (!number_destinations)
                        m_output_nets.push_back(net);
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

    void SimulationInput::compute_net_groups()
    {
        std::unordered_set<const Module*> simulated_modules;

        // all nets that are part of the simulation
        std::unordered_map<const Net*, DirectionType> ungrouped_nets;

        for (const Net* n : m_partial_nets) ungrouped_nets[n] = Undefined;
        for (const Net* n : m_input_nets)   ungrouped_nets[n] = InputOnly;
        for (const Net* n : m_output_nets)  ungrouped_nets[n] = OutputOnly;

        // all modules that contain at least one simulated gate
        for (const Gate* g : mSimulationSet)
        {
            const Module *m = g->get_module();
            while (m)
            {
                simulated_modules.insert(m);
                m = m->get_parent_module();
            }
        }

        // check all pingroups from affected modules, working the module hierarchy top -> bottom
        auto it = simulated_modules.begin();
        for (int level = 0; ! simulated_modules.empty(); )
        {
           while ((*it)->get_submodule_depth() != level)
           {
               if (++it == simulated_modules.end())
               {
                   ++level;
                   it = simulated_modules.begin();
               }
           }

           const Module*m = (*it);
           for (PinGroup<ModulePin>* pg : m->get_pin_groups())
           {
               if (pg->size() < 2 || pg->size() > 31) continue;
               bool pin_group_simulated = true;
               DirectionType groupType = Undefined;

               for (ModulePin* mp : pg->get_pins())
               {
                   Net* n = mp->get_net();
                   if (n)
                   {
                       auto itNet = ungrouped_nets.find(n);
                       if (itNet == ungrouped_nets.end())
                       {
                           // pin : net exists and is not part of the simulation, ignore pin group
                           pin_group_simulated = false;
                           break;
                       }
                       else
                       {
                           if (groupType == Undefined)
                               groupType = itNet->second;
                           else if (groupType != Mixed && groupType != itNet->second)
                               groupType = Mixed;
                       }
                   }
               }
               if (pin_group_simulated)
               {
                   NetGroup group;
                   group.module_pin_group = pg;
                   group.direction = groupType;
                   for (ModulePin* mp : pg->get_pins())
                   {
                       Net* n = mp->get_net();
                       if (n)
                       {
                           ungrouped_nets.erase(n);
                       }
                   }
                   m_netgroups.push_back(group);
               }
           }

           simulated_modules.erase(it);
           it = simulated_modules.begin();
        }

        for (const Gate* g : mSimulationSet)
        {
            const GateType* gt = g->get_type();

            for (PinGroup<GatePin>* pg : gt->get_pin_groups())
            {
                if (pg->size() < 2 || pg->size() > 31) continue;
                bool pin_group_simulated = true;
                DirectionType groupType = Undefined;
                std::unordered_set<const Net*> connectedNets;

                for (GatePin* gp : pg->get_pins())
                {
                    Net* n = nullptr;
                    switch (gp->get_direction())
                    {
                    case PinDirection::inout:
                        n = g->get_fan_in_net(gp);
                        break;
                    case PinDirection::output:
                        n = g->get_fan_out_net(gp);
                        break;
                    default:
                        break;
                    }
                    if (n)
                    {
                        auto itNet = ungrouped_nets.find(n);
                        if (itNet == ungrouped_nets.end())
                        {
                            // pin : net already assigned to module pin group, ignore gate pin group
                            pin_group_simulated = false;
                            break;
                        }
                        else
                        {
                            connectedNets.insert(n);
                            if (groupType == Undefined)
                                groupType = itNet->second;
                            else if (groupType != Mixed && groupType != itNet->second)
                                groupType = Mixed;
                        }
                    }
                    else
                        pin_group_simulated = false;
                }

                if (pin_group_simulated)
                {
                    NetGroup group;
                    group.gate = g;
                    group.gate_pin_group = pg;
                    group.direction = groupType;
                    for (const Net* n : connectedNets)
                        ungrouped_nets.erase(n);
                    m_netgroups.push_back(group);
                }
            }
        }
    }
}
