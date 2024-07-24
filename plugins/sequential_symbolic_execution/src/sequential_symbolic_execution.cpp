#pragma once

#include "hal_core/netlist/boolean_function/simplification.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/utils.h"
#include "z3_utils/simplification.h"
#include "z3_utils/z3_utils.h"

#include <sys/resource.h>

namespace hal
{
    namespace sse
    {
        namespace
        {
            inline BooleanFunction
                get_function_of_gate(const Gate* const gate, const std::string& output_pin, const u32 time_index, std::map<std::tuple<u32, std::string, u32>, BooleanFunction>& cache)
            {
                if (auto it = cache.find({gate->get_id(), output_pin, time_index}); it != cache.end())
                {
                    return it->second;
                }

                BooleanFunction bf = gate->get_boolean_function(output_pin);

                std::vector<std::string> input_vars = utils::to_vector(bf.get_variable_names());
                while (!input_vars.empty())
                {
                    const std::string var = input_vars.back();
                    input_vars.pop_back();

                    const PinDirection pin_dir = gate->get_type()->get_pins([var](GatePin* p) { return p->get_name() == var; }).front()->get_direction();

                    if (pin_dir == PinDirection::input)
                    {
                        const Net* const input_net = gate->get_fan_in_net(var);
                        if (input_net == nullptr)
                        {
                            // if no net is connected, the input pin name cannot be replaced
                            log_warning("sequential_symbolic_execution", "no net is connected to input pin '{}' of gate with ID {}, cannot replace pin name with net ID.", var, gate->get_id());
                            return bf;
                        }

                        bf = bf.substitute(var, "net_" + std::to_string(input_net->get_id()) + "_" + std::to_string(time_index));
                    }
                    else if ((pin_dir == PinDirection::internal) || (pin_dir == PinDirection::output))
                    {
                        BooleanFunction bf_interal = gate->get_boolean_function(var);
                        if (bf_interal.is_empty())
                        {
                            log_warning("sequential_symbolic_execution",
                                        "trying to replace {} in function {} for gate {} and pin {} but cannot find boolean fucntion.",
                                        var,
                                        bf.to_string(),
                                        gate->get_id(),
                                        output_pin);
                            return bf;
                        }

                        const std::vector<std::string> internal_input_vars = utils::to_vector(bf_interal.get_variable_names());
                        input_vars.insert(input_vars.end(), internal_input_vars.begin(), internal_input_vars.end());

                        auto substituted = bf.substitute(var, bf_interal);
                        if (substituted.is_error())
                        {
                            log_error("sequential_symbolic_execution", "{}", substituted.get_error().get());
                            return BooleanFunction();
                        }
                        bf = substituted.get();
                    }
                }

                cache.insert({{gate->get_id(), output_pin, time_index}, bf});
                return bf;
            }

            inline BooleanFunction
                get_function_of_seq_gate(const Gate* const gate, const std::string& output_pin, const u32 time_index, std::map<std::tuple<u32, std::string, u32>, BooleanFunction>& cache)
            {
                if (auto it = cache.find({gate->get_id(), output_pin, time_index}); it != cache.end())
                {
                    return it->second;
                }

                auto state_pins = gate->get_type()->get_pins([](GatePin* p) { return p->get_type() == PinType::state; });

                if (state_pins.size() != 1)
                {
                    log_error("iphone_tools", "Found {} state pisn for gate {} of type {}. Can only handle one!", state_pins.size(), gate->get_id(), gate->get_type()->get_name());
                }

                auto state_pin = *(state_pins.begin());

                auto internal_state_bf = gate->get_type()->get_boolean_function(state_pin);
                if (!internal_state_bf.is_variable())
                {
                    log_error("sequential_symbolic_execution", "Internal state of ff type {} is not a single variable. Cannot handle!", gate->get_type()->get_name());
                }
                auto internal_state_name = *(internal_state_bf.get_variable_names().begin());

                if (internal_state_bf.is_empty())
                {
                    log_error("sequential_symbolic_execution", "No booleanfunction for pin {} at gatetype {}!", output_pin, gate->get_type()->get_name());
                }

                auto ff_cfg = gate->get_type()->get_component_as<FFComponent>([](const GateTypeComponent* gtc) { return gtc->get_type() == GateTypeComponent::ComponentType::ff; });

                auto bf = (ff_cfg->get_clock_function() & ff_cfg->get_next_state_function()) | (~ff_cfg->get_clock_function() & internal_state_bf);

                if (auto async_reset = ff_cfg->get_async_reset_function(); !async_reset.is_empty())
                {
                    bf = bf & (~async_reset);
                }
                if (auto async_set = ff_cfg->get_async_set_function(); !async_set.is_empty())
                {
                    bf = bf | async_set;
                }

                bf = bf.substitute(internal_state_name, state_pin->get_name());

                // const std::map<std::string, BooleanFunction> gate_type_to_boolean_function = {
                //     {"SB_DFFER", BooleanFunction::from_string("(((C & E) & !R) & D) | (((!C | !E) & !R) & Q)").get()},
                //     {"SB_DFFS",  BooleanFunction::from_string("(C & (D | S)) | (!C & Q)").get()},
                //     {"SB_DFFES", BooleanFunction::from_string("(C & ((E & D) | S | (!E & Q))) | (!C & Q)").get()},
                // };

                if (gate->get_type()->get_pins([output_pin](GatePin* p) { return p->get_name() == output_pin; }).front()->get_type() == PinType::neg_state)
                {
                    bf = ~bf;
                }

                for (const auto& pin_name : bf.get_variable_names())
                {
                    const auto pin = gate->get_type()->get_pins([pin_name](GatePin* p) { return p->get_name() == pin_name; }).front();
                    const auto net = (pin->get_direction() == PinDirection::input) ? gate->get_fan_in_net(pin) : gate->get_fan_out_net(pin);
                    // const u32 new_time_index = (pin->get_type() == PinType::neg_state || pin->get_type() == PinType::state) ? (time_index - 1) : time_index;
                    const u32 new_time_index = time_index - 1;

                    if (net != nullptr)
                    {
                        const std::string net_name = "net_" + std::to_string(net->get_id()) + "_" + std::to_string(new_time_index);
                        bf                         = bf.substitute(pin_name, net_name);
                    }
                }

                cache.insert({{gate->get_id(), output_pin, time_index}, bf});
                return bf;
            }

            Result<z3::expr> get_function_of_net_z3_word_level_internal(Net* net,
                                                                        const u32 time_index,
                                                                        const std::vector<bool>& subgraph_gates_byte_map,
                                                                        const std::vector<std::map<u32, bool>>& known_inputs,
                                                                        const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                                                        std::map<std::tuple<u32, std::string, u32>, BooleanFunction>& gate_cache,
                                                                        std::map<std::pair<u32, u32>, z3::expr>& net_cache,
                                                                        const bool substitute_endpoints,
                                                                        z3::context& ctx)
            {
                // std::cout << "Getting function at time step " << time_index << " for net " << net->get_id() << " - " << net->get_name() << std::endl;

                if (net == nullptr)
                {
                    return ERR("nullptr given for target net");
                }
                else if (net->get_num_of_sources() > 1)
                {
                    return ERR("target net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + "has more than one source");
                }

                if (known_inputs.at(time_index).find(net->get_id()) != known_inputs.at(time_index).end())
                {
                    const auto known_value = known_inputs.at(time_index).at(net->get_id()) ? 1 : 0;
                    const auto bf          = ctx.bv_val(known_value, 1);
                    net_cache.insert({{net->get_id(), time_index}, bf});
                    return OK(bf);
                }

                if (const auto it = net_cache.find({net->get_id(), time_index}); it != net_cache.end())
                {
                    return OK(it->second);
                }

                if (net->is_global_input_net())
                {
                    if (substitute_endpoints)
                    {
                        const auto pin                = net->get_netlist()->get_top_module()->get_pin_by_net(net);
                        const auto [pin_group, index] = pin->get_group();
                        const std::string name        = pin_group->get_name() + "_" + std::to_string(time_index);

                        const auto bf = ctx.bv_const(name.c_str(), pin_group->size()).extract(index, index);
                        net_cache.insert({{net->get_id(), time_index}, bf});
                        return OK(bf);
                    }

                    const auto bf = ctx.bv_const(("net_" + std::to_string(net->get_id()) + "_" + std::to_string(time_index)).c_str(), 1);
                    net_cache.insert({{net->get_id(), time_index}, bf});
                    return OK(bf);
                }

                if (net->get_num_of_sources() == 0)
                {
                    // log_debug("sequential_symbolic_execution", "target net with ID {} has no sources.", net->get_id());

                    const auto bf = ctx.bv_const(("net_" + std::to_string(net->get_id()) + "_" + std::to_string(time_index)).c_str(), 1);
                    net_cache.insert({{net->get_id(), time_index}, bf});
                    return OK(bf);
                }

                const Gate* src_gate          = net->get_sources()[0]->get_gate();
                const std::string src_pin     = net->get_sources()[0]->get_pin()->get_name();
                const bool has_sequential_src = src_gate->get_type()->has_property(GateTypeProperty::sequential);

                const auto all_src_modules = src_gate->get_modules(nullptr, true);

                // reached end of subgraph or time limit
                if ((!subgraph_gates_byte_map.at(src_gate->get_id())) || (has_sequential_src && (time_index == 0)))
                {
                    if (substitute_endpoints)
                    {
                        // Find highest order module with an intact bitorder to substitute net with sliced bit field from module
                        for (auto it = all_src_modules.rbegin(); it != all_src_modules.rend(); it++)
                        {
                            const auto src_mod = *it;
                            if (src_mod->is_output_net(net))
                            {
                                const auto pin                = src_mod->get_pin_by_net(net);
                                const auto [pin_group, index] = pin->get_group();

                                if (pin_group->size() > 1)
                                {
                                    const std::string name = src_mod->get_name() + "_" + pin_group->get_name() + "_" + std::to_string(time_index);
                                    const auto bf          = ctx.bv_const(name.c_str(), pin_group->size()).extract(index, index);
                                    net_cache.insert({{net->get_id(), time_index}, bf});
                                    return OK(bf);
                                }
                            }
                        }
                    }

                    const auto bf = ctx.bv_const(("net_" + std::to_string(net->get_id()) + "_" + std::to_string(time_index)).c_str(), 1);
                    net_cache.insert({{net->get_id(), time_index}, bf});
                    return OK(bf);
                }

                // check whether we reached a net that is part of a pin group with a multi bit calculation
                for (auto it = all_src_modules.rbegin(); it != all_src_modules.rend(); it++)
                {
                    const auto src_mod = *it;
                    if (src_mod->is_output_net(net))
                    {
                        const auto pin                = src_mod->get_pin_by_net(net);
                        const auto [pin_group, index] = pin->get_group();

                        if (const auto it = word_level_calculations.find(pin_group); it != word_level_calculations.end())
                        {
                            const auto& wlc_operation = it->second;

                            // check whether we have known inputs
                            std::map<std::string, BooleanFunction> known_signals;
                            for (const auto& var_name : wlc_operation.get_variable_names())
                            {
                                if (var_name == "UNKNOWN_OPERATION")
                                {
                                    continue;
                                }

                                const auto var_net_res = BooleanFunctionNetDecorator::get_net_from(net->get_netlist(), var_name);
                                if (var_net_res.is_error())
                                {
                                    return ERR_APPEND(var_net_res.get_error(),
                                                      "cannot get function of net " + net->get_name() + " with ID " + std::to_string(net->get_id())
                                                          + ": failed to extract index from word level computation variable");
                                }
                                const auto var_net = var_net_res.get();

                                if (const auto it = known_inputs[time_index].find(var_net->get_id()); it != known_inputs[time_index].end())
                                {
                                    known_signals.insert({var_name, BooleanFunction::Const(it->second ? 1 : 0, 1)});
                                }
                            }

                            const auto wlc_simplified = wlc_operation.substitute(known_signals).get().simplify_local();

                            std::map<std::string, z3::expr> operand_signals;
                            for (const auto& var_name : wlc_simplified.get_variable_names())
                            {
                                if (var_name == "UNKNOWN_OPERATION")
                                {
                                    continue;
                                }

                                const auto var_net = BooleanFunctionNetDecorator::get_net_from(net->get_netlist(), var_name).get();

                                if (const auto it = net_cache.find({var_net->get_id(), time_index}); it != net_cache.end())
                                {
                                    operand_signals.emplace(var_name, it->second);
                                }
                                else
                                {
                                    return ERR("cannot get function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to find net id "
                                               + std::to_string(var_net->get_id()) + " for time index " + std::to_string(time_index) + " in net cache");
                                }
                            }

                            auto bf_word_level_z3 = z3_utils::from_bf(wlc_simplified, ctx, operand_signals);
                            // bf_word_level_z3      = bf_word_level_z3.simplify();

                            // insert result into cache for all nets of the pin group
                            for (u32 pin_idx = 0; pin_idx < pin_group->size(); pin_idx++)
                            {
                                const auto pin_net = pin_group->get_pins().at(pin_idx)->get_net();

                                // TODO remove
                                // std::cout << "Pin net: " << pin_net->get_id() << " - " << pin_net->get_name() << " at index " << pin_idx << std::endl;

                                z3::expr pin_e = bf_word_level_z3.extract(pin_idx, pin_idx);
                                pin_e          = z3_utils::simplify_local(pin_e).get();

                                net_cache.insert({{pin_net->get_id(), time_index}, pin_e});
                            }

                            return OK(bf_word_level_z3.extract(index, index));
                        }
                    }
                }

                BooleanFunction bf;
                if (has_sequential_src)
                {
                    bf = get_function_of_seq_gate(src_gate, src_pin, time_index, gate_cache);
                }
                else
                {
                    bf = get_function_of_gate(src_gate, src_pin, time_index, gate_cache);
                }

                // substitute known values in gate function to prevent following irrelevant paths
                for (const auto& var : bf.get_variable_names())
                {
                    const std::vector<std::string> split = utils::split(var, '_');
                    const u32 var_net_id                 = std::stoi(split.at(1));
                    const u32 var_time_index             = std::stoi(split.at(2));

                    if (known_inputs[var_time_index].find(var_net_id) != known_inputs[var_time_index].end())
                    {
                        BooleanFunction::Value known_value = known_inputs[var_time_index].at(var_net_id) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO;
                        if (auto substitution = bf.substitute(var, BooleanFunction::Const(known_value)); substitution.is_ok())
                        {
                            bf = substitution.get();
                        }
                        else
                        {
                            log_error("sequential_symbolic_execution", "{}", substitution.get_error().get());
                        }
                    }
                }

                // NOTE regular simplify seems to have a memory leak (maybe also some saved data in abc)
                if (auto simplification = Simplification::local_simplification(bf); simplification.is_ok())
                {
                    bf = simplification.get();
                }
                else
                {
                    log_error("sequential_symbolic_execution", "{}", simplification.get_error().get());
                }

                std::map<std::string, z3::expr> var_name_to_function;

                for (const auto& var : bf.get_variable_names())
                {
                    const std::vector<std::string> split = utils::split(var, '_');
                    const u32 var_net_id                 = std::stoi(split.at(1));
                    const u32 var_time_index             = std::stoi(split.at(2));

                    // Net* var_net = net->get_netlist()->get_net_by_id(var_net_id);
                    // auto var_bf_res =
                    //     get_function_of_net_z3_word_level(var_net, time_index, subgraph_gates_byte_map, known_inputs, word_level_calculations, gate_cache, net_cache, substitute_endpoints, ctx);
                    // if (var_bf_res.is_error())
                    // {
                    //     return var_bf_res;
                    // }

                    if (const auto it = net_cache.find({var_net_id, var_time_index}); it != net_cache.end())
                    {
                        var_name_to_function.emplace(var, it->second);
                    }
                    else
                    {
                        return ERR("cannot get function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to find net id " + std::to_string(var_net_id)
                                   + " for time index " + std::to_string(var_time_index) + " in net cache");
                    }
                }

                auto bf_z3 = z3_utils::from_bf(bf, ctx, var_name_to_function);

                // bf_z3                   = bf_z3.simplify();
                const auto simplify_res = z3_utils::simplify_local(bf_z3);
                if (simplify_res.is_error())
                {
                    return ERR_APPEND(simplify_res.get_error(), "cannot get function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to simplify expression");
                }
                bf_z3 = simplify_res.get();

                net_cache.insert({{net->get_id(), time_index}, bf_z3});
                return OK(bf_z3);
            }

            Result<std::vector<std::pair<Net*, u32>>> get_net_inputs(Net* net,
                                                                     const u32 time_index,
                                                                     const std::vector<bool>& subgraph_gates_byte_map,
                                                                     const std::vector<std::map<u32, bool>>& known_inputs,
                                                                     const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                                                     std::map<std::tuple<u32, std::string, u32>, BooleanFunction>& gate_cache,
                                                                     std::map<std::pair<u32, u32>, z3::expr>& net_cache)
            {
                // std::cout << "Getting missing inputs at time step " << time_index << " for net " << net->get_id() << " - " << net->get_name() << std::endl;

                std::vector<std::pair<Net*, u32>> indexed_net_inputs;

                if (net == nullptr)
                {
                    return ERR("nullptr given for target net");
                }
                else if (net->get_num_of_sources() > 1)
                {
                    return ERR("target net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + "has more than one source");
                }

                if (known_inputs.at(time_index).find(net->get_id()) != known_inputs.at(time_index).end())
                {
                    return OK({});
                }

                if (const auto it = net_cache.find({net->get_id(), time_index}); it != net_cache.end())
                {
                    return OK({});
                }

                if (net->is_global_input_net())
                {
                    return OK({});
                }

                if (net->get_num_of_sources() == 0)
                {
                    // log_debug("sequential_symbolic_execution", "target net with ID {} has no sources.", net->get_id());

                    return OK({});
                }

                const Gate* src_gate          = net->get_sources()[0]->get_gate();
                const std::string src_pin     = net->get_sources()[0]->get_pin()->get_name();
                const bool has_sequential_src = src_gate->get_type()->has_property(GateTypeProperty::sequential);

                const auto all_src_modules = src_gate->get_modules(nullptr, true);

                // reached end of subgraph or time limit
                if ((!subgraph_gates_byte_map.at(src_gate->get_id())) || (has_sequential_src && (time_index == 0)))
                {
                    return OK({});
                }

                // check whether we reached a net that is part of a pin group with a multi bit calculation
                for (auto it = all_src_modules.rbegin(); it != all_src_modules.rend(); it++)
                {
                    const auto src_mod = *it;
                    if (src_mod->is_output_net(net))
                    {
                        const auto pin                = src_mod->get_pin_by_net(net);
                        const auto [pin_group, index] = pin->get_group();

                        if (const auto it = word_level_calculations.find(pin_group); it != word_level_calculations.end())
                        {
                            const auto& wlc_operation = it->second;

                            // check whether we have known inputs
                            std::map<std::string, BooleanFunction> known_signals;
                            for (const auto& var_name : wlc_operation.get_variable_names())
                            {
                                if (var_name == "UNKNOWN_OPERATION")
                                {
                                    continue;
                                }

                                const auto var_net_res = BooleanFunctionNetDecorator::get_net_from(net->get_netlist(), var_name);
                                if (var_net_res.is_error())
                                {
                                    return ERR_APPEND(var_net_res.get_error(),
                                                      "cannot get missing net inputs of net " + net->get_name() + " with ID " + std::to_string(net->get_id())
                                                          + ": failed to extract index from word level computation variable");
                                }
                                const auto var_net = var_net_res.get();

                                if (const auto it = known_inputs[time_index].find(var_net->get_id()); it != known_inputs[time_index].end())
                                {
                                    known_signals.insert({var_name, BooleanFunction::Const(it->second ? 1 : 0, 1)});
                                }
                            }

                            const auto wlc_simplified = wlc_operation.substitute(known_signals).get().simplify_local();

                            for (const auto& var_name : wlc_simplified.get_variable_names())
                            {
                                if (var_name == "UNKNOWN_OPERATION")
                                {
                                    continue;
                                }

                                auto var_net = BooleanFunctionNetDecorator::get_net_from(net->get_netlist(), var_name).get();

                                if (const auto it = net_cache.find({var_net->get_id(), time_index}); it == net_cache.end())
                                {
                                    indexed_net_inputs.push_back(std::pair<Net*, u32>{var_net, time_index});
                                }
                            }

                            return OK(indexed_net_inputs);
                        }
                    }
                }

                BooleanFunction bf;
                if (has_sequential_src)
                {
                    bf = get_function_of_seq_gate(src_gate, src_pin, time_index, gate_cache);
                }
                else
                {
                    bf = get_function_of_gate(src_gate, src_pin, time_index, gate_cache);
                }

                // substitute known values in gate function to prevent following irrelevant paths
                for (const auto& var : bf.get_variable_names())
                {
                    const std::vector<std::string> split = utils::split(var, '_');
                    const u32 var_net_id                 = std::stoi(split.at(1));
                    const u32 var_time_index             = std::stoi(split.at(2));

                    if (known_inputs[var_time_index].find(var_net_id) != known_inputs[var_time_index].end())
                    {
                        BooleanFunction::Value known_value = known_inputs[var_time_index].at(var_net_id) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO;
                        if (auto substitution = bf.substitute(var, BooleanFunction::Const(known_value)); substitution.is_ok())
                        {
                            bf = substitution.get();
                        }
                        else
                        {
                            log_error("sequential_symbolic_execution", "{}", substitution.get_error().get());
                        }
                    }
                }

                // NOTE regular simplify seems to have a memory leak (maybe also some saved data in abc)
                if (auto simplification = Simplification::local_simplification(bf); simplification.is_ok())
                {
                    bf = simplification.get();
                }
                else
                {
                    log_error("sequential_symbolic_execution", "{}", simplification.get_error().get());
                }

                for (const auto& var : bf.get_variable_names())
                {
                    const std::vector<std::string> split = utils::split(var, '_');
                    const u32 var_net_id                 = std::stoi(split.at(1));
                    const u32 var_time_index             = std::stoi(split.at(2));

                    Net* var_net = net->get_netlist()->get_net_by_id(var_net_id);

                    if (const auto it = net_cache.find({var_net_id, var_time_index}); it == net_cache.end())
                    {
                        indexed_net_inputs.push_back({var_net, var_time_index});
                    }
                }

                return OK(indexed_net_inputs);
            }

            Result<z3::expr> get_function_of_net_z3_word_level(Net* net,
                                                               const u32 time_index,
                                                               const std::vector<bool>& subgraph_gates_byte_map,
                                                               const std::vector<std::map<u32, bool>>& known_inputs,
                                                               const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                                               std::map<std::tuple<u32, std::string, u32>, BooleanFunction>& gate_cache,
                                                               std::map<std::pair<u32, u32>, z3::expr>& net_cache,
                                                               const bool substitute_endpoints,
                                                               z3::context& ctx)
            {
                std::vector<std::pair<Net*, u32>> stack = {std::pair<Net*, u32>{net, time_index}};
                std::set<std::pair<Net*, u32>> visited  = {std::pair<Net*, u32>{net, time_index}};

                while (!stack.empty())
                {
                    const auto [n, t]             = stack.back();
                    const auto missing_inputs_res = get_net_inputs(n, t, subgraph_gates_byte_map, known_inputs, word_level_calculations, gate_cache, net_cache);
                    if (missing_inputs_res.is_error())
                    {
                        return ERR_APPEND(missing_inputs_res.get_error(),
                                          "cannot get function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to get missing input nets for net " + n->get_name()
                                              + " with ID " + std::to_string(n->get_id()) + " at time index " + std::to_string(t));
                    }

                    const auto missing_inputs = missing_inputs_res.get();

                    if (!missing_inputs.empty())
                    {
                        const u32 before = stack.size();
                        for (const auto& nt : missing_inputs)
                        {
                            if (visited.find(nt) == visited.end())
                            {
                                stack.push_back(nt);
                                visited.insert(nt);
                            }
                        }

                        // If the stack size did not change, just add the missing inputs no matter what to the back of the stack
                        // Otherwise the top element will keep "waiting" until its inputs are resolved, however they lay further down in the stack
                        if (stack.size() == before)
                        {
                            for (const auto& nt : missing_inputs)
                            {
                                stack.push_back(nt);
                                visited.insert(nt);
                            }
                        }

                        continue;
                    }

                    const auto bf_res =
                        get_function_of_net_z3_word_level_internal(n, t, subgraph_gates_byte_map, known_inputs, word_level_calculations, gate_cache, net_cache, substitute_endpoints, ctx);
                    if (bf_res.is_error())
                    {
                        return bf_res;
                    }

                    if (stack.size() == 1)
                    {
                        return bf_res;
                    }

                    stack.pop_back();
                }

                return ERR("unreachable reached");
            }

            Result<z3::expr> subsitute_nets_with_pins(const z3::expr& e, const Netlist* nl, const bool simplify)
            {
                const auto var_names = z3_utils::get_variable_names(e);

                z3::expr_vector from(e.ctx());
                z3::expr_vector to(e.ctx());

                for (const auto& vn : var_names)
                {
                    // check whether the variable is a net variable
                    if (vn.find("net_") != 0)
                    {
                        continue;
                    }

                    const std::vector<std::string> split = utils::split(vn, '_');
                    const u32 var_net_id                 = std::stoi(split.at(1));
                    const u32 var_time_index             = std::stoi(split.at(2));

                    Net* net = nl->get_net_by_id(var_net_id);

                    if (net->is_global_input_net())
                    {
                        const auto pin                = nl->get_top_module()->get_pin_by_net(net);
                        const auto [pin_group, index] = pin->get_group();

                        const std::string name = pin_group->get_name() + "_" + std::to_string(var_time_index);
                        const auto to_bf       = e.ctx().bv_const(name.c_str(), pin_group->size()).extract(index, index);
                        const auto from_bf     = e.ctx().bv_const(vn.c_str(), 1);

                        // TODO remove
                        // std::cout << "FROM: " << from_bf << std::endl;
                        // std::cout << "TO:   " << to_bf << std::endl;

                        from.push_back(from_bf);
                        to.push_back(to_bf);

                        continue;
                    }

                    if (net->get_num_of_sources() != 1)
                    {
                        continue;
                    }

                    const Gate* src_gate      = net->get_sources()[0]->get_gate();
                    const std::string src_pin = net->get_sources()[0]->get_pin()->get_name();

                    const auto all_src_modules = src_gate->get_modules(nullptr, true);

                    // Find highest order module with an intact bitorder to substitute net with sliced bit field from module
                    for (auto it = all_src_modules.rbegin(); it != all_src_modules.rend(); it++)
                    {
                        const auto src_mod = *it;
                        if (src_mod->is_output_net(net))
                        {
                            const auto pin                = src_mod->get_pin_by_net(net);
                            const auto [pin_group, index] = pin->get_group();

                            if (pin_group->size() > 1)
                            {
                                const std::string name = src_mod->get_name() + "_" + pin_group->get_name() + "_" + std::to_string(var_time_index);
                                const auto to_bf       = e.ctx().bv_const(name.c_str(), pin_group->size()).extract(index, index);
                                const auto from_bf     = e.ctx().bv_const(vn.c_str(), 1);

                                from.push_back(from_bf);
                                to.push_back(to_bf);

                                break;
                            }
                        }
                    }
                }

                auto e_cpy = e;
                auto res   = e_cpy.substitute(from, to);
                if (simplify)
                {
                    // res                     = res.simplify();
                    const auto simplify_res = z3_utils::simplify_local(res);
                    if (simplify_res.is_error())
                    {
                        return ERR_APPEND(simplify_res.get_error(), "cannot substitute endpoints for z3::expr: failed to simplify expression");
                    }
                    res = simplify_res.get();
                }

                return OK(res);
            }

        }    // namespace

        Result<z3::expr> get_value_at_z3(Net* net,
                                         const u32 time_index,
                                         const std::vector<bool>& subgraph_gates_byte_map,
                                         const std::vector<std::map<u32, bool>>& known_inputs,
                                         const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                         const bool substitute_endpoints,
                                         z3::context& ctx)
        {
            std::map<std::tuple<u32, std::string, u32>, BooleanFunction> gate_cache;
            std::map<std::pair<u32, u32>, z3::expr> net_cache;

            auto bf_res = sse::get_function_of_net_z3_word_level(net, time_index, subgraph_gates_byte_map, known_inputs, word_level_calculations, gate_cache, net_cache, substitute_endpoints, ctx);

            return bf_res;
        }

        Result<std::vector<z3::expr>> get_word_values_at_z3(const std::vector<std::vector<Net*>>& words,
                                                            const std::vector<u32>& time_indices,
                                                            const std::vector<bool>& subgraph_gates_byte_map,
                                                            const std::vector<std::map<u32, bool>>& known_inputs,
                                                            const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                                            const bool substitute_endpoints,
                                                            z3::context& ctx)
        {
            std::map<std::tuple<u32, std::string, u32>, BooleanFunction> gate_cache;
            std::map<std::pair<u32, u32>, z3::expr> net_cache;

            std::vector<z3::expr> result;

            for (u32 word_idx = 0; word_idx < words.size(); word_idx++)
            {
                const auto& nets = words.at(word_idx);
                const auto bf_res =
                    sse::get_function_of_net_z3_word_level(nets.front(), time_indices.at(word_idx), subgraph_gates_byte_map, known_inputs, word_level_calculations, gate_cache, net_cache, false, ctx);

                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(), "cannot get word value for word at index " + std::to_string(word_idx) + ": failed to build single net function");
                }

                z3::expr bf_word = bf_res.get();
                for (u32 bit_idx = 1; bit_idx < nets.size(); bit_idx++)
                {
                    const auto bf_bit_res = sse::get_function_of_net_z3_word_level(
                        nets.at(bit_idx), time_indices.at(word_idx), subgraph_gates_byte_map, known_inputs, word_level_calculations, gate_cache, net_cache, false, ctx);
                    if (bf_bit_res.is_error())
                    {
                        return ERR_APPEND(bf_res.get_error(), "cannot get word value for word at index " + std::to_string(word_idx) + ": failed to build single net function");
                    }

                    bf_word = z3::concat(bf_bit_res.get(), bf_word);
                }

                if (substitute_endpoints)
                {
                    const Netlist* nl         = words.front().front()->get_netlist();
                    const auto substitute_res = subsitute_nets_with_pins(bf_word, nl, true);
                    if (substitute_res.is_error())
                    {
                        return ERR_APPEND(substitute_res.get_error(), "cannot get word value for word at index " + std::to_string(word_idx) + ": failed to substitute nets with pins");
                    }
                    bf_word = substitute_res.get();
                }

                // bf_word                 = bf_word.simplify();
                const auto simplify_res = z3_utils::simplify_local(bf_word);
                if (simplify_res.is_error())
                {
                    return ERR_APPEND(simplify_res.get_error(), "cannot get word value for word at index " + std::to_string(word_idx) + ": failed to simplify expression");
                }
                bf_word = simplify_res.get();

                result.push_back(bf_word);
            }

            return OK(result);
        }

        Result<std::vector<z3::expr>> get_word_values_at_z3(const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& words,
                                                            const std::vector<u32>& time_indices,
                                                            const std::vector<bool>& subgraph_gates_byte_map,
                                                            const std::vector<std::map<u32, bool>>& known_inputs,
                                                            const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                                            const bool substitute_endpoints,
                                                            z3::context& ctx)
        {
            std::map<std::tuple<u32, std::string, u32>, BooleanFunction> gate_cache;
            std::map<std::pair<u32, u32>, z3::expr> net_cache;

            std::vector<std::vector<Net*>> word_nets;

            for (const auto& [_m, pg] : words)
            {
                std::vector<Net*> nets;
                for (const auto& pin : pg->get_pins())
                {
                    nets.push_back(pin->get_net());
                }
                word_nets.push_back(nets);
            }

            return get_word_values_at_z3(word_nets, time_indices, subgraph_gates_byte_map, known_inputs, word_level_calculations, substitute_endpoints, ctx);
        }

    }    // namespace sse
}    // namespace hal
