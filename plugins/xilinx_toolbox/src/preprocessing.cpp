#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "xilinx_toolbox/plugin_xilinx_toolbox.h"

namespace
{
    // Extract the numeric index from an address pin name.
    // Handles both "Ai" (e.g. "A0", "A3") and "A(i)" (e.g. "A(0)", "A(4)") formats.
    u32 srl_addr_pin_index(const hal::GatePin* p)
    {
        std::string digits;
        for (unsigned char c : p->get_name())
        {
            if (c >= '0' && c <= '9')
            {
                digits += static_cast<char>(c);
            }
        }
        return static_cast<u32>(std::stoull(digits));
    }

    // Returns the fixed cascade output pin name for cascadeable SRL types, or empty string.
    // SRLC16E always outputs stage 15 on Q15; SRLC32E always outputs stage 31 on Q31.
    std::string srl_cascade_pin(const std::string& type_name)
    {
        if (type_name == "SRLC16E")
        {
            return "Q15";
        }
        if (type_name == "SRLC32E")
        {
            return "Q31";
        }
        return {};
    }

    bool is_srl_type(const std::string& type_name)
    {
        return type_name == "SRL16E" || type_name == "SRL16" || type_name == "SRLC16E" || type_name == "SRLC32E";
    }
}    // namespace

namespace hal
{
    namespace xilinx_toolbox
    {
        Result<std::monostate> split_lut(Gate* g, bool create_module)
        {
            if (g->get_type()->get_name() != "LUT6_2")
            {
                return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " is not of type LUT6_2");
            }

            Netlist* nl     = g->get_netlist();
            auto* lut5_type = nl->get_gate_library()->get_gate_type_by_name("LUT5");
            auto* lut6_type = nl->get_gate_library()->get_gate_type_by_name("LUT6");

            auto* o5 = g->get_fan_out_net("O5");
            auto* o6 = g->get_fan_out_net("O6");

            // Determine target module for replacement gates.
            Module* target_mod = nullptr;
            if (create_module)
            {
                target_mod = nl->create_module(g->get_name(), g->get_module());
            }
            else if (!g->get_module()->is_top_module())
            {
                target_mod = g->get_module();
            }

            // LUT6_2 uses a 64-bit INIT string (16 hex chars): O6 uses all 64 bits,
            // O5 uses bits [0, 31] (the lower half, i.e., truth table for I5=0).
            if (o5 != nullptr && o5->get_num_of_destinations() > 0)
            {
                const auto init_O5_res = g->get_init_string("O5");
                if (init_O5_res.is_error())
                {
                    log_warning("xilinx_toolbox", "could not get INIT string for O5 of gate '{}' with ID {}, skipping O5 split.", g->get_name(), g->get_id());
                }
                else
                {
                    const auto init_O5 = init_O5_res.get();
                    auto* lut5         = nl->create_gate(lut5_type, g->get_name() + "_split_O5");
                    lut5->set_data("xilinx_preprocessing_information", "original_init", "string", init_O5);

                    if (lut5->set_init_string("O", init_O5).is_error())
                    {
                        log_warning("xilinx_toolbox", "could not set INIT string of gate '{}' with ID {}.", lut5->get_name(), lut5->get_id());
                        nl->delete_gate(lut5);
                    }
                    else
                    {
                        if (target_mod != nullptr)
                        {
                            target_mod->assign_gate(lut5);
                        }

                        // I5 selects the upper/lower half of the INIT; LUT5 only uses the lower
                        // half (I5=0), so I5 is not a real input and is excluded here.
                        for (const auto& in_ep : g->get_fan_in_endpoints())
                        {
                            if (in_ep->get_pin()->get_name() == "I5")
                            {
                                continue;
                            }

                            in_ep->get_net()->add_destination(lut5, in_ep->get_pin()->get_name());
                        }

                        o5->add_source(lut5, "O");
                    }
                }
            }

            if (o6 != nullptr && o6->get_num_of_destinations() > 0)
            {
                const auto init_O6_res = g->get_init_string("O6");
                if (init_O6_res.is_error())
                {
                    return ERR("could not get O6 INIT string of gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()));
                }
                else
                {
                    const auto init_O6 = init_O6_res.get();
                    auto* lut6         = nl->create_gate(lut6_type, g->get_name() + "_split_O6");
                    lut6->set_data("xilinx_preprocessing_information", "original_init", "string", init_O6);

                    // O6 is a full 6-input LUT and uses the entire 64-bit INIT string.
                    if (lut6->set_init_string("O", init_O6).is_error())
                    {
                        log_warning("xilinx_toolbox", "could not set INIT string of gate '{}' with ID {}.", lut6->get_name(), lut6->get_id());
                        nl->delete_gate(lut6);
                    }
                    else
                    {
                        if (target_mod != nullptr)
                        {
                            target_mod->assign_gate(lut6);
                        }

                        for (const auto& in_ep : g->get_fan_in_endpoints())
                        {
                            in_ep->get_net()->add_destination(lut6, in_ep->get_pin()->get_name());
                        }

                        o6->add_source(lut6, "O");
                    }
                }
            }

            // Always delete the original LUT6_2 regardless of which outputs were used.
            if (!nl->delete_gate(g))
            {
                return ERR("failed to delete gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()));
            }

            return OK({});
        }

        Result<u32> split_luts(const std::vector<Gate*>& gates, bool create_module)
        {
            u32 count = 0;
            for (auto* g : gates)
            {
                if (auto res = split_lut(g, create_module); res.is_error())
                {
                    log_warning("xilinx_toolbox", "skipping gate '{}' with ID {}: {}", g->get_name(), g->get_id(), res.get_error().get());
                }
                else
                {
                    count++;
                }
            }
            log_info("xilinx_toolbox", "split {} of {} LUT6_2 gates into LUT6 and LUT5 gates", count, gates.size());
            return OK(count);
        }

        Result<u32> split_luts(Netlist* nl, bool create_module)
        {
            return split_luts(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6_2"; }), create_module);
        }

        Result<std::monostate> split_shift_register(Gate* g, bool create_module)
        {
            const std::string type_name = g->get_type()->get_name();
            if (!is_srl_type(type_name))
            {
                return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " is not a supported shift register type (SRL16, SRL16E, SRLC16E, SRLC32E)");
            }

            Netlist* nl     = g->get_netlist();
            GateType* ff_gt = nl->get_gate_library()->get_gate_type_by_name("FDCE");
            if (ff_gt == nullptr)
            {
                return ERR("could not find gate type 'FDCE' in gate library");
            }

            // --- Address (control) pins: sort A0 < A1 < ... ---
            auto ctrl_pins = g->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::control; });

            const u32 expected_ctrl = (type_name == "SRLC32E") ? 5u : 4u;
            if (ctrl_pins.size() != expected_ctrl)
            {
                return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " has " + std::to_string(ctrl_pins.size()) + " control pins, expected "
                           + std::to_string(expected_ctrl));
            }

            std::sort(ctrl_pins.begin(), ctrl_pins.end(), [](const GatePin* a, const GatePin* b) { return srl_addr_pin_index(a) < srl_addr_pin_index(b); });

            // Compute select_value from constant address pins (binary, little-endian: A0 = bit 0).
            // Stage select_value is where Q is tapped; depth = select_value + 1 clock cycles.
            u32 select_value = 0;
            for (u32 i = 0; i < ctrl_pins.size(); i++)
            {
                const Net* cn = g->get_fan_in_net(ctrl_pins[i]);
                if (cn == nullptr)
                {
                    return ERR("control pin '" + ctrl_pins[i]->get_name() + "' of gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " has no connected net");
                }
                if (!cn->is_gnd_net() && !cn->is_vcc_net())
                {
                    return ERR("control pin '" + ctrl_pins[i]->get_name() + "' of gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " is not driven by a constant (GND/VCC) net");
                }
                if (cn->is_vcc_net())
                {
                    select_value |= (1u << i);
                }
            }

            // --- Clock / enable / data pins ---
            const auto clk_pins = g->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::clock; });
            if (clk_pins.size() != 1)
            {
                return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " has " + std::to_string(clk_pins.size()) + " clock pins, expected 1");
            }

            const auto en_pins = g->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::enable; });
            if (en_pins.size() != 1)
            {
                return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " has " + std::to_string(en_pins.size()) + " enable pins, expected 1");
            }

            const auto data_pins = g->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::data; });
            if (data_pins.size() != 1)
            {
                return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " has " + std::to_string(data_pins.size()) + " data pins, expected 1");
            }

            Net* clk_in    = g->get_fan_in_net(clk_pins.front());
            Net* enable_in = g->get_fan_in_net(en_pins.front());
            Net* data_in   = g->get_fan_in_net(data_pins.front());

            if (clk_in == nullptr)
            {
                return ERR("no clock input connected to gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()));
            }
            if (enable_in == nullptr)
            {
                return ERR("no enable input connected to gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()));
            }
            if (data_in == nullptr)
            {
                return ERR("no data input connected to gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()));
            }

            // --- Output nets ---
            const std::string casc_pin = srl_cascade_pin(type_name);
            Net* state_out             = g->get_fan_out_net("Q");
            Net* cascade_out           = casc_pin.empty() ? nullptr : g->get_fan_out_net(casc_pin);

            // Cascade output is only "used" when it has actual downstream consumers.
            const bool cascade_used = (cascade_out != nullptr && cascade_out->get_num_of_destinations() > 0);

            if (state_out == nullptr && !cascade_used)
            {
                return ERR("no output connected to gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()));
            }

            // When the cascade output is used, all stages up to the maximum must be materialized
            // (SRLC16E: 0..15, SRLC32E: 0..31) so that Q15/Q31 reflects the true last stage.
            const u32 max_depth     = (1u << ctrl_pins.size()) - 1u;
            const u32 register_size = cascade_used ? max_depth : select_value;

            // --- Build the FDE chain ---
            Module* parent_mod = g->get_module();

            // Determine target module for replacement FFs.
            Module* target_mod = nullptr;
            if (create_module)
            {
                target_mod = nl->create_module(g->get_name(), parent_mod);
            }
            else if (!parent_mod->is_top_module())
            {
                target_mod = parent_mod;
            }

            // Pre-extract parameters to copy to each FF (evaluated once, applied per-FF in the chain).
            // INIT is an N-bit SRL reset value: bit i becomes the 1-bit INIT of FF i.
            // IS_CLK_INVERTED is 1-bit and the same for all FFs; mapped to IS_C_INVERTED on FDCE.
            // Both are forwarded silently — no error if either side lacks the declaration.
            const bool has_clk_inv    = g->has_parameter("IS_CLK_INVERTED");
            const std::string clk_inv_val = has_clk_inv ? g->get_parameter_value("IS_CLK_INVERTED").get() : "";

            bool has_init    = g->has_parameter("INIT");
            u64 init_bits    = 0;
            if (has_init)
            {
                try
                {
                    init_bits = std::stoull(g->get_parameter_value("INIT").get(), nullptr, 0);
                }
                catch (...)
                {
                    log_warning("xilinx_toolbox", "could not parse INIT of gate '{}' with ID {}, skipping INIT copy", g->get_name(), g->get_id());
                    has_init = false;
                }
            }

            std::vector<Net*> chain_nets;    // Q output net of each FF, used to wire D of next

            for (u32 i = 0; i <= register_size; i++)
            {
                const std::string ff_name = g->get_name() + "_split_ff_" + std::to_string(i);
                Gate* ff                  = nl->create_gate(ff_gt, ff_name);

                if (target_mod != nullptr)
                {
                    target_mod->assign_gate(ff);
                }

                if (has_init)
                {
                    if (auto decl = ff->get_type()->get_parameter("INIT"); decl.is_ok())
                    {
                        const std::string bit_val = ((init_bits >> i) & 1u) ? "0x1" : "0x0";
                        if (ff->set_parameter(decl.get(), bit_val).is_error())
                        {
                            log_warning("xilinx_toolbox", "could not copy INIT bit {} to FF '{}' with ID {}", i, ff->get_name(), ff->get_id());
                        }
                    }
                }
                if (has_clk_inv)
                {
                    if (auto decl = ff->get_type()->get_parameter("IS_C_INVERTED"); decl.is_ok())
                    {
                        if (ff->set_parameter(decl.get(), clk_inv_val).is_error())
                        {
                            log_warning("xilinx_toolbox", "could not copy IS_CLK_INVERTED to IS_C_INVERTED on FF '{}' with ID {}", ff->get_name(), ff->get_id());
                        }
                    }
                }

                clk_in->add_destination(ff, "C");
                enable_in->add_destination(ff, "CE");

                (i == 0 ? data_in : chain_nets.back())->add_destination(ff, "D");

                const bool is_q_stage       = (state_out != nullptr && i == select_value);
                const bool is_cascade_stage = (cascade_used && i == register_size);

                if (is_q_stage)
                {
                    state_out->add_source(ff, "Q");
                    if (is_cascade_stage)
                    {
                        // Q and cascade tap the same stage: a single FF output pin can source only
                        // one net, so merge cascade_out's consumers into state_out.
                        // Capture gate/pin BEFORE removal since remove_destination frees the Endpoint.
                        std::vector<std::pair<Gate*, std::string>> casc_consumers;
                        for (const Endpoint* ep : cascade_out->get_destinations())
                        {
                            casc_consumers.emplace_back(ep->get_gate(), ep->get_pin()->get_name());
                        }
                        for (const auto& [g, pin] : casc_consumers)
                        {
                            cascade_out->remove_destination(g, pin);
                            state_out->add_destination(g, pin);
                        }
                    }
                    chain_nets.push_back(state_out);
                }
                else if (is_cascade_stage)
                {
                    cascade_out->add_source(ff, "Q");
                    chain_nets.push_back(cascade_out);
                }
                else
                {
                    Net* internal = nl->create_net(ff_name + "_out");
                    internal->add_source(ff, "Q");
                    chain_nets.push_back(internal);
                }
            }

            if (!nl->delete_gate(g))
            {
                return ERR("failed to delete gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()));
            }

            return OK({});
        }

        Result<u32> split_shift_registers(const std::vector<Gate*>& gates, bool create_module)
        {
            u32 count = 0;
            for (Gate* g : gates)
            {
                if (auto res = split_shift_register(g, create_module); res.is_error())
                {
                    log_warning("xilinx_toolbox", "skipping gate '{}' with ID {}: {}", g->get_name(), g->get_id(), res.get_error().get());
                }
                else
                {
                    count++;
                }
            }
            log_info("xilinx_toolbox", "split {} of {} shift register gates into FDE flip-flop chains", count, gates.size());
            return OK(count);
        }

        Result<u32> split_shift_registers(Netlist* nl, bool create_module)
        {
            return split_shift_registers(nl->get_gates([](const Gate* g) { return is_srl_type(g->get_type()->get_name()); }), create_module);
        }
    }    // namespace xilinx_toolbox
}    // namespace hal
