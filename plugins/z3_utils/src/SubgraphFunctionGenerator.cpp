#include "hal_core/utilities/log.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

#include "SubgraphFunctionGenerator.h"

#include "z3++.h"

#include <queue>

namespace hal
{
    namespace z3_utils
    {
        BooleanFunction SubgraphFunctionGenerator::get_function_of_gate(const Gate* gate, const std::string& out_pin)
        {
            if (auto it = m_cache.find({gate->get_id(), out_pin}); it != m_cache.end())
            {
                return it->second;
            }
            BooleanFunction bf = gate->get_boolean_function(out_pin);

            if (bf.is_empty())
            {
                log_error("z3_utils", "function of gate {} (type {}) associated with pin {} is empty!", gate->get_name(), gate->get_type()->get_name(), out_pin);
                return BooleanFunction();
            }

            // before replacing input pins with their connected net id, check if the function depends on other output pins
            auto output_pins = gate->get_output_pins();
            while (true)
            {
                auto vars = bf.get_variables();
                std::vector<std::string> output_pins_that_are_also_function_inputs;
                std::set_intersection(vars.begin(), vars.end(), output_pins.begin(), output_pins.end(), std::back_inserter(output_pins_that_are_also_function_inputs));

                if (output_pins_that_are_also_function_inputs.empty())
                {
                    break;
                }

                for (auto const& output_pin : output_pins_that_are_also_function_inputs)
                {
                    bf = bf.substitute(output_pin, gate->get_boolean_function(output_pin));
                }
            }

            // replace input pins with their connected net id
            for (auto const& input_pin : gate->get_input_pins())
            {
                const auto& input_net = gate->get_fan_in_net(input_pin);
                if (!input_net)
                {
                    log_info("z3_utils", "Pin ({}) has no input net. Gate id: ({})", input_pin, gate->get_id());
                    continue;
                }
                bf = bf.substitute(input_pin, std::to_string(input_net->get_id()));
            }

            m_cache.emplace(std::make_tuple(gate->get_id(), out_pin), bf);

            return bf;
        }

        void SubgraphFunctionGenerator::get_subgraph_z3_function(const Net* output_net,
                                      const std::vector<Gate*> subgraph_gates,
                                      z3::context& ctx,
                                      z3::expr& result,
                                      std::unordered_set<u32>& input_net_ids)
        {
            /* check validity of subgraph_gates */
            if (subgraph_gates.empty())
            {
                log_error("z3_utils", "parameter 'subgraph_gates' is empty");
            }
            if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](auto& g) { return g == nullptr; }))
            {
                log_error("z3_utils", "parameter 'subgraph_gates' contains a nullptr");
            }
            if (output_net->get_num_of_sources() != 1)
            {
                log_error("z3_utils", "target net has 0 or more than 1 sources.");
            }

            std::queue<Net*> q;

            input_net_ids.clear();

            // prepare queue and result with initial gate
            {
                auto source     = output_net->get_sources()[0];
                auto start_gate = source->get_gate();

                // Check wether start gate is in the subgraph gates
                if (std::find(subgraph_gates.begin(), subgraph_gates.end(), start_gate) == subgraph_gates.end()) {
                    result = ctx.bv_const(std::to_string(output_net->get_id()).c_str(), 1);
                    return;
                }
                
                auto f = get_function_of_gate(start_gate, source->get_pin());

                for (auto id_string : f.get_variables())
                {
                    input_net_ids.insert(std::stoi(id_string));
                }

                result = f.to_z3(ctx);

                for (auto& n : start_gate->get_fan_in_nets())
                {
                    q.push(n);
                }
            }

            while (!q.empty())
            {
                auto n = q.front();
                q.pop();

                if (n->get_num_of_sources() != 1)
                {
                    //log_error("z3_utils", "net has 0 or more than 1 sources. not expanding the function here");
                    continue;
                }

                if (input_net_ids.find(n->get_id()) == input_net_ids.end())
                {
                    // log_info("z3_utils", "net {} (id {}) is not an input to the function", n->get_name(), n->get_id());
                    continue;
                }

                auto src      = n->get_sources()[0];
                auto src_gate = src->get_gate();

                if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src_gate) != subgraph_gates.end())
                {
                    auto f = get_function_of_gate(src_gate, src->get_pin());

                    // substitute function into z3 expression
                    z3::expr_vector from_vec(ctx);
                    z3::expr_vector to_vec(ctx);

                    z3::expr from = ctx.bv_const(std::to_string(n->get_id()).c_str(), 1);
                    z3::expr to   = f.to_z3(ctx);

                    from_vec.push_back(from);
                    to_vec.push_back(to);

                    result = result.substitute(from_vec, to_vec);

                    // replace the net id which was substituted with all input ids of the substituted function
                    input_net_ids.erase(n->get_id());
                    for (auto id_string : f.get_variables())
                    {
                        input_net_ids.insert(std::stoi(id_string));
                    }

                    // add input nets of gate to queue
                    for (auto& sn : src_gate->get_fan_in_nets())
                    {
                        q.push(sn);
                    }
                }
            }
        }

        RecursiveSubgraphFunctionGenerator::RecursiveSubgraphFunctionGenerator(z3::context& ctx, const std::vector<Gate*>& subgraph_gates) : m_ctx(&ctx), m_subgraph_gates(subgraph_gates) {

        };

        BooleanFunction RecursiveSubgraphFunctionGenerator::get_function_of_gate(const Gate* gate, const std::string& out_pin)
        {
            if (auto it = m_cache.find({gate->get_id(), out_pin}); it != m_cache.end())
            {
                return it->second;
            }
            BooleanFunction bf = gate->get_boolean_function(out_pin);

            if (bf.is_empty())
            {
                log_error("z3_utils", "function of gate {} (type {}) associated with pin {} is empty!", gate->get_name(), gate->get_type()->get_name(), out_pin);
                return BooleanFunction();
            }

            // TODO should this also take internal pins into account?
            // before replacing input pins with their connected net id, check if the function depends on other output pins
            auto output_pins = gate->get_output_pins();
            while (true)
            {
                auto vars = bf.get_variables();
                std::vector<std::string> output_pins_that_are_also_function_inputs;
                std::set_intersection(vars.begin(), vars.end(), output_pins.begin(), output_pins.end(), std::back_inserter(output_pins_that_are_also_function_inputs));

                if (output_pins_that_are_also_function_inputs.empty())
                {
                    break;
                }

                for (auto const& output_pin : output_pins_that_are_also_function_inputs)
                {
                    bf = bf.substitute(output_pin, gate->get_boolean_function(output_pin));
                }
            }

            m_cache.emplace(std::make_tuple(gate->get_id(), out_pin), bf);

            return bf;
        }

        z3::expr RecursiveSubgraphFunctionGenerator::get_function_of_net(const Net* net, z3::context& ctx, const std::vector<Gate*>& subgraph_gates) {
            
            if (m_expr_cache.find(net) != m_expr_cache.end()) {
                return m_expr_cache.at(net);
            }

            const std::vector<Endpoint*> sources = net->get_sources();

            // net is multi driven
            if (sources.size() > 1) {
                log_error("z3_utils", "Cannot handle multi driven nets! Encountered at net {}.", net->get_id());
                return ctx.bv_const("ERROR", 1);
            }

            // net has no source
            if (sources.empty()) {
                z3::expr ret = ctx.bv_const(std::to_string(net->get_id()).c_str(), 1);
                m_expr_cache.insert({net, ret});
                return ret;
            }

            const Endpoint* src_ep = sources.front();

            if (src_ep->get_gate() == nullptr) {
                log_error("z3_utils", "Gate at source for net {} is null.", net->get_id());
            }

            const Gate* src = src_ep->get_gate();

            // source is not in subgraph gates
            if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src) == subgraph_gates.end()) {
                z3::expr ret = ctx.bv_const(std::to_string(net->get_id()).c_str(), 1);
                m_expr_cache.insert({net, ret});
                return ret;
            }

            const BooleanFunction bf = get_function_of_gate(src, src_ep->get_pin());

            std::map<std::string, z3::expr> pin_to_expr;
            
            for (const std::string& pin : bf.get_variables()) {
                Net* in_net = src->get_fan_in_net(pin);

                if (in_net == nullptr) {
                    log_error("z3_utils", "Cannot find in_net at pin {} of gate {}!", pin, src->get_id());
                }

                pin_to_expr.insert({pin, get_function_of_net(in_net, ctx, subgraph_gates)});
            } 

            z3::expr ret = bf.to_z3(ctx, pin_to_expr);
            m_expr_cache.insert({net, ret});
            return ret;
        }

        void RecursiveSubgraphFunctionGenerator::get_subgraph_z3_function_recursive(const Net* net, z3::expr& result)
        {
            /* check validity of subgraph_gates */
            if (m_subgraph_gates.empty())
            {
                log_error("z3_utils", "parameter 'subgraph_gates' is empty");
            }
            if (std::any_of(m_subgraph_gates.begin(), m_subgraph_gates.end(), [](auto& g) { return g == nullptr; }))
            {
                log_error("z3_utils", "parameter 'subgraph_gates' contains a nullptr");
            }

            result = get_function_of_net(net, *m_ctx, m_subgraph_gates);
            return;
        }
    
    }    // namespace z3_utils
}    // namespace hal