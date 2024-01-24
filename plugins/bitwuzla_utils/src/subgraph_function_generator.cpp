#include "bitwuzla_utils/bitwuzla_utils.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"

#include <queue>
namespace hal
{
    namespace bitwuzla_utils
    {
        namespace
        {
            Result<bitwuzla::Term> get_function_of_net(const std::vector<Gate*>& subgraph_gates,
                                                       const Net* net,
                                                       std::map<u32, bitwuzla::Term>& net_cache,
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
                    return ERR("cannot get Boolean bitwuzla function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": net is multi driven.");
                }

                // net has no source
                if (sources.empty())
                {
                    bitwuzla::Term ret = bitwuzla::mk_const(bitwuzla::mk_bv_sort(1), BooleanFunctionNetDecorator(*net).get_boolean_variable_name());
                    net_cache.insert({net->get_id(), ret});
                    return OK(ret);
                }

                const Endpoint* src_ep = sources.front();

                if (src_ep->get_gate() == nullptr)
                {
                    return ERR("cannot get Boolean bitwuzla function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": net source is null.");
                }

                const Gate* src = src_ep->get_gate();

                // source is not in subgraph gates
                if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src) == subgraph_gates.end())
                {
                    bitwuzla::Term ret = bitwuzla::mk_const(bitwuzla::mk_bv_sort(1), BooleanFunctionNetDecorator(*net).get_boolean_variable_name());
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
                                          "cannot get Boolean bitwuzla function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to get function of gate.");
                    }
                    bf = bf_res.get();

                    gate_cache.insert({{src->get_id(), src_ep->get_pin()}, bf});
                }
                else
                {
                    bf = it->second;
                }

                std::map<std::string, bitwuzla::Term> input_to_term;

                for (const std::string& in_net_str : bf.get_variable_names())
                {
                    const auto in_net_res = BooleanFunctionNetDecorator::get_net_from(src->get_netlist(), in_net_str);
                    if (in_net_res.is_error())
                    {
                        return ERR_APPEND(in_net_res.get_error(),
                                          "cannot get Boolean bitwuzla function of net " + net->get_name() + " with ID " + std::to_string(net->get_id())
                                              + ": failed to reconstruct input net from variable " + in_net_str + ".");
                    }
                    const auto in_net = in_net_res.get();

                    const auto in_bf_res = get_function_of_net(subgraph_gates, in_net, net_cache, gate_cache);
                    if (in_bf_res.is_error())
                    {
                        // NOTE since this can lead to a deep recursion we do not append the error and instead only propagate it.
                        return in_bf_res;
                    }
                    const auto in_bf = in_bf_res.get();

                    input_to_term.insert({in_net_str, in_bf});
                }

                const auto res = bitwuzla_utils::from_bf(bf, input_to_term);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "cannot get Boolean bitwuzla function of net " + net->get_name() + " with ID " + std::to_string(net->get_id())
                                          + ": failed to translate gate function to bitwuzla term");
                }

                const auto ret = res.get();
                net_cache.insert({net->get_id(), ret});
                return OK(ret);
            }

            Result<bitwuzla::Term> get_subgraph_bitwuzla_function_internal(const std::vector<Gate*>& subgraph_gates,
                                                                           const Net* net,
                                                                           std::map<u32, bitwuzla::Term>& net_cache,
                                                                           std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache)
            {
                // check validity of subgraph_gates
                if (subgraph_gates.empty())
                {
                    return ERR("could not get subgraph bitwuzla function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": subgraph contains no gates");
                }
                else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
                {
                    return ERR("could not get subgraph bitwuzla function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": subgraph contains a gate that is a 'nullptr'");
                }
                else if (net == nullptr)
                {
                    return ERR("could not get subgraph bitwuzla function: net is a 'nullptr'");
                }
                else if (net->get_num_of_sources() > 1)
                {
                    return ERR("could not get subgraph bitwuzla function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has more than one source");
                }
                else if (net->is_global_input_net())
                {
                    return OK(bitwuzla::mk_const(bitwuzla::mk_bv_sort(1), BooleanFunctionNetDecorator(*net).get_boolean_variable_name()));
                }
                else if (net->get_num_of_sources() == 0)
                {
                    return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has no sources");
                }

                return get_function_of_net(subgraph_gates, net, net_cache, gate_cache);
            }

        }    // namespace

        Result<bitwuzla::Term> get_subgraph_bitwuzla_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output)
        {
            std::map<u32, bitwuzla::Term> net_cache;
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;

            return get_subgraph_bitwuzla_function_internal(subgraph_gates, subgraph_output, net_cache, gate_cache);
        }

        Result<std::vector<bitwuzla::Term>> get_subgraph_bitwuzla_functions(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*> subgraph_outputs)
        {
            std::map<u32, bitwuzla::Term> net_cache;
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;

            std::vector<bitwuzla::Term> results;

            for (const auto& net : subgraph_outputs)
            {
                const auto res = get_subgraph_bitwuzla_function_internal(subgraph_gates, net, net_cache, gate_cache);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "unable to generate subgraph functions: failed to generate function for net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ".");
                }

                results.push_back(res.get());
            }

            return OK(results);
        }

    }    // namespace bitwuzla_utils
}    // namespace hal
