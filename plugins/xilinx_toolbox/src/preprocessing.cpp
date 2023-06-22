#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "xilinx_toolbox/plugin_xilinx_toolbox.h"

namespace hal
{
    namespace XILINX_UNISIM
    {
        Result<u32> split_luts(Netlist* nl)
        {
            u32 deleted_gates = 0;
            u32 new_gates     = 0;
            std::vector<Gate*> to_delete;

            const auto lut6_type = nl->get_gate_library()->get_gate_type_by_name("LUT6");
            const auto lut5_type = nl->get_gate_library()->get_gate_type_by_name("LUT5");

            const auto lut6_2_gates = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6_2"; });

            for (const auto& g : lut6_2_gates)
            {
                auto* o5       = g->get_fan_out_net("O5");
                auto* o6       = g->get_fan_out_net("O6");
                const auto* i5 = g->get_fan_in_net("I5");

                const auto init_get_res = g->get_init_data();
                if (init_get_res.is_error())
                {
                    log_warning("xilinx_toolbox", "could not get INIT string of gate '{}' with ID {}, skipping this gate.", g->get_name(), g->get_id());
                    continue;
                }
                auto init = init_get_res.get().front();
                if (init.length() != 16)
                {
                    log_warning("xilinx_toolbox", "INIT string '{}' has length {}, expected 16.", init, init.length());
                    continue;
                }

                if (o5->get_num_of_destinations() > 0)
                {
                    // create LUT5
                    auto* lut5 = nl->create_gate(lut5_type, g->get_name() + "_split_O5");
                    lut5->set_data("xilinx_preprocessing_information", "original_init", "string", init);

                    auto init_O5 = init.substr(8, 8);
                    if (lut5->set_init_data({init_O5}).is_error())
                    {
                        log_warning("xilinx_toolbox", "could not set INIT string of gate '{}' with ID {}, skipping this gate.", lut5->get_name(), lut5->get_id());
                    }

                    if (auto* mod = g->get_module(); !mod->is_top_module())
                    {
                        mod->assign_gate(lut5);
                    }

                    for (const auto& in_ep : g->get_fan_in_endpoints([](const Endpoint* ep) { return ep->get_pin()->get_name() != "I5"; }))
                    {
                        in_ep->get_net()->add_destination(lut5, in_ep->get_pin()->get_name());
                    }

                    new_gates++;
                    o5->add_source(lut5, "O");
                }
                
                if (o6->get_num_of_destinations() > 0)
                {
                    // create LUT6
                    auto* lut6 = nl->create_gate(lut6_type, g->get_name() + "_split_O6");
                    lut6->set_data("xilinx_preprocessing_information", "original_init", "string", init);

                    if (lut6->set_init_data({init}).is_error())
                    {
                        log_warning("xilinx_toolbox", "could not set INIT string of gate '{}' with ID {}, skipping this gate.", lut6->get_name(), lut6->get_id());
                        nl->delete_gate(lut6);
                        continue;
                    }

                    if (auto* mod = g->get_module(); !mod->is_top_module())
                    {
                        mod->assign_gate(lut6);
                    }

                    for (const auto& in_ep : g->get_fan_in_endpoints())
                    {
                        in_ep->get_net()->add_destination(lut6, in_ep->get_pin()->get_name());
                    }

                    new_gates++;
                    o6->add_source(lut6, "O");
                }
                to_delete.push_back(g);
            }

            for (const auto& g : to_delete)
            {
                if (!nl->delete_gate(g))
                {
                    return ERR("Cannot split luts for netlist with ID " + std::to_string(nl->get_id()) + ": Failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                }
                else
                {
                    deleted_gates++;
                }
            }

            log_info("xilinx_toolbox", "Split up {} LUT6_2 into {} smaller LUTs", deleted_gates, new_gates);
            return OK(deleted_gates);
        }
    }    // namespace XILINX_UNISIM

    Result<u32> XilinxToolboxPlugin::split_luts(Netlist* nl)
    {
        std::map<std::string, std::function<Result<u32>(Netlist*)>> gate_lib_to_func = {{"XILINX_UNISIM", XILINX_UNISIM::split_luts}};

        if (gate_lib_to_func.find(nl->get_gate_library()->get_name()) == gate_lib_to_func.end())
        {
            return ERR("Cannot split LUTs for netlist with ID " + std::to_string(nl->get_id()) + ": Gate library " + nl->get_gate_library()->get_name() + " not supported");
        }

        return gate_lib_to_func.at(nl->get_gate_library()->get_name())(nl);
    }

}    // namespace hal