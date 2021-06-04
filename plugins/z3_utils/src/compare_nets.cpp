#include "compare_nets.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include "SubgraphFunctionGenerator.h"
#include "z3++.h"


namespace hal
{
    namespace z3_utils
    {
       bool compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const Net* net_a, const Net* net_b, bool replace_net_ids) {
            if (netlist_a == nullptr) {
                log_error("z3_utils", "netlist_a is a nullptr!");
                return false;
            }

            if (netlist_b == nullptr) {
                log_error("z3_utils", "netlist_b is a nullptr!");
                return false;
            }

            if (net_a == nullptr) {
                log_error("z3_utils", "net_a is a nullptr!");
                return false;
            }

            if (net_b == nullptr) {
                log_error("z3_utils", "net_b is a nullptr!");
                return false;
            }

            SubgraphFunctionGenerator sfg_a;
            SubgraphFunctionGenerator sfg_b;

            std::vector<Gate*> gates_a = netlist_a->get_gates([](Gate* g) {auto props = g->get_type()->get_properties(); return props.find(GateTypeProperty::combinational) != props.end();}); 
            std::vector<Gate*> gates_b = netlist_b->get_gates([](Gate* g) {auto props = g->get_type()->get_properties(); return props.find(GateTypeProperty::combinational) != props.end();}); 

            z3::context ctx;

            z3::expr bf_a(ctx);
            z3::expr bf_b(ctx);

            std::unordered_set<u32> input_net_ids_a;
            std::unordered_set<u32> input_net_ids_b;

            // if the net has no source just use itself as boolean function
            if (net_a->get_sources().size() == 0) {
                bf_a = ctx.bv_const(std::to_string(net_a->get_id()).c_str(), 1);
            } else {
                sfg_a.get_subgraph_z3_function(net_a, gates_a, ctx, bf_a, input_net_ids_a);
            }

            // if the net has no source just use itself as boolean function
            if (net_b->get_sources().size() == 0) {
                bf_b = ctx.bv_const(std::to_string(net_b->get_id()).c_str(), 1);
            } else {
                sfg_b.get_subgraph_z3_function(net_b, gates_b, ctx, bf_b, input_net_ids_b);
            }

            z3::solver s(ctx);

            if (replace_net_ids) {
                // replace nets form netlist_a
                for (const u32 net_id : input_net_ids_a) {
                    const Net* net = netlist_a->get_net_by_id(net_id);
                    if (net->get_sources().size() > 1) {
                        log_error("z3_utils", "Cannot replace net_id for multi driven net {}!", net_id);
                        return false;
                    }
                    if (net->get_sources().empty()) {
                        log_warning("z3_utils", "No source found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const Endpoint* src = net->get_sources().front();
                    
                    if (src->get_gate() == nullptr) {
                        log_warning("z3_utils", "No source gate found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const u32 src_gate_id = src->get_gate()->get_id();
                    const std::string src_pin = src->get_pin();
                    const std::string name = std::to_string(src_gate_id) + "_" + src_pin;
                    const z3::expr new_expr = ctx.bv_const(name.c_str(), 1);
                    const z3::expr net_expr = ctx.bv_const(std::to_string(net_id).c_str(), 1);

                    s.add(net_expr == new_expr);
                }

                // replace nets form netlist_b
                for (const u32 net_id : input_net_ids_b) {
                    const Net* net = netlist_b->get_net_by_id(net_id);
                    if (net->get_sources().size() > 1) {
                        log_error("z3_utils", "Cannot replace net_id for multi driven net {}!", net_id);
                        return false;
                    }
                    if (net->get_sources().empty()) {
                        log_warning("z3_utils", "No source found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const Endpoint* src = net->get_sources().front();
                    
                    if (src->get_gate() == nullptr) {
                        log_warning("z3_utils", "No source gate found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const u32 src_gate_id = src->get_gate()->get_id();
                    const std::string src_pin = src->get_pin();
                    const std::string name = std::to_string(src_gate_id) + "_" + src_pin;
                    const z3::expr new_expr = ctx.bv_const(name.c_str(), 1);
                    const z3::expr net_expr = ctx.bv_const(std::to_string(net_id).c_str(), 1);

                    s.add(net_expr == new_expr);
                }
            }

            s.add(bf_a != bf_b);
            const z3::check_result c = s.check();
            
            if (c == z3::unsat) {
                return true;
            }

            std::cout << "A: " << bf_a << std::endl;
            std::cout << "B: " << bf_b << std::endl;
            std::cout << "Model: " << s.get_model() << std::endl;

            return false;
       }
    }    // namespace z3_utils
}    // namespace hal