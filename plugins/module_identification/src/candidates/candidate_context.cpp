#include "module_identification/candidates/candidate_context.h"

#include "boolean_influence/boolean_influence.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    namespace module_identification
    {
        CandidateContext::CandidateContext(const Netlist* nl, const std::vector<Gate*>& gates) : m_gates(gates), m_netlist(nl){};

#ifdef HAL_CANDIDATE_CONTEXT
        Result<std::monostate> CandidateContext::populate_boolean_function_cache(const std::vector<Net*> nets)
        {
            const auto subgraph_dec = SubgraphNetlistDecorator(*m_netlist);
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;

            for (const auto& n : nets)
            {
                auto bf_res = subgraph_dec.get_subgraph_function(m_gates, n, gate_cache);
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(),
                                      "cannot populate context with Boolean functions: failed to generate subgraph function for net " + n->get_name() + " with ID " + std::to_string(n->get_id()));
                }
                auto bf = bf_res.get();

                auto sub_res = BooleanFunctionDecorator(bf).substitute_power_ground_nets(m_netlist);
                if (sub_res.is_error())
                {
                    return ERR_APPEND(sub_res.get_error(),
                                      "cannot populate context with Boolean functions: failed to substitute power and ground nets for net " + n->get_name() + " with ID "
                                          + std::to_string(n->get_id()));
                }

                m_boolean_function_cache.insert({{n, {}}, sub_res.get().simplify_local()});
            }

            return OK({});
        }

        Result<const BooleanFunction> CandidateContext::get_boolean_function_const(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping) const
        {
            if (const auto it = m_boolean_function_cache.find({n, ctrl_mapping}); it != m_boolean_function_cache.end())
            {
                return OK(it->second);
            }

            return ERR("Failed to get boolean function from chache");
        }

        Result<std::vector<BooleanFunction>> CandidateContext::get_boolean_functions_const(const std::vector<Net*> nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping) const
        {
            std::vector<BooleanFunction> functions;
            for (const auto& net : nets)
            {
                const auto bf_res = get_boolean_function_const(net, ctrl_mapping);
                if (bf_res.is_error())
                {
                    return ERR(bf_res.get_error().get());
                }
                functions.push_back(bf_res.get().clone());
            }

            return OK(functions);
        }

        Result<const BooleanFunction> CandidateContext::get_boolean_function(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_function_cache.find({n, ctrl_mapping}); it != m_boolean_function_cache.end())
            {
                return OK(it->second);
            }

            if (ctrl_mapping.empty())
            {
                auto func_res = SubgraphNetlistDecorator(*m_netlist).get_subgraph_function(m_gates, n);
                if (func_res.is_error())
                {
                    return ERR_APPEND(func_res.get_error(),
                                      "cannot get Boolean function from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed subgraph function generation");
                }
                auto bf = func_res.get();

                auto sub_res = BooleanFunctionDecorator(bf).substitute_power_ground_nets(m_netlist);
                if (sub_res.is_error())
                {
                    return ERR_APPEND(sub_res.get_error(),
                                      "cannot get Boolean function from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to substitute power/ground nets");
                }
                auto sub_bf = sub_res.get().simplify_local();

                const auto [it, _] = m_boolean_function_cache.insert({{n, ctrl_mapping}, sub_bf});
                return OK(it->second);
            }

            const auto bf_res = get_boolean_function(n, {});
            if (bf_res.is_error())
            {
                return bf_res;
            }
            const auto& bf = bf_res.get();

            std::map<std::string, BooleanFunction> substitution_map;
            for (const auto& [n_ctrl, val] : ctrl_mapping)
            {
                substitution_map.insert({BooleanFunctionNetDecorator(*n_ctrl).get_boolean_variable_name(), BooleanFunction::Const(val)});
            }
            auto sub_res = bf.substitute(substitution_map);
            if (sub_res.is_error())
            {
                return ERR_APPEND(sub_res.get_error(),
                                  "cannot get Boolean function from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to substitute with input mapping");
            }
            const auto sub_bf = sub_res.get().simplify_local();

            const auto [it, _] = m_boolean_function_cache.insert({{n, ctrl_mapping}, sub_bf});
            return OK(it->second);
        }

        Result<std::vector<BooleanFunction>> CandidateContext::get_boolean_functions(const std::vector<Net*> nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            std::vector<BooleanFunction> functions;
            for (const auto& net : nets)
            {
                const auto bf_res = get_boolean_function(net, ctrl_mapping);
                if (bf_res.is_error())
                {
                    return ERR(bf_res.get_error().get());
                }
                functions.push_back(bf_res.get().clone());
            }

            return OK(functions);
        }

        Result<const std::set<std::string>> CandidateContext::get_variable_names(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_vars_cache.find({n, ctrl_mapping}); it != m_boolean_vars_cache.end())
            {
                return OK(it->second);
            }

            const auto bf_res = get_boolean_function(n, ctrl_mapping);
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(),
                                  "cannot get variable names from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to retrieve Boolean function from context");
            }
            const auto& bf = bf_res.get();

            const auto [it, _] = m_boolean_vars_cache.insert({{n, ctrl_mapping}, bf.get_variable_names()});
            return OK(it->second);
        }

        Result<const std::set<Net*>> CandidateContext::get_variable_nets(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            const auto& var_names_res = get_variable_names(n, ctrl_mapping);
            if (var_names_res.is_error())
            {
                return ERR_APPEND(var_names_res.get_error(),
                                  "cannot get variable nets from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to retrieve variable names from context");
            }
            const auto& var_names = var_names_res.get();

            std::set<Net*> result;
            for (const auto& var : var_names)
            {
                result.insert(BooleanFunctionNetDecorator::get_net_from(m_netlist, var).get());
            }

            return OK(result);
        }

        Result<std::unordered_map<std::string, double>> CandidateContext::get_boolean_influence(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_influence_cache.find({n, ctrl_mapping}); it != m_boolean_influence_cache.end())
            {
                return OK(it->second);
            }

            const auto bf_res = get_boolean_function(n, ctrl_mapping);
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(),
                                  "cannot get Boolean influence from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id())
                                      + ": failed to retrieve Boolean function from context");
            }
            const auto& bf = bf_res.get();

            const auto res = boolean_influence::get_boolean_influence(bf, 1024);
            // const auto res = boolean_influence::get_boolean_influence(bf, 32000);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "cannot get Boolean influence from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to calculate Boolean influence");
            }

            const auto [it, _] = m_boolean_influence_cache.insert({{n, ctrl_mapping}, res.get()});
            return OK(it->second);
        }

        Result<std::vector<BooleanFunction::Value>>
            CandidateContext::evaluate(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping, const std::map<std::string, BooleanFunction::Value>& eval_mapping)
        {
            const auto bf                                                                  = get_boolean_function(n, ctrl_mapping).get();
            std::unordered_map<std::string, BooleanFunction::Value> unordered_eval_mapping = {eval_mapping.begin(), eval_mapping.end()};
            return OK({bf.evaluate(unordered_eval_mapping).get()});
        }
#endif

#ifdef Z3_CANDIDATE_CONTEXT
        Result<std::monostate> CandidateContext::populate_boolean_function_cache(const std::vector<Net*> nets)
        {
            auto func_res = z3_utils::get_subgraph_z3_functions(m_gates, nets, m_ctx);
            if (func_res.is_error())
            {
                // TODO make all this return results
                log_error("module_identification", "Error on boolean sugraph function creation, for nets: {}", func_res.get_error().get());
            }
            auto functions = func_res.get();

            for (u32 idx = 0; idx < nets.size(); idx++)
            {
                auto bf         = functions.at(idx);
                const auto vars = z3_utils::get_variable_names(bf);

                // substitute power and ground nets
                z3::expr_vector from(m_ctx);
                z3::expr_vector to(m_ctx);

                for (const std::string& var_name : vars)
                {
                    const auto net_res = BooleanFunctionNetDecorator::get_net_from(m_netlist, var_name);
                    if (net_res.is_error())
                    {
                        log_error("module_identification", "Error when trying to replace gnd and vcc nets in boolean function for variable '{}': {}", var_name, net_res.get_error().get());
                    }
                    const Net* net = net_res.get();

                    if (const auto sources = net->get_sources(); sources.size() == 1)
                    {
                        if (sources.front()->get_gate()->get_type()->has_property(GateTypeProperty::power))
                        {
                            from.push_back(m_ctx.bv_const(var_name.c_str(), 1));
                            to.push_back(m_ctx.bv_val(1, 1));
                        }
                        else if (sources.front()->get_gate()->get_type()->has_property(GateTypeProperty::ground))
                        {
                            from.push_back(m_ctx.bv_const(var_name.c_str(), 1));
                            to.push_back(m_ctx.bv_val(0, 1));
                        }
                    }
                }

                bf = bf.substitute(from, to).simplify();

                const auto s_res = z3_utils::simplify_local(bf);
                if (s_res.is_error())
                {
                    std::cout << "ERROR: " << s_res.get_error().get() << std::endl;
                }
                bf = s_res.get();

                m_boolean_function_cache.insert({{nets.at(idx), {}}, bf});
            }

            return OK({});
        }

        Result<z3::expr> CandidateContext::get_boolean_function(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_function_cache.find({n, ctrl_mapping}); it != m_boolean_function_cache.end())
            {
                return OK(it->second);
            }

            if (ctrl_mapping.empty())
            {
                auto func_res = z3_utils::get_subgraph_z3_function(m_gates, n, m_ctx);
                if (func_res.is_error())
                {
                    // TODO make all this return results
                    log_error("module_identification", "Error on boolean sugraph function creation, for nets: {}", func_res.get_error().get());
                }
                auto bf = func_res.get();

                const auto vars = z3_utils::get_variable_names(bf);

                // substitute power and ground nets
                z3::expr_vector from_power_gnd(m_ctx);
                z3::expr_vector to_power_gnd(m_ctx);

                for (const std::string& var_name : vars)
                {
                    const auto net_res = BooleanFunctionNetDecorator::get_net_from(m_netlist, var_name);
                    if (net_res.is_error())
                    {
                        log_error("module_identification", "Error when trying to replace gnd and vcc nets in boolean function for variable '{}': {}", var_name, net_res.get_error().get());
                    }
                    const Net* net = net_res.get();

                    if (const auto sources = net->get_sources(); sources.size() == 1)
                    {
                        if (sources.front()->get_gate()->get_type()->has_property(GateTypeProperty::power))
                        {
                            from_power_gnd.push_back(m_ctx.bv_const(var_name.c_str(), 1));
                            to_power_gnd.push_back(m_ctx.bv_val(1, 1));
                        }
                        else if (sources.front()->get_gate()->get_type()->has_property(GateTypeProperty::ground))
                        {
                            from_power_gnd.push_back(m_ctx.bv_const(var_name.c_str(), 1));
                            to_power_gnd.push_back(m_ctx.bv_val(0, 1));
                        }
                    }
                }

                bf = bf.substitute(from_power_gnd, to_power_gnd).simplify();

                const auto s_res = z3_utils::simplify_local(bf);
                if (s_res.is_error())
                {
                    std::cout << "ERROR: " << s_res.get_error().get() << std::endl;
                }
                bf = s_res.get();

                m_boolean_function_cache.insert({{n, {}}, bf});
            }

            auto bf = get_boolean_function(n, {}).get();

            z3::expr_vector from(m_ctx);
            z3::expr_vector to(m_ctx);
            for (const auto& [cn, val] : ctrl_mapping)
            {
                from.push_back(m_ctx.bv_const(BooleanFunctionNetDecorator(*cn).get_boolean_variable_name().c_str(), 1));
                to.push_back(m_ctx.bv_val(val == BooleanFunction::Value::ZERO ? 0 : 1, 1));
            }
            bf = bf.substitute(from, to).simplify();

            const auto s_res = z3_utils::simplify_local(bf);
            if (s_res.is_error())
            {
                std::cout << "ERROR: " << s_res.get_error().get() << std::endl;
            }
            bf = s_res.get();

            const auto it = m_boolean_function_cache.insert({{n, ctrl_mapping}, bf});
            return OK(it.first->second);
        }

        Result<z3::expr> CandidateContext::get_boolean_function_const(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping) const
        {
            if (const auto it = m_boolean_function_cache.find({n, ctrl_mapping}); it != m_boolean_function_cache.end())
            {
                return OK(it->second);
            }

            std::cout << "Control Mapping: " << std::endl;
            for (const auto& [cn, v] : ctrl_mapping)
            {
                std::cout << "\t" << cn->get_id() << ": " << v << std::endl;
            }

            std::cout << "Candidate Gates: " << std::endl;
            for (const auto& g : this->m_gates)
            {
                std::cout << "\t" << g->get_id() << ": " << g->get_name() << std::endl;
            }

            return ERR("Failed to get boolean function for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + " from boolean function cache");
        }

        Result<BooleanFunction> CandidateContext::get_hal_boolean_function(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_function_hal_cache.find({n, ctrl_mapping}); it != m_boolean_function_hal_cache.end())
            {
                return OK(it->second);
            }

            const auto bf_res = get_boolean_function(n, ctrl_mapping);
            if (bf_res.is_error())
            {
                return ERR(bf_res.get_error().get());
            }

            const auto bf_hal_res = z3_utils::to_bf(bf_res.get());
            if (bf_hal_res.is_error())
            {
                return ERR(bf_hal_res.get_error().get());
            }

            const auto [it, _] = m_boolean_function_hal_cache.insert({{n, ctrl_mapping}, bf_hal_res.get()});
            return OK(it->second);
        }

        Result<BooleanFunction> CandidateContext::get_hal_boolean_function_const(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_function_hal_cache.find({n, ctrl_mapping}); it != m_boolean_function_hal_cache.end())
            {
                return OK(it->second);
            }

            std::cout << "Control Mapping: " << std::endl;
            for (const auto& [cn, v] : ctrl_mapping)
            {
                std::cout << "\t" << cn->get_id() << ": " << v << std::endl;
            }

            std::cout << "Candidate Gates: " << std::endl;
            for (const auto& g : this->m_gates)
            {
                std::cout << "\t" << g->get_id() << ": " << g->get_name() << std::endl;
            }

            return ERR("Failed to get boolean function for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + " from boolean function cache");
        }

        Result<std::vector<BooleanFunction>> CandidateContext::get_boolean_functions(const std::vector<Net*>& nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            std::vector<BooleanFunction> functions;
            for (const auto& net : nets)
            {
                const auto bf_hal_res = get_hal_boolean_function(net, ctrl_mapping);
                if (bf_hal_res.is_error())
                {
                    return ERR(bf_hal_res.get_error().get());
                }

                functions.push_back(bf_hal_res.get());
            }

            return OK(functions);
        }

        Result<std::vector<BooleanFunction>> CandidateContext::get_boolean_functions_const(const std::vector<Net*>& nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            std::vector<BooleanFunction> functions;
            for (const auto& net : nets)
            {
                const auto bf_hal_res = get_hal_boolean_function_const(net, ctrl_mapping);
                if (bf_hal_res.is_error())
                {
                    return ERR(bf_hal_res.get_error().get());
                }

                functions.push_back(bf_hal_res.get());
            }

            return OK(functions);
        }

        Result<std::set<std::string>> CandidateContext::get_variable_names(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_vars_cache.find({n, ctrl_mapping}); it != m_boolean_vars_cache.end())
            {
                return OK(it->second);
            }

            const auto bf_res = get_boolean_function(n, ctrl_mapping);
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(),
                                  "cannot get variable names from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to retrieve Boolean function from context");
            }
            const auto& bf = bf_res.get();

            const auto [it, _] = m_boolean_vars_cache.insert({{n, ctrl_mapping}, z3_utils::get_variable_names(bf)});
            return OK(it->second);
        }

        Result<std::set<Net*>> CandidateContext::get_variable_nets(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            const auto& var_names_res = get_variable_names(n, ctrl_mapping);
            if (var_names_res.is_error())
            {
                return ERR_APPEND(var_names_res.get_error(),
                                  "cannot get variable nets from context for net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to retrieve variable names from context");
            }
            const auto& var_names = var_names_res.get();

            std::set<Net*> result;
            for (const auto& var : var_names)
            {
                result.insert(BooleanFunctionNetDecorator::get_net_from(m_netlist, var).get());
            }

            return OK(result);
        }

        Result<std::unordered_map<std::string, double>> CandidateContext::get_boolean_influence(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping)
        {
            if (const auto it = m_boolean_influence_cache.find({n, ctrl_mapping}); it != m_boolean_influence_cache.end())
            {
                return OK(it->second);
            }

            const auto& bf                                      = get_boolean_function(n, ctrl_mapping).get();
            Result<std::unordered_map<std::string, double>> res = boolean_influence::get_boolean_influence(bf, 1024);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "failed to create boolean influence");
            }
            m_boolean_influence_cache.insert({{n, ctrl_mapping}, res.get()});

            return OK(m_boolean_influence_cache.at({n, ctrl_mapping}));
        }

        Result<std::vector<BooleanFunction::Value>>
            CandidateContext::evaluate(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping, const std::map<std::string, BooleanFunction::Value>& eval_mapping)
        {
            auto bf_z3 = get_boolean_function(n, ctrl_mapping).get();

            z3::expr_vector from(m_ctx);
            z3::expr_vector to(m_ctx);

            // Iterate over the map of variables and their assigned integer values.
            for (const auto& [var_name, val] : eval_mapping)
            {
                z3::expr var      = m_ctx.bv_const(var_name.c_str(), 1);    // Create a variable expression.
                z3::expr val_expr = m_ctx.bv_val(val, 1);                   // Create a value expression.
                from.push_back(var);                                        // Prepare for substitution.
                to.push_back(val_expr);
            }

            // Substitute variables in the original expression with the provided values.
            z3::expr result = bf_z3.substitute(from, to).simplify();

            if (!result.is_numeral())
            {
                return ERR("eval result is non numeral");
            }

            const u64 res_int = result.get_numeral_uint64();

            if (res_int == 1)
            {
                return OK({BooleanFunction::Value::ONE});
            }

            if (res_int == 0)
            {
                return OK({BooleanFunction::Value::ZERO});
            }

            return ERR("invalid value " + std::to_string(res_int) + " as result of eval");
        };
#endif
    }    // namespace module_identification
}    // namespace hal
