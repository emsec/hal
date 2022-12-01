#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistPreprocessingPlugin>();
    }

    std::string NetlistPreprocessingPlugin::get_name() const
    {
        return std::string("netlist_preprocessing");
    }

    std::string NetlistPreprocessingPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void NetlistPreprocessingPlugin::initialize()
    {
    }

    Result<u32> NetlistPreprocessingPlugin::remove_irrelevant_lut_inputs(Netlist* nl)
    {
        return ERR("not implemented");
    }

    Result<u32> NetlistPreprocessingPlugin::remove_buffers(Netlist* nl)
    {
        return ERR("not implemented");
    }

    Result<u32> NetlistPreprocessingPlugin::remove_redundant_logic(Netlist* nl)
    {
        const auto& nets   = nl->get_nets();
        auto nets_to_check = std::set<Net*>(nets.begin(), nets.end());

        while (!nets_to_check.empty())
        {
            auto* current_net = *nets_to_check.begin();

            // only continue for nets with multiple destinations
            if (current_net->get_num_of_destinations() <= 1)
            {
                nets_to_check.erase(current_net);
                continue;
            }

            std::vector<std::vector<Gate*>> redundant_logic;
            std::set<Gate*> visited_gates;

            auto destinations = current_net->get_destinations();
            for (u32 i = 0; i < destinations.size(); i++)
            {
                auto master_destination = destinations.at(i);
                auto master_gate        = master_destination->get_gate();

                // check if we have already identified the current master gate as duplicate of some other gate
                if (visited_gates.find(master_gate) != visited_gates.end())
                {
                    continue;
                }

                // skip everything that is not combinational, a FF, or a latch
                auto master_type = master_gate->get_type();
                if (master_type->has_property(GateTypeProperty::combinational) || master_type->has_property(GateTypeProperty::ff) || master_type->has_property(GateTypeProperty::latch))
                {
                    continue;
                }

                // cache master inputs and sort for faster comparison
                auto master_inputs = master_gate->get_fan_in_nets();
                std::sort(master_inputs.begin(), master_inputs.end());

                // identify duplicate gates
                std::vector<Gate*> duplicate_gates = {master_gate};
                for (u32 j = 1; j < destinations.size(); j++)
                {
                    auto current_destination = destinations.at(j);
                    auto current_gate        = current_destination->get_gate();

                    // check if we have already identified the current gate as duplicate of some other gate
                    if (visited_gates.find(current_gate) != visited_gates.end())
                    {
                        continue;
                    }

                    // check against master gate type
                    if (current_gate->get_type() != master_type)
                    {
                        continue;
                    }

                    // check against master input nets
                    auto current_inputs = current_gate->get_fan_in_nets();
                    std::sort(current_inputs.begin(), current_inputs.end());
                    if (current_inputs != master_inputs)
                    {
                        continue;
                    }

                    // TODO SMT check
                    bool skip_gate = false;
                    for (const auto* pin : master_type->get_output_pins())
                    {
                        const auto solver_res =
                            master_gate->get_resolved_boolean_function(pin)
                                .map<BooleanFunction>([pin, current_gate](auto&& bf_master) {
                                    return current_gate->get_resolved_boolean_function(pin).map<BooleanFunction>(
                                        [bf_master = std::move(bf_master)](auto&& bf_current) mutable { return BooleanFunction::Eq(std::move(bf_master), bf_current.clone(), 1); });
                                })
                                .map<BooleanFunction>([](auto&& bf_eq) { return BooleanFunction::Not(bf_eq.clone(), 1); })
                                .map<SMT::SolverResult>([](auto&& bf_not) -> Result<SMT::SolverResult> { return SMT::Solver({SMT::Constraint(std::move(bf_not))}).query(SMT::QueryConfig()); });

                        if (solver_res.is_error() || !solver_res.get().is_unsat())
                        {
                            skip_gate = true;
                            break;
                        }
                    }

                    if (skip_gate)
                    {
                        continue;
                    }

                    // gate is determined to be duplicate of other gate
                    duplicate_gates.push_back(current_gate);
                    visited_gates.insert(current_gate);
                }

                // mark as redundant logic if duplicate gates found
                if (duplicate_gates.size() > 1)
                {
                    redundant_logic.push_back(duplicate_gates);
                }
            }

            // TODO remove duplicate gates

            // TODO remove output nets of duplicate gates (also from set)

            // TODO insert output net of remaining destination into set (must be checked again)

            nets_to_check.erase(current_net);
        }

        return ERR("not implemented");
    }
}    // namespace hal
