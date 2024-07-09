#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "z3_utils/include/utils/json.hpp"
#include "z3_utils/include/netlist_comparison.h"
#include "z3_utils/include/z3_utils.h"


namespace hal
{
    namespace z3_utils
    {
        // NOTE this is almost an exact copy of the subgraph function generator with the small addition to pass a variable name prefix.
        //      This allows us to generate distinct net variables for two different netlists by passing differend prefixes to the function (netlist_a_net_1 vs. netlist_b_net_1)
        namespace
        {
            Result<z3::expr> get_prefixed_function_of_net(const std::vector<Gate*>& subgraph_gates,
                                                          const Net* net,
                                                          const std::string& variable_prefix,
                                                          z3::context& ctx,
                                                          std::map<u32, z3::expr>& net_cache,
                                                          std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache)
            {
                if (const auto it = net_cache.find(net->get_id()); it != net_cache.end())
                {
                    return OK(it->second);
                }

                const std::vector<Endpoint*> sources = net->get_sources();

                // net is multi driven
                if (sources.size() > 1)
                {
                    return ERR("cannot get Boolean z3 function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": net is multi driven.");
                }

                // net has no source
                if (sources.empty())
                {
                    const std::string var_name = variable_prefix + BooleanFunctionNetDecorator(*net).get_boolean_variable_name();
                    z3::expr ret               = ctx.bv_const(var_name.c_str(), 1);
                    net_cache.insert({net->get_id(), ret});
                    return OK(ret);
                }

                const Endpoint* src_ep = sources.front();

                if (src_ep->get_gate() == nullptr)
                {
                    return ERR("cannot get Boolean z3 function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": net source is null.");
                }

                const Gate* src = src_ep->get_gate();

                // source is not in subgraph gates
                if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src) == subgraph_gates.end())
                {
                    const std::string var_name = variable_prefix + BooleanFunctionNetDecorator(*net).get_boolean_variable_name();
                    z3::expr ret               = ctx.bv_const(var_name.c_str(), 1);
                    net_cache.insert({net->get_id(), ret});
                    return OK(ret);
                }

                BooleanFunction bf;
                if (const auto it = gate_cache.find({src->get_id(), src_ep->get_pin()}); it == gate_cache.end())
                {
                    const auto bf_res = src->get_resolved_boolean_function(src_ep->get_pin());
                    if (bf_res.is_error())
                    {
                        return ERR_APPEND(bf_res.get_error(),
                                          "cannot get Boolean z3 function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to get function of gate.");
                    }
                    bf = bf_res.get();

                    gate_cache.insert({{src->get_id(), src_ep->get_pin()}, bf});
                }
                else
                {
                    bf = it->second;
                }

                std::map<std::string, z3::expr> input_to_expr;

                for (const std::string& in_net_str : bf.get_variable_names())
                {
                    const auto in_net_res = BooleanFunctionNetDecorator::get_net_from(src->get_netlist(), in_net_str);
                    if (in_net_res.is_error())
                    {
                        return ERR_APPEND(in_net_res.get_error(),
                                          "cannot get Boolean z3 function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to reconstruct input net from variable "
                                              + in_net_str + ".");
                    }
                    const auto in_net = in_net_res.get();

                    const auto in_bf_res = get_prefixed_function_of_net(subgraph_gates, in_net, variable_prefix, ctx, net_cache, gate_cache);
                    if (in_bf_res.is_error())
                    {
                        // NOTE since this can lead to a deep recursion we do not append the error and instead only propagate it.
                        return in_bf_res;
                    }
                    const auto in_bf = in_bf_res.get();

                    input_to_expr.insert({in_net_str, in_bf});
                }

                z3::expr ret = z3_utils::from_bf(bf, ctx, input_to_expr).simplify();
                net_cache.insert({net->get_id(), ret});

                return OK(ret);
            }

            Result<z3::expr> get_prefixed_subgraph_z3_function_internal(const std::vector<Gate*>& subgraph_gates,
                                                                        const Net* net,
                                                                        const std::string& variable_prefix,
                                                                        z3::context& ctx,
                                                                        std::map<u32, z3::expr>& net_cache,
                                                                        std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache)
            {
                // check validity of subgraph_gates
                if (subgraph_gates.empty())
                {
                    return ERR("could not get subgraph z3 function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": subgraph contains no gates");
                }
                else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
                {
                    return ERR("could not get subgraph z3 function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": subgraph contains a gate that is a 'nullptr'");
                }
                else if (net == nullptr)
                {
                    return ERR("could not get subgraph z3 function: net is a 'nullptr'");
                }
                else if (net->get_num_of_sources() > 1)
                {
                    return ERR("could not get subgraph z3 function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has more than one source");
                }
                // else if (net->is_global_input_net())
                // {
                //     const auto net_dec = BooleanFunctionNetDecorator(*net);
                //     return OK(ctx.bv_const(net_dec.get_boolean_variable_name().c_str(), 1));
                // }
                // else if (net->get_num_of_sources() == 0)
                // {
                //     return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has no sources");
                // }

                return get_prefixed_function_of_net(subgraph_gates, net, variable_prefix, ctx, net_cache, gate_cache);
            }

            Result<z3::expr> get_prefixed_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, const std::string& variable_prefix, z3::context& ctx)
            {
                std::map<u32, z3::expr> net_cache;
                std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;

                return get_prefixed_subgraph_z3_function_internal(subgraph_gates, subgraph_output, variable_prefix, ctx, net_cache, gate_cache);
            }
        }    // namespace

        namespace
        {
            std::unordered_map<Gate*, std::vector<std::string>> restore_ff_replacements(const Netlist* nl)
            {
                std::unordered_map<Gate*, std::vector<std::string>> replacements;

                for (auto& g : nl->get_gates())
                {
                    if (g->has_data("preprocessing_information", "replaced_gates"))
                    {
                        const auto& [_, s]                           = g->get_data("preprocessing_information", "replaced_gates");
                        std::vector<std::string> replaced_gate_names = nlohmann::json::parse(s);
                        replacements.insert({g, replaced_gate_names});
                    }
                }

                return replacements;
            }
        }    // namespace

        namespace
        {
            Result<std::monostate> substitute_net_ids(z3::context& ctx, z3::solver& s, const std::vector<Net*>& nets, const std::string& variable_prefix, const Netlist* nl)
            {
                // replace nets
                for (const auto& net : nets)
                {
                    if (net->get_sources().size() > 1)
                    {
                        return ERR("cannot replace net id for net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + " because it is multi driven!");
                    }

                    const std::string var_name = variable_prefix + BooleanFunctionNetDecorator(*net).get_boolean_variable_name();
                    const auto sources = net->get_sources([](const Endpoint* ep) { return (ep->get_gate() != nullptr) && (ep->get_gate()->get_type()->has_property(GateTypeProperty::sequential)); });
                    if (net->is_global_input_net())
                    {
                        const auto pin = nl->get_top_module()->get_pin_by_net(net);

                        if (pin == nullptr)
                        {
                            return ERR("cannot replace net id for net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": net is global input but unable to find pin at top module!");
                        }

                        const z3::expr new_expr = ctx.bv_const(("GLOBAL_IN_" + pin->get_name()).c_str(), 1);
                        const z3::expr net_expr = ctx.bv_const(var_name.c_str(), 1);

                        // TODO remove
                        // std::cout << "Added global IO constraint: " << (net_expr == new_expr) << std::endl;

                        s.add(net_expr == new_expr);
                        continue;
                    }
                    
                    if (sources.empty())
                    {
                        // TODO this is not an ideal solution, but i dont know a better one
                        log_debug("z3_utils", "No source found for net {}. Cannot replace net and will rename with net name {}!", net->get_id(), net->get_name());

                        const z3::expr new_expr = ctx.bv_const(net->get_name().c_str(), 1);
                        const z3::expr net_expr = ctx.bv_const(var_name.c_str(), 1);

                        // TODO remove
                        // std::cout << "Added constraint: " << (net_expr == new_expr) << std::endl;

                        s.add(net_expr == new_expr);
                        continue;
                    }

                    const Endpoint* src = sources.front();

                    if (src->get_gate() == nullptr)
                    {
                        log_warning("z3_utils", "No source gate found for net {}. Cannot replace net and will rename with net name {}!", net->get_id(), net->get_name());

                        const z3::expr new_expr = ctx.bv_const(net->get_name().c_str(), 1);
                        const z3::expr net_expr = ctx.bv_const(var_name.c_str(), 1);

                        s.add(net_expr == new_expr);
                        continue;
                    }

                    const std::string src_gate_name = src->get_gate()->get_name();
                    const std::string src_pin       = src->get_pin()->get_name();
                    const std::string name          = src_gate_name + "_" + src_pin;
                    const z3::expr new_expr         = ctx.bv_const(name.c_str(), 1);
                    const z3::expr net_expr         = ctx.bv_const(var_name.c_str(), 1);

                    s.add(net_expr == new_expr);
                }

                return OK({});
            }

            /*
            Result<std::monostate> substitute_net_ids(z3::context& ctx, z3::solver& s, const z3::expr& bf, const std::string& variable_prefix, const Netlist* nl)
            {
                const auto input_vars = z3_utils::get_variable_names(bf);

                std::vector<Net*> nets;

                // replace nets form netlist_b
                for (const std::string var : input_vars)
                {
                    auto net_id_res = BooleanFunctionNetDecorator::get_net_id_from(utils::replace(var, variable_prefix, std::string("")));
                    if (net_id_res.is_error())
                    {
                        return ERR_APPEND(net_id_res.get_error(), "cannot replace net id for unknown net: failed to extract id from variable name.");
                    }
                    const u32 net_id = net_id_res.get();

                    Net* net = nl->get_net_by_id(net_id);
                    nets.push_back(net);
                }

                return substitute_net_ids(ctx, s, nets, variable_prefix, nl);
            }
            */

            void add_replacements_equal_constraints(z3::context& ctx, z3::solver& s, const std::unordered_map<Gate*, std::vector<std::string>>& replacements)
            {
                for (const auto& [g, replaced_names] : replacements)
                {
                    for (const auto& ep : g->get_fan_out_endpoints())
                    {
                        const std::string g_name = g->get_name() + "_" + ep->get_pin()->get_name();
                        const z3::expr g_expr    = ctx.bv_const(g_name.c_str(), 1);

                        for (const auto& name : replaced_names)
                        {
                            const std::string replaced_name = name + "_" + ep->get_pin()->get_name();
                            const z3::expr replaced_expr    = ctx.bv_const(replaced_name.c_str(), 1);

                            s.add(g_expr == replaced_expr);
                        }
                    }
                }

                return;
            }

            Result<std::monostate> setup_solver(z3::context& ctx, z3::solver& s, const Netlist* netlist_a, const Netlist* netlist_b, const std::unordered_map<hal::Gate *, std::vector<std::string>>& ff_replacements_a, const std::unordered_map<hal::Gate *, std::vector<std::string>>& ff_replacements_b)
            {
                std::vector<Net*> sub_nets_a;
                for (const auto& n : netlist_a->get_nets())
                {
                    const auto comb_sources = n->get_sources([](const Endpoint* ep) { return (ep->get_gate() != nullptr) && (ep->get_gate()->get_type()->has_property(GateTypeProperty::combinational)); });
                    if (comb_sources.empty())
                    {
                        sub_nets_a.push_back(n);
                    }
                }
                std::vector<Net*> sub_nets_b;
                for (const auto& n : netlist_b->get_nets())
                {
                    const auto comb_sources = n->get_sources([](const Endpoint* ep) { return (ep->get_gate() != nullptr) && (ep->get_gate()->get_type()->has_property(GateTypeProperty::combinational)); });
                    if (comb_sources.empty())
                    {
                        sub_nets_b.push_back(n);
                    }
                }

                auto sub_a_res = substitute_net_ids(ctx, s, sub_nets_a, "netlist_a_", netlist_a);
                if (sub_a_res.is_error())
                {
                    return ERR_APPEND(sub_a_res.get_error(), "cannot compare netA" + std::to_string(netlist_a->get_id()));
                }
                auto sub_b_res = substitute_net_ids(ctx, s, sub_nets_b, "netlist_b_", netlist_b);
                if (sub_b_res.is_error())
                {
                    return ERR_APPEND(sub_b_res.get_error(), "cannot compare net A " + std::to_string(netlist_b->get_id()));
                }

                add_replacements_equal_constraints(ctx, s, ff_replacements_a);
                add_replacements_equal_constraints(ctx, s, ff_replacements_b);

                return OK({});
            }

            Result<bool> compare_nets_internal(z3::context& ctx,
                                               z3::solver& s,
                                               //    const Netlist* netlist_a,
                                               //    const Netlist* netlist_b,
                                               const Net* net_a,
                                               const Net* net_b,
                                               const std::vector<Gate*>& gates_a,
                                               const std::vector<Gate*>& gates_b,
                                               const bool fail_on_unknown,
                                               const u32 solver_timeout)
            {
                // TODO Debug printing remove
                /*
                std::cout << "Comparing net A: " << net_a->get_id() << " / " << net_a->get_name() << " with " << net_b->get_id() << " / " << net_b->get_name() << std::endl;
                const auto inputs_a = SubgraphNetlistDecorator(*netlist_a).get_subgraph_function_inputs(gates_a, net_a).get();
                std::cout << "Function A inputs: " << inputs_a.size() << std::endl;
                const auto inputs_b = SubgraphNetlistDecorator(*netlist_b).get_subgraph_function_inputs(gates_b, net_b).get();
                std::cout << "Function B inputs: " << inputs_b.size() << std::endl;
                */

                const auto bf_res_a = z3_utils::get_prefixed_subgraph_z3_function(gates_a, net_a, "netlist_a_", ctx);
                if (bf_res_a.is_error())
                {
                    return ERR_APPEND(bf_res_a.get_error(),
                                      "cannot compare net A " + net_a->get_name() + " with ID " + std::to_string(net_a->get_id()) + " with net B " + net_b->get_name() + " with ID "
                                          + std::to_string(net_b->get_id()) + ": failed to build subgraph function for net a");
                }
                const auto bf_a = bf_res_a.get();

                const auto bf_res_b = z3_utils::get_prefixed_subgraph_z3_function(gates_b, net_b, "netlist_b_", ctx);
                if (bf_res_b.is_error())
                {
                    return ERR_APPEND(bf_res_b.get_error(),
                                      "cannot compare net A " + net_a->get_name() + " with ID " + std::to_string(net_a->get_id()) + " with net B " + net_b->get_name() + " with ID "
                                          + std::to_string(net_b->get_id()) + ": failed to build subgraph function for net b");
                }
                const auto bf_b = bf_res_b.get();

                auto config = hal::SMT::QueryConfig().with_timeout(solver_timeout).without_model_generation();

                // TODO we can enable this again after we add time out capabilities to the bitwuzla call
                // if (SMT::Solver::has_local_solver_for(hal::SMT::SolverType::Bitwuzla, hal::SMT::SolverCall::Library))
                // {
                //     config = config.with_solver(hal::SMT::SolverType::Bitwuzla).with_call(hal::SMT::SolverCall::Library);
                // }

                s.add(bf_a != bf_b);
                auto smt2_str = s.to_smt2();
                auto query_res = SMT::Solver::query_local(config, smt2_str);
                if (query_res.is_error())
                {
                    return ERR_APPEND(query_res.get_error(), "cannot compare net A " + net_a->get_name() + " with ID " + std::to_string(net_a->get_id()) + " with net B " + net_b->get_name() + " with ID "
                                          + std::to_string(net_b->get_id()) + ": failed solver_check");
                }
                const auto check_result = query_res.get();

                if (check_result.is_unsat())
                {
                    return OK(true);
                }

                if (check_result.is_unknown())
                {
                    return OK(!fail_on_unknown);
                }

                return OK(false);
            }
        }    // namespace

        Result<bool> compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const Net* net_a, const Net* net_b, const bool fail_on_unknown, const u32 solver_timeout)
        {
            if (netlist_a == nullptr)
            {
                return ERR("cannot compare nets: netlist_a is a nullptr!");
            }

            if (netlist_b == nullptr)
            {
                return ERR("cannot compare nets: netlist_b is a nullptr!");
            }

            if (net_a == nullptr)
            {
                return ERR("cannot compare nets: net_a is a nullptr!");
            }

            if (net_b == nullptr)
            {
                return ERR("cannot compare nets: net_b is a nullptr!");
            }

            z3::context ctx;
            z3::solver s(ctx);

            const auto ff_replacements_a = restore_ff_replacements(netlist_a);
            const auto ff_replacements_b = restore_ff_replacements(netlist_b);

            const std::vector<Gate*> comb_gates_a = netlist_a->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });
            const std::vector<Gate*> comb_gates_b = netlist_b->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

            auto setup_res = setup_solver(ctx, s, netlist_a, netlist_b, ff_replacements_a, ff_replacements_b);
            if (setup_res.is_error())
            {
                return ERR_APPEND(setup_res.get_error(), "cannot compare netlist a with ID " + std::to_string(netlist_a->get_id()) + " netlist b with ID " + std::to_string(netlist_b->get_id()) + ": failed to setup solver");
            }

            return compare_nets_internal(ctx, s, net_a, net_b, comb_gates_a, comb_gates_b, fail_on_unknown, solver_timeout);
        }

        Result<bool> compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const std::vector<std::pair<Net*, Net*>>& nets, const bool fail_on_unknown, const u32 solver_timeout)
        {
            if (netlist_a == nullptr)
            {
                return ERR("cannot compare nets: netlist_a is a nullptr!");
            }

            if (netlist_b == nullptr)
            {
                return ERR("cannot compare nets: netlist_b is a nullptr!");
            }

            z3::context ctx;
            z3::solver s(ctx);

            const auto ff_replacements_a = restore_ff_replacements(netlist_a);
            const auto ff_replacements_b = restore_ff_replacements(netlist_b);

            const std::vector<Gate*> comb_gates_a = netlist_a->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });
            const std::vector<Gate*> comb_gates_b = netlist_b->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

            auto setup_res = setup_solver(ctx, s, netlist_a, netlist_b, ff_replacements_a, ff_replacements_b);
            if (setup_res.is_error())
            {
                return ERR_APPEND(setup_res.get_error(), "cannot compare netlist a with ID " + std::to_string(netlist_a->get_id()) + " netlist b with ID " + std::to_string(netlist_b->get_id()) + ": failed to setup solver");
            }

            for (const auto& [net_a, net_b] : nets)
            {
                if ((net_a == nullptr) && (net_b == nullptr))
                {
                    continue;
                }

                if ((net_a == nullptr) || (net_b == nullptr))
                {
                    return OK(false);
                }

                s.push();
                auto comp_res = compare_nets_internal(ctx, s, net_a, net_b, comb_gates_a, comb_gates_b, fail_on_unknown, solver_timeout);
                s.pop();
                
                if (comp_res.is_error())
                {
                    return ERR_APPEND(comp_res.get_error(),
                                      "cannot compare netlist a with ID " + std::to_string(netlist_a->get_id()) + " netlist b with ID " + std::to_string(netlist_b->get_id())
                                          + ": failed net comparison");
                }
                const auto eq = comp_res.get();

                if (!eq)
                {
                    log_warning("z3_utils", "Failed net comparison for net A {} / {}  and net B {} / {}", net_a->get_id(), net_a->get_name(), net_b->get_id(), net_b->get_name());
                    return OK(false);
                }
            }

            return OK(true);
        }

        Result<bool> compare_netlists(const Netlist* netlist_a, const Netlist* netlist_b, const bool fail_on_unknown, const u32 solver_timeout)
        {
            const std::vector<Gate*> seq_gates_a = netlist_a->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
            const std::vector<Gate*> seq_gates_b = netlist_b->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });

            const std::vector<Gate*> comb_gates_a = netlist_a->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });
            const std::vector<Gate*> comb_gates_b = netlist_b->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

            const auto ff_replacements_a = restore_ff_replacements(netlist_a);
            const auto ff_replacements_b = restore_ff_replacements(netlist_b);

            // TODO let the user provide a name mapping from gate a to net b incase there was a renaming of the sequential gates
            std::unordered_map<std::string, Gate*> gate_name_to_gate_a;
            std::unordered_map<std::string, Gate*> gate_name_to_gate_b;

            for (const auto& gate_a : seq_gates_a)
            {
                gate_name_to_gate_a[gate_a->get_name()] = gate_a;

                if (const auto& it = ff_replacements_a.find(gate_a); it != ff_replacements_a.end())
                {
                    for (const auto& s : it->second)
                    {
                        gate_name_to_gate_a[s] = gate_a;
                    }
                }
            }
            for (const auto& gate_b : seq_gates_b)
            {
                gate_name_to_gate_b[gate_b->get_name()] = gate_b;

                if (const auto& it = ff_replacements_b.find(gate_b); it != ff_replacements_b.end())
                {
                    for (const auto& s : it->second)
                    {
                        gate_name_to_gate_b[s] = gate_b;
                    }
                }
            }

            for (const auto& [gate_a_name, gate_a] : gate_name_to_gate_a)
            {
                if (const auto gate_b_it = gate_name_to_gate_b.find(gate_a_name); gate_b_it == gate_name_to_gate_b.end())
                {
                    log_debug("z3_utils",
                              "netlist a with ID {} and netlist b with ID {} are not equal: gate a {} with ID {} is included in netlist a but does not have a counter part in netlist b!",
                              netlist_a->get_id(),
                              netlist_b->get_id(),
                              gate_a->get_name(),
                              gate_a->get_id());
                    return OK(false);
                }
            }
            for (const auto& [gate_b_name, gate_b] : gate_name_to_gate_b)
            {
                if (const auto gate_a_it = gate_name_to_gate_a.find(gate_b_name); gate_a_it == gate_name_to_gate_a.end())
                {
                    log_debug("z3_utils",
                              "netlist a with ID {} and netlist b with ID {} are not equal: gate b {} with ID {} is included in netlist b but does not have a counter part in netlist a!",
                              netlist_a->get_id(),
                              netlist_b->get_id(),
                              gate_b->get_name(),
                              gate_b->get_id());
                    return OK(false);
                }
            }

            log_info("z3_utils", "Checking {} sequential gates for equality.", seq_gates_a.size());

            z3::context ctx;
            z3::solver s(ctx);

            auto setup_res = setup_solver(ctx, s, netlist_a, netlist_b, ff_replacements_a, ff_replacements_b);
            if (setup_res.is_error())
            {
                return ERR_APPEND(setup_res.get_error(), "cannot compare netlist a with ID " + std::to_string(netlist_a->get_id()) + " netlist b with ID " + std::to_string(netlist_b->get_id()) + ": failed to setup solver");
            }

            std::set<std::pair<Net*, Net*>> to_compare;

            // find matching global output and add the to the comparison set
            const auto out_pins_a = netlist_a->get_top_module()->get_output_pin_names();
            const auto out_pins_b = netlist_b->get_top_module()->get_output_pin_names();

            auto all_out_pins = out_pins_a;
            all_out_pins.insert(all_out_pins.end(), out_pins_b.begin(), out_pins_b.end());

            for (const auto& pin : all_out_pins)
            {
                auto it_a = std::find(out_pins_a.begin(), out_pins_a.end(), pin);
                if (it_a == out_pins_a.end())
                {
                    log_warning("z3_utils",
                            "netlist a with ID {} and netlist b with ID {} might not be equal: netlist a has output pin {} that does not exist in netlist b!",
                            netlist_a->get_id(),
                            netlist_b->get_id(),
                            pin);
                    continue;
                }

                auto it_b = std::find(out_pins_b.begin(), out_pins_b.end(), pin);
                if (it_b == out_pins_b.end())
                {
                    log_warning("z3_utils",
                            "netlist a with ID {} and netlist b with ID {} might not be equal: netlist a has output pin {} that does not exist in netlist b!",
                            netlist_a->get_id(),
                            netlist_b->get_id(),
                            pin);
                    continue;
                }

                Net* net_a = netlist_a->get_top_module()->get_pin_by_name(pin)->get_net();
                Net* net_b = netlist_b->get_top_module()->get_pin_by_name(pin)->get_net();
            
                to_compare.insert({net_a, net_b});
            }

            // add inputs of sequential gates to the comparison list
            for (const Gate* gate_a : seq_gates_a)
            {
                const Gate* gate_b = gate_name_to_gate_b.at(gate_a->get_name());

                if (gate_a->get_type() != gate_b->get_type())
                {
                    log_warning("z3_utils",
                                "netlist a with ID {} and netlist b with ID {} are not equal: gate a {} with ID {} and gate b {} with ID {} do not have the same type! {} vs. {}",
                                netlist_a->get_id(),
                                netlist_b->get_id(),
                                gate_a->get_name(),
                                gate_a->get_id(),
                                gate_b->get_name(),
                                gate_b->get_id(),
                                gate_a->get_type()->get_name(),
                                gate_b->get_type()->get_name());
                    return OK(false);
                }

                for (const GatePin* pin : gate_a->get_type()->get_input_pins())
                {
                    Net* net_a = gate_a->get_fan_in_net(pin);
                    Net* net_b = gate_b->get_fan_in_net(pin);

                    to_compare.insert({net_a, net_b});
                }
            }

            return compare_nets(netlist_a, netlist_b, utils::to_vector(to_compare), fail_on_unknown, solver_timeout);
        }
    }    // namespace z3_utils
}    // namespace hal