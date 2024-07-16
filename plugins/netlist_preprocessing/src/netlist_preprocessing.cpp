#include "netlist_preprocessing/netlist_preprocessing.h"

#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/token_stream.h"
#include "rapidjson/document.h"
#include "resynthesis/resynthesis.h"

#include <fstream>
#include <regex>

namespace hal
{
    namespace netlist_preprocessing
    {
        namespace
        {
            Result<u32> remove_encasing_inverters(Netlist* nl)
            {
                // check whether all inputs and output are inverted -> remove all inverters

                // TODO: this only considers HAL muxes, but i do not see a reason why. There is no resynthesis happening here
                std::vector<Gate*> muxes = nl->get_gates([](const Gate* g) { return (g->get_type()->get_name().find("HAL_MUX") != std::string::npos); });

                u32 delete_count = 0;
                std::vector<Gate*> delete_gate_q;

                for (const auto& g : muxes)
                {
                    if (g->get_successors().size() > 1)
                    {
                        continue;
                    }

                    auto data_pins = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_type() == PinType::data) && (pin->get_direction() == PinDirection::input); });
                    auto out_pins  = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_direction() == PinDirection::output); });

                    if (data_pins.size() < 2)
                    {
                        continue;
                    }

                    if (out_pins.size() != 1)
                    {
                        continue;
                    }

                    bool preceded_by_inv = true;
                    for (const auto& pin : data_pins)
                    {
                        const auto pred = g->get_predecessor(pin);
                        if (pred == nullptr || pred->get_gate() == nullptr || !pred->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter))
                        {
                            preceded_by_inv = false;
                            break;
                        }
                    }

                    if (!preceded_by_inv)
                    {
                        continue;
                    }

                    bool succeded_by_inv = true;
                    for (const auto& pin : out_pins)
                    {
                        const auto suc = g->get_successor(pin);
                        if (suc == nullptr || suc->get_gate() == nullptr || !suc->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter))
                        {
                            succeded_by_inv = false;
                            break;
                        }
                    }

                    if (!succeded_by_inv)
                    {
                        continue;
                    }

                    // delete all connections from and to inverters (and inverter gates if they do not share any other connection)
                    for (const auto& pin : data_pins)
                    {
                        const auto pred = g->get_predecessor(pin);

                        // disconnect inverter output from mux
                        pred->get_net()->remove_destination(g, pin);

                        // connect inverter input net to mux
                        auto in_net = pred->get_gate()->get_fan_in_nets().front();
                        in_net->add_destination(g, pin);

                        // delete inverter gate if it does not have any successors
                        if (pred->get_gate()->get_successors().empty())
                        {
                            delete_gate_q.push_back(pred->get_gate());
                        }
                    }

                    for (const auto& pin : out_pins)
                    {
                        const auto suc = g->get_successor(pin);

                        // disconnect inverter input from mux
                        suc->get_net()->remove_source(g, pin);

                        // connect inverter output net to mux
                        auto in_net = suc->get_gate()->get_fan_out_nets().front();
                        in_net->add_source(g, pin);

                        // delete inverter gate if it does not have any predecessors
                        if (suc->get_gate()->get_predecessors().empty())
                        {
                            delete_gate_q.push_back(suc->get_gate());
                        }
                    }
                }

                for (auto g : delete_gate_q)
                {
                    nl->delete_gate(g);
                    delete_count++;
                }

                log_info("netlist_preprocessing", "removed {} encasing inverters", delete_count);

                return OK(delete_count);
            }

            struct MuxFingerprint
            {
                GateType* type;
                std::set<GatePin*> inverters;

                bool operator<(const MuxFingerprint& other) const
                {
                    return (other.type < type) || (other.type == type && other.inverters < inverters);
                }
            };

            Result<u32> unify_inverted_select_signals(Netlist* nl, GateLibrary* mux_inv_gl)
            {
                if (nl == nullptr)
                {
                    return ERR("netlist is a nullptr");
                }

                if (mux_inv_gl == nullptr)
                {
                    return ERR("gate library is a nullptr");
                }

                auto base_path_res = utils::get_unique_temp_directory("resynthesis_");
                if (base_path_res.is_error())
                {
                    return ERR_APPEND(base_path_res.get_error(), "unable to resynthesize boolean functions with yosys: failed to get unique temp directory");
                }
                const std::filesystem::path base_path   = base_path_res.get();
                const std::filesystem::path genlib_path = base_path / "mux_inv.genlib";
                std::filesystem::create_directory(base_path);

                const auto gl_save_res = gate_library_manager::save(genlib_path, mux_inv_gl, true);
                if (!gl_save_res)
                {
                    return ERR("unable to unify muxe select signals: failed to save gate library " + mux_inv_gl->get_name() + " to location " + genlib_path.string());
                }

                const i64 initial_size = nl->get_gates().size();

                // resynthesize all muxes where any select signal is preceded by an inverter hoping to unify the structure with regards to other muxes conntected to the same select signal

                // TODO: as long as resynthezising the subgraph this can only consider HAL muxes
                std::vector<Gate*> muxes = nl->get_gates([](const Gate* g) { return (g->get_type()->get_name().find("HAL_MUX") != std::string::npos); });

                std::map<MuxFingerprint, std::unique_ptr<Netlist>> resynth_cache;

                for (const auto& g : muxes)
                {
                    // MUX fingerprint for caching resynthesis results
                    MuxFingerprint mf;
                    mf.type = g->get_type();

                    // mapping from MUX select pins to either the input net of the preceding inverter or the net directly connected to the select pin
                    std::map<GatePin*, Net*> pin_to_input;

                    auto select_pins = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_type() == PinType::select) && (pin->get_direction() == PinDirection::input); });

                    std::vector<Gate*> preceding_inverters;
                    for (const auto& pin : g->get_type()->get_input_pins())
                    {
                        const auto pred      = g->get_predecessor(pin);
                        const auto is_select = (std::find(select_pins.begin(), select_pins.end(), pin) != select_pins.end());
                        if (!is_select || pred == nullptr || pred->get_gate() == nullptr || !pred->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter)
                            || (pred->get_gate()->get_fan_in_endpoints().size() != 1))
                        {
                            pin_to_input.insert({pin, g->get_fan_in_net(pin)});
                        }
                        else
                        {
                            auto inv_gate = pred->get_gate();
                            preceding_inverters.push_back(inv_gate);
                            pin_to_input.insert({pin, inv_gate->get_fan_in_endpoints().front()->get_net()});
                            mf.inverters.insert(pin);
                        }
                    }

                    // if there is at least one inverter in front of the mux gate we build a subgraph containing all inverters and the mux gate and resynthesize
                    if (!preceding_inverters.empty())
                    {
                        const Netlist* resynth_nl;

                        auto subgraph = preceding_inverters;
                        subgraph.push_back(g);

                        // try to use cached resynth netlist
                        if (const auto it = resynth_cache.find(mf); it == resynth_cache.end())
                        {
                            std::unordered_map<std::string, BooleanFunction> bfs;
                            for (const auto& ep : g->get_fan_out_endpoints())
                            {
                                const auto bf_res = SubgraphNetlistDecorator(*nl).get_subgraph_function(subgraph, ep->get_net());
                                if (bf_res.is_error())
                                {
                                    return ERR_APPEND(bf_res.get_error(),
                                                      "unable to unify muxes select signals: failed to build boolean function for mux " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                                          + "  at output " + ep->get_pin()->get_name());
                                }
                                auto bf = bf_res.get();

                                // replace all net id vars with generic vaiables refering to their connectivity to the mux
                                for (const auto& [pin, net] : pin_to_input)
                                {
                                    auto sub_res = bf.substitute(BooleanFunctionNetDecorator(*net).get_boolean_variable_name(), BooleanFunction::Var(pin->get_name(), 1));
                                    if (sub_res.is_error())
                                    {
                                        return ERR_APPEND(sub_res.get_error(), "unable to unify muxes select signals: failed to substitute net_id variable with generic variable");
                                    }
                                    bf = sub_res.get();
                                }

                                bfs.insert({ep->get_pin()->get_name(), std::move(bf)});
                            }

                            auto resynth_res = resynthesis::generate_resynth_netlist_for_boolean_functions(bfs, genlib_path, mux_inv_gl, true);
                            if (resynth_res.is_error())
                            {
                                return ERR_APPEND(resynth_res.get_error(), "unable to unify  select signals of muxes: failed to resynthesize mux subgraph to netlist");
                            }
                            auto unique_resynth_nl = resynth_res.get();
                            resynth_nl             = unique_resynth_nl.get();
                            resynth_cache.insert({mf, std::move(unique_resynth_nl)});
                        }
                        else
                        {
                            resynth_nl = it->second.get();
                        }

                        std::unordered_map<Net*, std::vector<Net*>> global_io_mapping;

                        // use top module pin names to find correponding nets in original netlist
                        for (const auto& pin : resynth_nl->get_top_module()->get_input_pins())
                        {
                            auto net_it = pin_to_input.find(g->get_type()->get_pin_by_name(pin->get_name()));
                            if (net_it == pin_to_input.end())
                            {
                                return ERR("unable to unify muxes select signals:: failed to locate net in destination netlist from global input " + pin->get_name() + " in resynthesized netlist");
                            }
                            global_io_mapping[pin->get_net()].push_back(net_it->second);
                        }
                        for (const auto& pin : resynth_nl->get_top_module()->get_output_pins())
                        {
                            auto net = g->get_fan_out_net(pin->get_name());
                            if (net == nullptr)
                            {
                                return ERR("unable to unify muxes select signals:: failed to locate net in destination netlist from global output " + pin->get_name() + " in resynthesized netlist");
                            }
                            global_io_mapping[pin->get_net()].push_back(net);
                        }

                        auto replace_res = resynthesis::replace_subgraph_with_netlist(subgraph, global_io_mapping, resynth_nl, nl, false);
                        if (replace_res.is_error())
                        {
                            return ERR("unable to unify muxes select signals: failed to replace mux subgraph with resynthesized netlist");
                        }

                        // delete old subgraph gates that only fed into the mux
                        std::vector<Gate*> to_delete;
                        for (const auto g : subgraph)
                        {
                            bool has_no_outside_destinations   = true;
                            bool has_only_outside_destinations = true;
                            for (const auto& suc : g->get_successors())
                            {
                                const auto it = std::find(subgraph.begin(), subgraph.end(), suc->get_gate());
                                if (it == subgraph.end())
                                {
                                    has_no_outside_destinations = false;
                                }

                                if (it != subgraph.end())
                                {
                                    has_only_outside_destinations = false;
                                }
                            }

                            if (has_no_outside_destinations || has_only_outside_destinations)
                            {
                                to_delete.push_back(g);
                            }
                        }

                        for (const auto& g : to_delete)
                        {
                            if (!nl->delete_gate(g))
                            {
                                return ERR("unable to unify muxes select signals: failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " in destination netlist");
                            }
                        }
                    }
                }

                // delete the created directory and the contained files
                std::filesystem::remove_all(base_path);

                const i64 new_size   = nl->get_gates().size();
                const i64 difference = std::abs(initial_size - new_size);

                return OK(u32(difference));
            }

            Result<u32> unify_select_signals(Netlist* nl)
            {
                if (nl == nullptr)
                {
                    return ERR("netlist is a nullptr");
                }

                u32 changed_connections = 0;

                // sort into groups of same type and identical select signals
                std::map<std::pair<GateType*, std::set<Net*>>, std::vector<Gate*>> grouped_muxes;
                for (const auto& g : nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_mux); }))
                {
                    std::set<Net*> select_signals;
                    const auto select_pins = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_type() == PinType::select) && (pin->get_direction() == PinDirection::input); });
                    for (const auto& sp : select_pins)
                    {
                        select_signals.insert(g->get_fan_in_net(sp));
                    }

                    grouped_muxes[{g->get_type(), select_signals}].push_back(g);
                }

                // unify select signals for each group
                for (const auto& [finger_print, mux_group] : grouped_muxes)
                {
                    const auto& [type, select_signals_set] = finger_print;
                    const auto select_pins                 = type->get_pins([](const GatePin* pin) { return (pin->get_type() == PinType::select) && (pin->get_direction() == PinDirection::input); });
                    const auto output_pins                 = type->get_pins([](const GatePin* pin) { return pin->get_direction() == PinDirection::output; });

                    if (output_pins.size() != 1)
                    {
                        log_warning("netlist_preprocessing",
                                    "Cannot unify select signals for muxes of type {} since the type has {} output signals and we can only handle 1.",
                                    type->get_name(),
                                    output_pins.size());
                        continue;
                    }

                    // check whether there is one mapping from select signals to select pins
                    std::map<std::map<GatePin*, Net*>, std::vector<Gate*>> select_map_to_muxes;
                    for (const auto& g : mux_group)
                    {
                        std::map<GatePin*, Net*> select_map;
                        for (const auto& sp : select_pins)
                        {
                            select_map.insert({sp, g->get_fan_in_net(sp)});
                        }

                        select_map_to_muxes[select_map].push_back(g);
                    }

                    if (select_map_to_muxes.size() == 1)
                    {
                        continue;
                    }

                    const std::vector<Net*> select_signals = {select_signals_set.begin(), select_signals_set.end()};

                    // collect a new mapping from net to gate pin for each mux gate
                    std::map<Gate*, std::map<GatePin*, Net*>> new_net_to_pin;

                    // add newly ordered select signals to pin/net mapping
                    for (const auto& g : mux_group)
                    {
                        for (u32 select_index = 0; select_index < select_pins.size(); select_index++)
                        {
                            auto select_pin               = select_pins.at(select_index);
                            auto select_signal            = select_signals.at(select_index);
                            new_net_to_pin[g][select_pin] = select_signal;
                        }
                    }

                    // determine new pin/net connection for each "data" signal
                    auto type_bf = type->get_boolean_function(output_pins.front());
                    for (u32 select_val = 0; select_val < (1 << select_signals.size()); select_val++)
                    {
                        std::map<std::string, BooleanFunction> type_substitution;
                        for (u32 select_idx = 0; select_idx < select_pins.size(); select_idx++)
                        {
                            auto select_pin = select_pins.at(select_idx);

                            auto type_substitution_val = ((select_val >> select_idx) & 0x1) ? BooleanFunction::Const(1, 1) : BooleanFunction::Const(0, 1);
                            type_substitution.insert({select_pin->get_name(), type_substitution_val});
                        }

                        auto type_substitution_res = type_bf.substitute(type_substitution);
                        if (type_substitution_res.is_error())
                        {
                            return ERR_APPEND(type_substitution_res.get_error(), "cannot unify mux select signals: failed to substitute type Boolean function with select signal value mapping.");
                        }
                        auto input = type_substitution_res.get().simplify_local();

                        if (!input.is_variable())
                        {
                            return ERR("cannot unify mux select signals: substituted and simplified type Boolean function (" + input.to_string() + ") is not a variable");
                        }

                        const auto pin_name = input.get_variable_name().get();
                        auto pin            = type->get_pins([pin_name](const auto& p) { return p->get_name() == pin_name; }).front();

                        for (const auto& g : mux_group)
                        {
                            auto gate_bf_res = g->get_resolved_boolean_function(output_pins.front(), false);
                            if (gate_bf_res.is_error())
                            {
                                return ERR_APPEND(gate_bf_res.get_error(),
                                                  "cannot unify mux select signals: failed to build Boolean function for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                            }
                            auto gate_bf = gate_bf_res.get();

                            std::map<std::string, BooleanFunction> gate_substitution;

                            for (u32 select_idx = 0; select_idx < select_pins.size(); select_idx++)
                            {
                                auto gate_substitution_val = ((select_val >> select_idx) & 0x1) ? BooleanFunction::Const(1, 1) : BooleanFunction::Const(0, 1);
                                gate_substitution.insert({BooleanFunctionNetDecorator(*(select_signals.at(select_idx))).get_boolean_variable_name(), gate_substitution_val});
                            }

                            auto gate_substitution_res = gate_bf.substitute(gate_substitution);
                            if (gate_substitution_res.is_error())
                            {
                                return ERR_APPEND(gate_substitution_res.get_error(), "cannot unify mux select signals: failed to substitute gate Boolean function with select signal value mapping.");
                            }
                            auto input_net_var = gate_substitution_res.get().simplify_local();
                            auto net_res       = BooleanFunctionNetDecorator::get_net_from(nl, input_net_var);

                            if (net_res.is_error())
                            {
                                return ERR_APPEND(net_res.get_error(), "cannot unify mux select signals: failed to extract net from substituted and simplified gate Boolean function");
                            }

                            auto net               = net_res.get();
                            new_net_to_pin[g][pin] = net;
                        }
                    }

                    // apply new pin/net mapping to all gates
                    for (auto& [g, pin_net] : new_net_to_pin)
                    {
                        for (const auto& [pin, net] : pin_net)
                        {
                            auto connected_net = g->get_fan_in_net(pin);
                            if (net == connected_net)
                            {
                                continue;
                            }

                            connected_net->remove_destination(g, pin);
                            net->add_destination(g, pin);

                            changed_connections += 1;
                        }
                    }
                }

                return OK(changed_connections);
            }
        }    // namespace

        Result<u32> manual_mux_optimizations(Netlist* nl, GateLibrary* mux_inv_gl)
        {
            u32 res_count = 0;

            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            if (mux_inv_gl == nullptr)
            {
                return ERR("gate library is a nullptr");
            }

            auto remove_res = remove_encasing_inverters(nl);
            if (remove_res.is_error())
            {
                return ERR_APPEND(remove_res.get_error(), "unable to apply manual mux optimizations: failed to remove encasing inverters");
            }
            res_count += remove_res.get();

            auto unify_inverted_res = unify_inverted_select_signals(nl, mux_inv_gl);
            if (unify_inverted_res.is_error())
            {
                return ERR_APPEND(unify_inverted_res.get_error(), "unable to apply manual mux optimizations: failed to unify inverted select signals");
            }
            res_count += unify_inverted_res.get();

            auto unify_res = unify_select_signals(nl);
            if (unify_res.is_error())
            {
                return ERR_APPEND(unify_res.get_error(), "unable to apply manual mux optimizations: failed to unify select signals");
            }
            res_count += unify_res.get();

            return OK(res_count);
        }

        Result<u32> propagate_constants(Netlist* nl)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            Net* gnd_net = nl->get_gnd_gates().empty() ? nullptr : nl->get_gnd_gates().front()->get_fan_out_nets().front();
            Net* vcc_net = nl->get_vcc_gates().empty() ? nullptr : nl->get_vcc_gates().front()->get_fan_out_nets().front();

            u32 total_replaced_dst_count = 0;

            while (true)
            {
                u32 replaced_dst_count = 0;
                std::vector<Gate*> to_delete;
                for (const auto g : nl->get_gates([](const auto g) {
                         return g->get_type()->has_property(GateTypeProperty::combinational) && !g->get_type()->has_property(GateTypeProperty::ground)
                                && !g->get_type()->has_property(GateTypeProperty::power);
                     }))
                {
                    bool has_global_output = false;
                    for (const auto ep : g->get_fan_out_endpoints())
                    {
                        if (ep->get_net()->is_global_output_net())
                        {
                            has_global_output = true;
                        }

                        auto bf_res = g->get_resolved_boolean_function(ep->get_pin(), false);
                        if (bf_res.is_error())
                        {
                            return ERR_APPEND(bf_res.get_error(),
                                              "unable to propagate constants: failed to generate boolean function at gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " for pin "
                                                  + ep->get_pin()->get_name());
                        }
                        auto bf      = bf_res.get();
                        auto sub_res = BooleanFunctionDecorator(bf).substitute_power_ground_nets(nl);
                        if (sub_res.is_error())
                        {
                            return ERR_APPEND(bf_res.get_error(),
                                              "unable to propagate constants: failed to substitue power and ground nets in boolean function of gate " + g->get_name() + " with ID "
                                                  + std::to_string(g->get_id()) + " for pin " + ep->get_pin()->get_name());
                        }
                        bf = sub_res.get();
                        bf = bf.simplify_local();

                        // if boolean function of output pin can be simplified to a constant connect all its successors to gnd/vcc instead
                        if (bf.is_constant())
                        {
                            Net* new_source;
                            if (bf.has_constant_value(0))
                            {
                                new_source = gnd_net;
                            }
                            else if (bf.has_constant_value(1))
                            {
                                new_source = vcc_net;
                            }
                            else
                            {
                                continue;
                            }

                            if (new_source == nullptr)
                            {
                                // log_error("netlist_preprocessing", "failed to replace bf {} with constant net because netlist is missing GND gate or VCC gate");
                                return ERR("unable to propagate constants: netlist is missing gnd or vcc net!");
                            }

                            std::vector<std::pair<Gate*, GatePin*>> to_replace;
                            for (auto dst : ep->get_net()->get_destinations())
                            {
                                to_replace.push_back({dst->get_gate(), dst->get_pin()});
                            }

                            for (const auto& [dst_g, dst_p] : to_replace)
                            {
                                ep->get_net()->remove_destination(dst_g, dst_p);
                                new_source->add_destination(dst_g, dst_p);

                                replaced_dst_count++;
                            }

                            nl->delete_net(ep->get_net());
                        }
                    }

                    if (!has_global_output && g->get_successors().empty())
                    {
                        to_delete.push_back(g);
                    }
                }

                for (auto g : to_delete)
                {
                    nl->delete_gate(g);
                }

                if (replaced_dst_count == 0)
                {
                    break;
                }

                log_debug("netlist_preprocessing", "replaced {} destinations this with power/ground nets this iteration", replaced_dst_count);
                total_replaced_dst_count += replaced_dst_count;
            }

            log_info("netlist_preprocessing", "replaced {} destinations with power/ground nets in total", total_replaced_dst_count);
            return OK(total_replaced_dst_count);
        }

        namespace
        {
            std::string generate_hex_truth_table_string(const std::vector<BooleanFunction::Value>& tt)
            {
                std::string tt_str = "";

                u32 acc = 0;
                for (u32 i = 0; i < tt.size(); i++)
                {
                    const BooleanFunction::Value bit = tt.at(i);
                    if (bit == BooleanFunction::Value::ONE)
                    {
                        acc += (1 << (i % 4));
                    }

                    if ((i % 4) == 3)
                    {
                        std::stringstream stream;
                        stream << std::hex << acc;

                        tt_str = stream.str() + tt_str;

                        acc = 0;
                    }
                }

                return tt_str;
            }
        }    // namespace

        Result<u32> simplify_lut_inits(Netlist* nl)
        {
            u32 num_inits = 0;

            for (auto g : nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
            {
                auto res = g->get_init_data();
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "unable to simplify lut init string for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to get original INIT string");
                }

                const auto original_inits = res.get();

                if (original_inits.size() != 1)
                {
                    return ERR("unable to simplify lut init string for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": found " + std::to_string(original_inits.size())
                               + " init data strings but expected exactly 1.");
                }

                const auto original_init = original_inits.front();

                // skip if the gate type has more than one fan out endpoints
                if (g->get_type()->get_output_pins().size() != 1)
                {
                    continue;
                }

                const auto out_ep = g->get_fan_out_endpoints().front();

                // skip if the gate has more than one boolean function
                if (g->get_boolean_functions().size() != 1)
                {
                    continue;
                }

                const auto bf_org   = g->get_boolean_function(out_ep->get_pin());
                const auto org_vars = bf_org.get_variable_names();

                const auto bf_replaced_res = BooleanFunctionDecorator(bf_org).substitute_power_ground_pins(g);
                if (bf_replaced_res.is_error())
                {
                    return ERR_APPEND(bf_replaced_res.get_error(),
                                      "cannot simplify LUT inits: failed to replace power and ground pins for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                }
                const auto bf_replaced   = bf_replaced_res.get();
                const auto bf_simplified = bf_replaced.simplify_local();

                const auto new_vars = bf_simplified.get_variable_names();

                if (org_vars.size() == new_vars.size())
                {
                    continue;
                }

                auto bf_extended = bf_simplified.clone();
                for (const auto& in_pin : g->get_type()->get_input_pin_names())
                {
                    if (new_vars.find(in_pin) == new_vars.end())
                    {
                        auto bf_filler = BooleanFunction::Var(in_pin) | (~BooleanFunction::Var(in_pin));
                        bf_extended    = BooleanFunction::And(std::move(bf_extended), std::move(bf_filler), 1).get();
                    }
                }

                const auto tt              = bf_extended.compute_truth_table().get();
                const auto new_init_string = generate_hex_truth_table_string(tt.front());

                // std::cout << "Org Init: " << g->get_init_data().get().front() << std::endl;
                // std::cout << "New Init: " << new_init_string << std::endl;

                g->set_init_data({new_init_string}).get();
                g->set_data("preprocessing_information", "original_init", "string", original_init);

                // const auto bf_test = g->get_boolean_function(out_ep->get_pin());

                // std::cout << "Org: " << bf_org << std::endl;
                // std::cout << "Rep: " << bf_replaced << std::endl;
                // std::cout << "Simp: " << bf_simplified << std::endl;
                // std::cout << "Test: " << bf_test << std::endl;
                // std::cout << "Ext: " << bf_extended << std::endl;

                num_inits++;
            }

            log_info("netlist_preprocessing", "simplified {} LUT INIT strings inside of netlist with ID {}.", num_inits, nl->get_id());
            return OK(num_inits);
        }

        namespace
        {
            struct indexed_identifier
            {
                indexed_identifier(const std::string& p_identifier, const u32 p_index, const std::string& p_origin) : identifier{p_identifier}, index{p_index}, origin{p_origin}
                {
                }

                std::string identifier;
                u32 index;
                std::string origin;
            };

            // TODO when the verilog parser changes are merged into the master this will no longer be needed
            const std::string hal_instance_index_pattern         = "__\\[(\\d+)\\]__";
            const std::string hal_instance_index_pattern_reverse = "<HAL>(\\d+)<HAL>";

            std::string replace_hal_instance_index(const std::string& name)
            {
                std::regex re(hal_instance_index_pattern);

                std::string input = name;
                std::string index;
                std::smatch match;
                while (std::regex_search(input, match, re))
                {
                    index = match[1];
                    input = utils::replace(input, match.str(), "<HAL>" + index + "<HAL>");
                }

                return input;
            }

            std::string reconstruct_hal_instance_index(const std::string& name)
            {
                std::regex re(hal_instance_index_pattern_reverse);

                std::string input = name;
                std::string index;
                std::smatch match;
                while (std::regex_search(input, match, re))
                {
                    index = match[1];
                    input = utils::replace(input, match.str(), "__[" + index + "]__");
                }

                return input;
            }

            const std::string net_index_pattern  = "\\((\\d+)\\)";
            const std::string gate_index_pattern = "\\[(\\d+)\\]";

            // Extracts an index from a string by taking the last integer enclosed by parentheses
            std::optional<indexed_identifier> extract_index(const std::string& name, const std::string& index_pattern, const std::string& origin)
            {
                std::regex re(index_pattern);

                std::string input = name;
                std::optional<std::string> last_match;
                std::optional<u32> last_index;

                // Search for last match within string
                std::smatch match;
                while (std::regex_search(input, match, re))
                {
                    // Capture integer and update input string to search from after the match
                    last_index = std::stoi(match[1]);
                    last_match = match.str();
                    input      = match.suffix().str();
                }

                if (!last_index.has_value())
                {
                    return std::nullopt;
                }

                const auto found_match = last_match.value();
                auto identifier_name   = name;
                identifier_name        = identifier_name.replace(name.rfind(found_match), found_match.size(), "");

                return std::optional<indexed_identifier>{{identifier_name, last_index.value(), origin}};
            }

            // annotate all found identifiers to a gate
            bool annotate_indexed_identifiers(Gate* gate, const std::vector<indexed_identifier>& identifiers)
            {
                std::string json_identifier_str =
                    "[" + utils::join(", ", identifiers, [](const auto& i) { return std::string("[") + '"' + i.identifier + '"' + ", " + std::to_string(i.index) + ", " + '"' + i.origin + '"' + "]"; })
                    + "]";

                return gate->set_data("preprocessing_information", "multi_bit_indexed_identifiers", "string", json_identifier_str);
            }

            // search for a net that connects to the gate at a pin of a specific type and tries to reconstruct an indexed identifier from its name or form a name of its merged wires
            std::vector<indexed_identifier> check_net_at_pin(const PinType pin_type, Gate* gate)
            {
                const auto typed_pins = gate->get_type()->get_pins([pin_type](const auto p) { return p->get_type() == pin_type; });

                std::vector<indexed_identifier> found_identfiers;

                for (const auto& pin : typed_pins)
                {
                    const auto typed_net = (pin->get_direction() == PinDirection::output) ? gate->get_fan_out_net(pin) : gate->get_fan_in_net(pin);

                    // 1) search the net name itself
                    const auto net_name_index = extract_index(typed_net->get_name(), net_index_pattern, "net_name");
                    if (net_name_index.has_value())
                    {
                        found_identfiers.push_back(net_name_index.value());
                    }

                    // 2) search all the names of the wires that where merged into this net
                    if (!typed_net->has_data("parser_annotation", "merged_nets"))
                    {
                        continue;
                    }

                    const auto all_merged_nets_str = std::get<1>(typed_net->get_data("parser_annotation", "merged_nets"));

                    if (all_merged_nets_str.empty())
                    {
                        continue;
                    }

                    // parse json list of merged net names
                    rapidjson::Document doc;
                    doc.Parse(all_merged_nets_str.c_str());

                    for (u32 i = 0; i < doc.GetArray().Size(); i++)
                    {
                        const auto list = doc[i].GetArray();
                        for (u32 j = 0; j < list.Size(); j++)
                        {
                            const auto merged_wire_name = list[j].GetString();

                            const auto merged_wire_name_index = extract_index(merged_wire_name, net_index_pattern, "net_name");
                            if (merged_wire_name_index.has_value())
                            {
                                found_identfiers.push_back(merged_wire_name_index.value());
                            }
                        }
                    }
                }

                return found_identfiers;
            }
        }    // namespace

        Result<u32> reconstruct_indexed_ff_identifiers(Netlist* nl)
        {
            u32 counter = 0;
            for (auto& ff : nl->get_gates([](const auto g) { return g->get_type()->has_property(GateTypeProperty::ff); }))
            {
                std::vector<indexed_identifier> all_identifiers;

                // 1) Check whether the ff gate already has an index annotated in its gate name
                const auto cleaned_gate_name = replace_hal_instance_index(ff->get_name());
                const auto gate_name_index   = extract_index(cleaned_gate_name, gate_index_pattern, "gate_name");

                if (gate_name_index.has_value())
                {
                    auto found_identifier       = gate_name_index.value();
                    found_identifier.identifier = reconstruct_hal_instance_index(found_identifier.identifier);
                    all_identifiers.push_back(found_identifier);
                }

                static const std::vector<PinType> relevant_pin_types = {PinType::state, PinType::neg_state, PinType::data};

                // 2) Check all relevant pin_types
                for (const auto& pt : relevant_pin_types)
                {
                    const auto found_identifiers = check_net_at_pin(pt, ff);
                    all_identifiers.insert(all_identifiers.end(), found_identifiers.begin(), found_identifiers.end());
                }

                if (!all_identifiers.empty())
                {
                    counter++;
                }

                annotate_indexed_identifiers(ff, all_identifiers);
            }

            return OK(counter);
        }

        Result<u32> reconstruct_top_module_pin_groups(Netlist* nl)
        {
            std::map<std::string, std::map<u32, std::vector<ModulePin*>>> pg_name_to_indexed_pins;

            for (const auto& pin : nl->get_top_module()->get_pins())
            {
                auto reconstruct = extract_index(pin->get_name(), net_index_pattern, "");
                if (!reconstruct.has_value())
                {
                    continue;
                }

                auto [pg_name, index, _] = reconstruct.value();

                pg_name_to_indexed_pins[pg_name][index].push_back(pin);
            }

            u32 reconstructed_counter = 0;
            for (const auto& [pg_name, indexed_pins] : pg_name_to_indexed_pins)
            {
                std::vector<ModulePin*> ordered_pins;

                bool valid_indices = true;
                // NOTE: since the map already orders the indices from low to high, if we iterate over it we also get the pins in the right order
                for (const auto& [_index, pins] : indexed_pins)
                {
                    if (pins.size() > 1)
                    {
                        valid_indices = false;
                        break;
                    }

                    ordered_pins.push_back(pins.front());
                }

                if (!valid_indices)
                {
                    continue;
                }

                auto res = nl->get_top_module()->create_pin_group(pg_name, ordered_pins);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot reconstruct top module pin groups: failed to create pin group " + pg_name);
                }

                reconstructed_counter++;
            }

            return OK(reconstructed_counter);
        }

        namespace
        {
            struct ComponentData
            {
                std::string name;
                std::string type;
                u64 x;
                u64 y;
            };

            TokenStream<std::string> tokenize(std::stringstream& ss)
            {
                const std::string delimiters = " ;-";
                std::string current_token;
                u32 line_number = 0;

                std::string line;
                bool escaped = false;

                std::vector<Token<std::string>> parsed_tokens;
                while (std::getline(ss, line))
                {
                    line_number++;

                    for (char c : line)
                    {
                        // deal with escaping and strings
                        if (c == '\\')
                        {
                            escaped = true;
                            continue;
                        }
                        else if (escaped && std::isspace(c))
                        {
                            escaped = false;
                            continue;
                        }

                        if (((!std::isspace(c) && delimiters.find(c) == std::string::npos) || escaped))
                        {
                            current_token += c;
                        }
                        else
                        {
                            if (!current_token.empty())
                            {
                                parsed_tokens.emplace_back(line_number, current_token);
                                current_token.clear();
                            }

                            if (!std::isspace(c))
                            {
                                parsed_tokens.emplace_back(line_number, std::string(1, c));
                            }
                        }
                    }

                    if (!current_token.empty())
                    {
                        parsed_tokens.emplace_back(line_number, current_token);
                        current_token.clear();
                    }
                }

                return TokenStream(parsed_tokens, {}, {});
            }

            Result<std::unordered_map<std::string, ComponentData>> parse_tokens(TokenStream<std::string>& ts)
            {
                ts.consume_until("COMPONENTS");
                ts.consume("COMPONENTS");
                const auto component_count_str = ts.consume().string;
                ts.consume(";");

                u32 component_count;
                if (const auto res = utils::wrapped_stoul(component_count_str); res.is_ok())
                {
                    component_count = res.get();
                }
                else
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens: failed to read component count from token" + component_count_str);
                }

                std::cout << "Component count: " << component_count << std::endl;

                std::unordered_map<std::string, ComponentData> component_data;
                for (u32 c_idx = 0; c_idx < component_count; c_idx++)
                {
                    // parse a line
                    ComponentData new_data_entry;
                    ts.consume("-");
                    new_data_entry.name = ts.consume().string;
                    new_data_entry.type = ts.consume().string;
                    ts.consume("+");
                    ts.consume("SOURCE");
                    ts.consume("DIST");
                    ts.consume("TIMING");
                    ts.consume("+");
                    ts.consume("PLACED");
                    ts.consume("FIXED");
                    ts.consume("(");

                    if (const auto res = utils::wrapped_stoull(ts.consume().string); res.is_ok())
                    {
                        new_data_entry.x = res.get();
                    }
                    else
                    {
                        return ERR_APPEND(res.get_error(), "could not parse tokens: failed to read x coordinate from token");
                    }

                    if (const auto res = utils::wrapped_stoull(ts.consume().string); res.is_ok())
                    {
                        new_data_entry.y = res.get();
                    }
                    else
                    {
                        return ERR_APPEND(res.get_error(), "could not parse tokens: failed to read y coordinate from token");
                    }

                    ts.consume(")");

                    ts.consume_current_line();

                    component_data.insert({new_data_entry.name, new_data_entry});
                }

                return OK(component_data);
            }
        }    // namespace

        Result<std::monostate> parse_def_file(Netlist* nl, const std::filesystem::path& def_file)
        {
            std::stringstream ss;
            std::ifstream ifs;
            ifs.open(def_file.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                return ERR("could not parse DEF (Design Exchange Format) file '" + def_file.string() + "' : unable to open file");
            }
            ss << ifs.rdbuf();
            ifs.close();

            auto ts = tokenize(ss);

            std::unordered_map<std::string, ComponentData> component_data;
            // parse tokens
            try
            {
                if (auto res = parse_tokens(ts); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse Design Exchange Format file '" + def_file.string() + "': unable to parse tokens");
                }
                else
                {
                    component_data = res.get();
                }
            }
            catch (TokenStream<std::string>::TokenStreamException& e)
            {
                if (e.line_number != (u32)-1)
                {
                    return ERR("could not parse Design Exchange Format file '" + def_file.string() + "': " + e.message + " (line " + std::to_string(e.line_number) + ")");
                }
                else
                {
                    return ERR("could not parse Design Exchange Format file '" + def_file.string() + "': " + e.message);
                }
            }

            std::unordered_map<std::string, Gate*> name_to_gate;
            for (auto g : nl->get_gates())
            {
                name_to_gate.insert({g->get_name(), g});
            }

            u32 counter = 0;
            for (const auto& [gate_name, data] : component_data)
            {
                if (const auto& g_it = name_to_gate.find(gate_name); g_it != name_to_gate.end())
                {
                    // TODO figure out whereever we are saving coordinates now...
                    g_it->second->set_location_x(data.x);
                    g_it->second->set_location_y(data.y);

                    counter++;
                }
            }

            log_info("netlist_preprocessing", "reconstructed coordinates for {} / {} ({:.2}) gates", counter, nl->get_gates().size(), (double)counter / (double)nl->get_gates().size());

            return OK({});
        }

        Result<std::vector<Module*>> create_multi_bit_gate_modules(Netlist* nl, const std::map<std::string, std::map<std::string, std::vector<std::string>>>& concatenated_pin_groups)
        {
            std::vector<Module*> all_modules;
            for (const auto& [gt_name, pin_groups] : concatenated_pin_groups)
            {
                const auto& gt = nl->get_gate_library()->get_gate_type_by_name(gt_name);
                if (gt == nullptr)
                {
                    return ERR("unable to create multi bit gate module for gate type " + gt_name + ": failed to find gate type with that name in gate library " + nl->get_gate_library()->get_name());
                }

                for (const auto& g : nl->get_gates([&gt](const auto& g) { return g->get_type() == gt; }))
                {
                    auto m = nl->create_module("module_" + g->get_name(), g->get_module(), {g});

                    for (const auto& [module_pg_name, gate_pg_names] : pin_groups)
                    {
                        std::vector<ModulePin*> module_pins;
                        for (const auto& gate_pg_name : gate_pg_names)
                        {
                            const auto& gate_pg = g->get_type()->get_pin_group_by_name(gate_pg_name);

                            if (gate_pg == nullptr)
                            {
                                return ERR("unable to create multi-bit gate module for gate type " + gt_name + " and pin group " + gate_pg_name + ": failed to find pin group with that name");
                            }

                            std::vector<GatePin*> pin_list = gate_pg->get_pins();
                            if (!gate_pg->is_ascending())
                            {
                                std::reverse(pin_list.begin(), pin_list.end());
                            }

                            for (const auto& gate_pin : pin_list)
                            {
                                const auto net = (gate_pin->get_direction() == PinDirection::output) ? g->get_fan_out_net(gate_pin) : g->get_fan_in_net(gate_pin);
                                if (net == nullptr)
                                {
                                    continue;
                                }

                                if (net->is_gnd_net() || net->is_vcc_net())
                                {
                                    continue;
                                }

                                const auto module_pin = m->get_pin_by_net(net);

                                module_pins.push_back(module_pin);
                            }
                        }

                        m->create_pin_group(module_pg_name, module_pins);
                        u32 idx_counter = 0;
                        for (const auto& mp : module_pins)
                        {
                            m->set_pin_name(mp, module_pg_name + "_" + std::to_string(idx_counter));
                            idx_counter++;
                        }
                    }

                    all_modules.push_back(m);
                }
            }

            return OK(all_modules);
        }

        Result<std::vector<Net*>> create_nets_at_unconnected_pins(Netlist* nl)
        {
            std::vector<Net*> created_nets;

            for (const auto& g : nl->get_gates())
            {
                for (const auto& p : g->get_type()->get_output_pins())
                {
                    if (g->get_fan_out_net(p) == nullptr)
                    {
                        auto new_net = nl->create_net("TEMP");
                        new_net->set_name("HAL_UNCONNECTED_" + std::to_string(new_net->get_id()));
                        new_net->add_source(g, p);

                        created_nets.push_back(new_net);
                    }
                }
            }

            return OK(created_nets);
        }

        Result<u32> unify_ff_outputs(Netlist* nl, const std::vector<Gate*>& ffs, GateType* inverter_type)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            if (inverter_type == nullptr)
            {
                const auto* gl = nl->get_gate_library();
                const auto inv_types =
                    gl->get_gate_types([](const GateType* gt) { return gt->has_property(GateTypeProperty::c_inverter) && gt->get_input_pins().size() == 1 && gt->get_output_pins().size() == 1; });
                if (inv_types.empty())
                {
                    return ERR("gate library '" + gl->get_name() + "' of netlist does not contain an inverter gate");
                }
                inverter_type = inv_types.begin()->second;
            }
            else
            {
                if (inverter_type->get_gate_library() != nl->get_gate_library())
                {
                    return ERR("inverter gate type '" + inverter_type->get_name() + "' of gate library '" + inverter_type->get_gate_library()->get_name() + "' does not belong to gate library '"
                               + nl->get_gate_library()->get_name() + "' of provided netlist");
                }

                if (!inverter_type->has_property(GateTypeProperty::c_inverter))
                {
                    return ERR("gate type '" + inverter_type->get_name() + "' of gate library '" + inverter_type->get_gate_library()->get_name() + "' is not an inverter gate type");
                }

                if (inverter_type->get_input_pins().size() != 1 || inverter_type->get_output_pins().size() != 1)
                {
                    return ERR("inverter gate type '" + inverter_type->get_name() + "' of gate library '" + inverter_type->get_gate_library()->get_name()
                               + "' has an invalid number of input pins or output pins");
                }
            }

            auto inv_in_pin  = inverter_type->get_input_pins().front();
            auto inv_out_pin = inverter_type->get_output_pins().front();

            u32 ctr = 0;

            const std::vector<Gate*>& gates = ffs.empty() ? nl->get_gates() : ffs;

            for (auto* ff : gates)
            {
                auto* ff_type = ff->get_type();

                if (!ff_type->has_property(GateTypeProperty::ff))
                {
                    continue;
                }

                GatePin* state_pin     = nullptr;
                GatePin* neg_state_pin = nullptr;

                for (auto* o_pin : ff_type->get_output_pins())
                {
                    if (o_pin->get_type() == PinType::state)
                    {
                        state_pin = o_pin;
                    }
                    else if (o_pin->get_type() == PinType::neg_state)
                    {
                        neg_state_pin = o_pin;
                    }
                }

                if (neg_state_pin == nullptr)
                {
                    continue;
                }

                auto* neg_state_ep = ff->get_fan_out_endpoint(neg_state_pin);
                if (neg_state_ep == nullptr)
                {
                    continue;
                }
                auto* neg_state_net = neg_state_ep->get_net();

                auto state_net = ff->get_fan_out_net(state_pin);
                if (state_net == nullptr)
                {
                    state_net = nl->create_net(ff->get_name() + "__STATE_NET__");
                    state_net->add_source(ff, state_pin);
                }

                auto* inv = nl->create_gate(inverter_type, ff->get_name() + "__NEG_STATE_INVERT__");
                state_net->add_destination(inv, inv_in_pin);
                neg_state_net->remove_source(neg_state_ep);
                neg_state_net->add_source(inv, inv_out_pin);
                ctr++;
            }

            return OK(ctr);
        }
    }    // namespace netlist_preprocessing
}    // namespace hal