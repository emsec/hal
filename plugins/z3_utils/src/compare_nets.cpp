#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "z3_utils/include/z3_utils.h"

namespace hal
{
    namespace z3_utils
    {
        Result<bool> compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const Net* net_a, const Net* net_b, bool replace_net_ids)
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

            std::vector<Gate*> gates_a = netlist_a->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });
            std::vector<Gate*> gates_b = netlist_b->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

            z3::context ctx;

            z3::expr bf_a(ctx);
            z3::expr bf_b(ctx);

            // std::unordered_set<u32> input_net_ids_a;
            // std::unordered_set<u32> input_net_ids_b;

            // if the net has no source just use itself as boolean function
            if (net_a->get_sources().size() == 0)
            {
                bf_a = ctx.bv_const(std::to_string(net_a->get_id()).c_str(), 1);
            }
            else
            {
                const auto bf_res = z3_utils::get_subgraph_z3_function(gates_a, net_a, ctx);
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(),
                                      "cannot compare net A " + net_a->get_name() + " with ID " + std::to_string(net_a->get_id()) + " with net B " + net_b->get_name() + " with ID "
                                          + std::to_string(net_b->get_id()) + ": failed to build subgraph function for net a");
                }
                bf_a = bf_res.get();
            }

            // if the net has no source just use itself as boolean function
            if (net_b->get_sources().size() == 0)
            {
                bf_b = ctx.bv_const(std::to_string(net_b->get_id()).c_str(), 1);
            }
            else
            {
                const auto bf_res = z3_utils::get_subgraph_z3_function(gates_b, net_b, ctx);
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(),
                                      "cannot compare net A " + net_a->get_name() + " with ID " + std::to_string(net_a->get_id()) + " with net B " + net_b->get_name() + " with ID "
                                          + std::to_string(net_b->get_id()) + ": failed to build subgraph function for net b");
                }
                bf_b = bf_res.get();
            }

            z3::solver s(ctx);

            if (replace_net_ids)
            {
                const auto input_net_ids_a = z3_utils::extract_net_ids(bf_a);
                const auto input_net_ids_b = z3_utils::extract_net_ids(bf_b);

                // replace nets form netlist_a
                for (const u32 net_id : input_net_ids_a)
                {
                    const Net* net = netlist_a->get_net_by_id(net_id);
                    if (net->get_sources().size() > 1)
                    {
                        return ERR("cannot compare net A " + net_a->get_name() + " with ID " + std::to_string(net_a->get_id()) + " with net B " + net_b->get_name() + " with ID "
                                   + std::to_string(net_b->get_id()) + ": Cannot replace net id for net " + net->get_name() + " with ID " + std::to_string(net->get_id())
                                   + " because it is multi driven!");
                    }
                    if (net->get_sources().empty())
                    {
                        log_warning("z3_utils", "No source found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const Endpoint* src = net->get_sources().front();

                    if (src->get_gate() == nullptr)
                    {
                        log_warning("z3_utils", "No source gate found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const std::string src_gate_name = src->get_gate()->get_name();
                    const std::string src_pin       = src->get_pin()->get_name();
                    const std::string name          = src_gate_name + "_" + src_pin;
                    const z3::expr new_expr         = ctx.bv_const(name.c_str(), 1);
                    const z3::expr net_expr         = ctx.bv_const(BooleanFunctionNetDecorator(*net).get_boolean_variable_name().c_str(), 1);

                    s.add(net_expr == new_expr);
                }

                // replace nets form netlist_b
                for (const u32 net_id : input_net_ids_b)
                {
                    const Net* net = netlist_b->get_net_by_id(net_id);
                    if (net->get_sources().size() > 1)
                    {
                        return ERR("cannot compare net A " + net_a->get_name() + " with ID " + std::to_string(net_a->get_id()) + " with net B " + net_b->get_name() + " with ID "
                                   + std::to_string(net_b->get_id()) + ": Cannot replace net id for net " + net->get_name() + " with ID " + std::to_string(net->get_id())
                                   + " because it is multi driven!");
                    }
                    if (net->get_sources().empty())
                    {
                        log_warning("z3_utils", "No source found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const Endpoint* src = net->get_sources().front();

                    if (src->get_gate() == nullptr)
                    {
                        log_warning("z3_utils", "No source gate found for net {}. Cannot replace net and will continue with id!", net_id);
                        continue;
                    }

                    const std::string src_gate_name = src->get_gate()->get_name();
                    const std::string src_pin       = src->get_pin()->get_name();
                    const std::string name          = src_gate_name + "_" + src_pin;
                    const z3::expr new_expr         = ctx.bv_const(name.c_str(), 1);
                    const z3::expr net_expr         = ctx.bv_const(BooleanFunctionNetDecorator(*net).get_boolean_variable_name().c_str(), 1);

                    s.add(net_expr == new_expr);
                }
            }

            s.add(bf_a != bf_b);
            const z3::check_result c = s.check();

            if (c == z3::unsat)
            {
                return OK(true);
            }

            std::cout << "A: " << bf_a << std::endl;
            std::cout << "B: " << bf_b << std::endl;
            std::cout << "Model: " << s.get_model() << std::endl;

            return OK(false);
        }

        Result<bool> compare_netlists(const Netlist* netlist_a, const Netlist* netlist_b)
        {
            std::vector<Gate*> seq_gates_a = netlist_a->get_gates([](const Gate* g) {
                auto props = g->get_type()->get_properties();
                return props.find(GateTypeProperty::sequential) != props.end();
            });
            std::vector<Gate*> seq_gates_b = netlist_b->get_gates([](const Gate* g) {
                auto props = g->get_type()->get_properties();
                return props.find(GateTypeProperty::sequential) != props.end();
            });

            if (seq_gates_a.size() != seq_gates_b.size())
            {
                log_debug("iphone_tools",
                          "netlist a with ID {} and netlist b with ID {} are not equal: unequal amount of sequential gates! {} vs. {}",
                          netlist_a->get_id(),
                          netlist_b->get_id(),
                          seq_gates_a.size(),
                          seq_gates_b.size());
                return OK(false);
            }

            // TODO let the user provide a name mapping from gate a to net b incase there was a renaming of the sequential gates
            std::unordered_map<std::string, Gate*> gate_name_to_gate_a;
            std::unordered_map<std::string, Gate*> gate_name_to_gate_b;

            for (const auto& gate_a : seq_gates_a)
            {
                gate_name_to_gate_a[gate_a->get_name()] = gate_a;
            }
            for (const auto& gate_b : seq_gates_b)
            {
                gate_name_to_gate_b[gate_b->get_name()] = gate_b;
            }

            for (const auto& [gate_a_name, gate_a] : gate_name_to_gate_a)
            {
                if (const auto gate_b_it = gate_name_to_gate_b.find(gate_a_name); gate_b_it == gate_name_to_gate_b.end())
                {
                    log_debug("iphone_tools",
                              "netlist a with ID {} and netlist b with ID {} are not equal: gate a {} with ID {} is included in netlist a but does not have a counter part in netlist b!",
                              netlist_a->get_id(),
                              netlist_b->get_id(),
                              gate_a->get_name(),
                              gate_a->get_id());
                    return OK(false);
                }
            }

            log_info("iphone_tools", "Checking {} sequential gates for equality.", seq_gates_a.size());

            for (const Gate* gate_a : seq_gates_a)
            {
                const Gate* gate_b = gate_name_to_gate_b.at(gate_a->get_name());

                if (gate_a->get_type() != gate_b->get_type())
                {
                    log_debug("iphone_tools",
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
                    const Net* net_a = gate_a->get_fan_in_net(pin);
                    const Net* net_b = gate_b->get_fan_in_net(pin);

                    if (net_a == nullptr && net_b == nullptr)
                    {
                        continue;
                    }

                    const auto eq_res = z3_utils::compare_nets(netlist_a, netlist_b, net_a, net_b, true);
                    if (eq_res.is_error())
                    {
                        return ERR_APPEND(eq_res.get_error(),
                                          "cannot compare netlist a with ID " + std::to_string(netlist_a->get_id()) + " netlist b with ID " + std::to_string(netlist_b->get_id())
                                              + ": failed to compare nets at gate a " + gate_a->get_name() + " with ID " + std::to_string(gate_a->get_id()) + " and gate b" + gate_b->get_name()
                                              + "with ID " + std::to_string(gate_b->get_id()));
                    }
                    const bool eq = eq_res.get();

                    if (!eq)
                    {
                        log_debug("iphone_tools",
                                  "netlist a with ID {} and netlist b with ID {} are not equal: gate a {} with ID {} and gate b {} with ID {} are not equal at pin {}",
                                  netlist_a->get_id(),
                                  netlist_b->get_id(),
                                  gate_a->get_name(),
                                  gate_a->get_id(),
                                  gate_b->get_name(),
                                  gate_b->get_id(),
                                  pin->get_name());
                        return OK(false);
                    }
                }
            }

            return OK(true);
        }
    }    // namespace z3_utils
}    // namespace hal