#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "xilinx_toolbox/plugin_xilinx_toolbox.h"

namespace hal
{
    namespace XILINX_UNISIM
    {
        Result<u32> split_shift_registers(Netlist* nl)
        {
            u32 deleted_gates = 0;
            u32 new_gates     = 0;
            std::vector<Gate*> to_delete;

            GateType* ff_gt = nl->get_gate_library()->get_gate_type_by_name("FDE");
            if (ff_gt == nullptr)
            {
                return ERR("TODO");
            }

            // const auto gnd_net_res = NetlistModificationDecorator(*nl).create_gnd_net();
            // if (gnd_net_res.is_error())
            // {
            //     return ERR_APPEND(gnd_net_res.get_error(), "TODO");
            // }
            // Net* gnd_net = gnd_net_res.get().front();

            for (const auto& gate : nl->get_gates([](const auto& g) { return g->get_type()->get_name() == "SRL16E"; }))
            {
                auto select_pins = gate->get_type()->get_pins([](const auto& pg) { return (pg->get_direction() == PinDirection::input) && (pg->get_type() == PinType::select); });
                if (select_pins.size() != 4)
                {
                    return ERR("TODO");
                }
                std::sort(select_pins.begin(), select_pins.end(), [](const auto& p1, const auto& p2) {
                    const u32 idx1 = std::stoull(p1->get_name().substr(1));
                    const u32 idx2 = std::stoull(p2->get_name().substr(1));

                    return idx1 < idx2;
                });

                u32 select_value = 0;
                for (u32 idx = 0; idx < select_pins.size(); idx++)
                {
                    const Net* sn = gate->get_fan_in_net(select_pins.at(idx));

                    if (sn == nullptr)
                    {
                        log_warning("TODO");
                        continue;
                    }

                    if (!sn->is_gnd_net() && !sn->is_vcc_net())
                    {
                        log_warning("TODO");
                        continue;
                    }

                    select_value += (sn->is_gnd_net() ? 0 : 1) << idx;
                }

                // std::cout << "Found select pins: " << std::endl;
                // for (const auto& sp : select_pins)
                // {
                //     std::cout << sp->get_name() << std::endl;
                // }
                // std::cout << "Select Value: " << select_value << std::endl;

                const auto clock_pins = gate->get_type()->get_pins([](const auto& p) { return (p->get_direction() == PinDirection::input) && (p->get_type() == PinType::clock); });
                if (clock_pins.size() != 1)
                {
                    return ERR("TODO");
                }

                const auto enable_pins = gate->get_type()->get_pins([](const auto& p) { return (p->get_direction() == PinDirection::input) && (p->get_type() == PinType::enable); });
                if (enable_pins.size() != 1)
                {
                    return ERR("TODO");
                }

                const auto data_pins = gate->get_type()->get_pins([](const auto& p) { return (p->get_direction() == PinDirection::input) && (p->get_type() == PinType::data); });
                if (data_pins.size() != 1)
                {
                    return ERR("TODO");
                }

                const auto state_pins = gate->get_type()->get_pins([](const auto& p) { return (p->get_direction() == PinDirection::output) && (p->get_type() == PinType::state); });
                if (state_pins.size() != 1)
                {
                    return ERR("TODO");
                }

                Net* clk_in    = gate->get_fan_in_net(clock_pins.front());
                Net* enable_in = gate->get_fan_in_net(enable_pins.front());
                Net* data_in   = gate->get_fan_in_net(data_pins.front());
                Net* state_out = gate->get_fan_out_net(state_pins.front());

                if (clk_in == nullptr)
                {
                    return ERR("TODO");
                }

                if (enable_in == nullptr)
                {
                    return ERR("TODO");
                }

                if (data_in == nullptr)
                {
                    return ERR("TODO");
                }

                if (state_out == nullptr)
                {
                    return ERR("TODO");
                }

                std::vector<Gate*> flip_flops;
                std::vector<Net*> state_nets;

                for (u32 ff_idx = 0; ff_idx <= select_value; ff_idx++)
                {
                    const std::string ff_name = gate->get_name() + "_split_ff_" + std::to_string(ff_idx);
                    Gate* new_gate            = nl->create_gate(ff_gt, ff_name);
                    new_gates++;

                    clk_in->add_destination(new_gate, "C");
                    enable_in->add_destination(new_gate, "CE");

                    if (ff_idx == 0)
                    {
                        data_in->add_destination(new_gate, "D");
                    }
                    else
                    {
                        state_nets.back()->add_destination(new_gate, "D");
                    }

                    if (ff_idx == select_value)
                    {
                        state_out->add_source(new_gate, "Q");
                        state_nets.push_back(state_out);
                    }
                    else
                    {
                        Net* new_net = nl->create_net(ff_name + "_out");
                        new_net->add_source(new_gate, "Q");
                        state_nets.push_back(new_net);
                    }
                }

                to_delete.push_back(gate);
            }

            for (const auto& g : to_delete)
            {
                if (!nl->delete_gate(g))
                {
                    return ERR("Cannot split shift register primitives for netlist with ID " + std::to_string(nl->get_id()) + ": Failed to delete gate " + g->get_name() + " with ID "
                               + std::to_string(g->get_id()));
                }
                else
                {
                    deleted_gates++;
                }
            }

            log_info("xilinx_toolbox", "Split up {} SRLE16 into {} flip flops", deleted_gates, new_gates);
            return OK(deleted_gates);
        }

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
        std::map<std::string, std::function<Result<u32>(Netlist*)>> gate_lib_to_func = {{"XILINX_UNISIM", XILINX_UNISIM::split_luts}, {"XILINX_UNISIM_WITH_HAL_TYPES", XILINX_UNISIM::split_luts}};

        if (gate_lib_to_func.find(nl->get_gate_library()->get_name()) == gate_lib_to_func.end())
        {
            return ERR("Cannot split LUTs for netlist with ID " + std::to_string(nl->get_id()) + ": Gate library " + nl->get_gate_library()->get_name() + " not supported");
        }

        return gate_lib_to_func.at(nl->get_gate_library()->get_name())(nl);
    }

    Result<u32> XilinxToolboxPlugin::split_shift_registers(Netlist* nl)
    {
        std::map<std::string, std::function<Result<u32>(Netlist*)>> gate_lib_to_func = {{"XILINX_UNISIM", XILINX_UNISIM::split_shift_registers},
                                                                                        {"XILINX_UNISIM_WITH_HAL_TYPES", XILINX_UNISIM::split_shift_registers}};

        if (gate_lib_to_func.find(nl->get_gate_library()->get_name()) == gate_lib_to_func.end())
        {
            return ERR("Cannot split shift registers for netlist with ID " + std::to_string(nl->get_id()) + ": Gate library " + nl->get_gate_library()->get_name() + " not supported");
        }

        return gate_lib_to_func.at(nl->get_gate_library()->get_name())(nl);
    }

}    // namespace hal