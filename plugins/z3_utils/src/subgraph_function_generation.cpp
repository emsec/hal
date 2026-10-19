#include "z3_utils/subgraph_function_generation.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"
#include "z3_utils/z3_utils.h"

#include <queue>

namespace hal
{
    namespace z3_utils
    {
        namespace
        {
            Result<z3::expr> get_function_of_net(const std::vector<Gate*>& subgraph_gates,
                                                 const Net* net,
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
                    z3::expr ret = ctx.bv_const(BooleanFunctionNetDecorator(*net).get_boolean_variable_name().c_str(), 1);
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
                    z3::expr ret = ctx.bv_const(BooleanFunctionNetDecorator(*net).get_boolean_variable_name().c_str(), 1);
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

                    const auto in_bf_res = get_function_of_net(subgraph_gates, in_net, ctx, net_cache, gate_cache);
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

            Result<z3::expr> get_subgraph_z3_function_internal(const std::vector<Gate*>& subgraph_gates,
                                                               const Net* net,
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
                else if (net->is_global_input_net())
                {
                    const auto net_dec = BooleanFunctionNetDecorator(*net);
                    return OK(ctx.bv_const(net_dec.get_boolean_variable_name().c_str(), 1));
                }
                else if (net->get_num_of_sources() == 0)
                {
                    return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has no sources");
                }

                return get_function_of_net(subgraph_gates, net, ctx, net_cache, gate_cache);
            }

        }    // namespace

        Result<z3::expr> get_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, z3::context& ctx)
        {
            std::map<u32, z3::expr> net_cache;
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;

            return get_subgraph_z3_function_internal(subgraph_gates, subgraph_output, ctx, net_cache, gate_cache);
        }

        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*>& subgraph_outputs, z3::context& ctx)
        {
            std::map<u32, z3::expr> net_cache;
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;

            std::vector<z3::expr> results;

            for (const auto& net : subgraph_outputs)
            {
                const auto res = get_subgraph_z3_function_internal(subgraph_gates, net, ctx, net_cache, gate_cache);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "unable to generate subgraph functions: failed to generate function for net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ".");
                }

                results.push_back(res.get());
            }

            return OK(results);
        }

    }    // namespace z3_utils
}    // namespace hal
