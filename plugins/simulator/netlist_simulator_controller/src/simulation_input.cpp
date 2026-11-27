#include "netlist_simulator_controller/simulation_input.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/module.h"
#include <algorithm>
#include <cctype>
#include <stdio.h>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

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
        m_netgroups.clear();
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

    namespace
    {
        // A pin group whose nets are all part of the simulation, competing for its nets with other pin groups.
        struct NetGroupCandidate
        {
            SimulationInput::NetGroup group;
            std::vector<const Net*> nets;
            int rank;                   // 0: module source group, 1: other module group, 2: gate source group, 3: other gate group
            std::string sort_name;      // lower-case pin group name
            std::string name;
            u32 owner_id;
            u32 pin_group_id;
        };

        SimulationInput::DirectionType group_direction(const std::vector<const Net*>& nets, const std::unordered_map<const Net*, SimulationInput::DirectionType>& simulated_nets)
        {
            SimulationInput::DirectionType retval = SimulationInput::Undefined;
            for (const Net* n : nets)
            {
                SimulationInput::DirectionType dir = simulated_nets.at(n);
                if (retval == SimulationInput::Undefined)
                    retval = dir;
                else if (retval != SimulationInput::Mixed && retval != dir)
                    retval = SimulationInput::Mixed;
            }
            return retval;
        }

        std::string to_lower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
            return s;
        }
    }    // namespace

    void SimulationInput::compute_net_groups()
    {
        m_netgroups.clear();

        // all nets that are part of the simulation
        std::unordered_map<const Net*, DirectionType> simulated_nets;

        for (const Net* n : m_partial_nets) simulated_nets[n] = Undefined;
        for (const Net* n : m_input_nets)   simulated_nets[n] = InputOnly;
        for (const Net* n : m_output_nets)  simulated_nets[n] = OutputOnly;

        // all modules that contain at least one simulated gate
        std::unordered_set<const Module*> simulated_modules;
        for (const Gate* g : mSimulationSet)
        {
            const Module *m = g->get_module();
            while (m)
            {
                simulated_modules.insert(m);
                m = m->get_parent_module();
            }
        }

        std::vector<NetGroupCandidate> candidates;

        for (const Module* m : simulated_modules)
        {
            for (PinGroup<ModulePin>* pg : m->get_pin_groups())
            {
                if (pg->size() < 2 || pg->size() > 31) continue;
                bool pin_group_simulated = true;
                bool is_source = true;
                std::vector<const Net*> nets;

                for (ModulePin* mp : pg->get_pins())
                {
                    if (mp->get_direction() != PinDirection::output)
                        is_source = false;
                    const Net* n = mp->get_net();
                    if (!n) continue;
                    if (simulated_nets.find(n) == simulated_nets.end())
                    {
                        // pin : net exists and is not part of the simulation, ignore pin group
                        pin_group_simulated = false;
                        break;
                    }
                    nets.push_back(n);
                }
                if (!pin_group_simulated || nets.empty()) continue;

                NetGroupCandidate cand;
                cand.group.module_pin_group = pg;
                cand.group.direction        = group_direction(nets, simulated_nets);
                cand.group.ascending        = pg->is_ascending();
                cand.nets                   = nets;
                cand.rank                   = is_source ? 0 : 1;
                cand.name                   = pg->get_name();
                cand.sort_name              = to_lower(cand.name);
                cand.owner_id               = m->get_id();
                cand.pin_group_id           = pg->get_id();
                candidates.push_back(cand);
            }
        }

        for (const Gate* g : mSimulationSet)
        {
            for (PinGroup<GatePin>* pg : g->get_type()->get_pin_groups())
            {
                if (pg->size() < 2 || pg->size() > 31) continue;
                bool pin_group_simulated = true;
                bool is_source = true;
                std::vector<const Net*> nets;

                for (GatePin* gp : pg->get_pins())
                {
                    if (gp->get_direction() != PinDirection::output)
                        is_source = false;
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
                    if (!n || simulated_nets.find(n) == simulated_nets.end())
                    {
                        pin_group_simulated = false;
                        break;
                    }
                    nets.push_back(n);
                }
                if (!pin_group_simulated) continue;

                NetGroupCandidate cand;
                cand.group.gate           = g;
                cand.group.gate_pin_group = pg;
                cand.group.direction      = group_direction(nets, simulated_nets);
                cand.group.ascending      = pg->is_ascending();
                cand.nets                 = nets;
                cand.rank                 = is_source ? 2 : 3;
                cand.name                 = pg->get_name();
                cand.sort_name            = to_lower(cand.name);
                cand.owner_id             = g->get_id();
                cand.pin_group_id         = pg->get_id();
                candidates.push_back(cand);
            }
        }

        // pin groups sharing nets: module groups before gate groups, then source groups (all pins are outputs), then the alphabetically first name
        std::sort(candidates.begin(), candidates.end(), [](const NetGroupCandidate& a, const NetGroupCandidate& b) {
            return std::tie(a.rank, a.sort_name, a.name, a.owner_id, a.pin_group_id) < std::tie(b.rank, b.sort_name, b.name, b.owner_id, b.pin_group_id);
        });

        std::unordered_set<const Net*> grouped_nets;
        for (const NetGroupCandidate& cand : candidates)
        {
            if (std::any_of(cand.nets.begin(), cand.nets.end(), [&grouped_nets](const Net* n) { return grouped_nets.find(n) != grouped_nets.end(); }))
                continue;
            grouped_nets.insert(cand.nets.begin(), cand.nets.end());
            m_netgroups.push_back(cand.group);
        }
    }
}
