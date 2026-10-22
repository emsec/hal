#include "hal_core/netlist/netlist_utils.h"

#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/log.h"

#include <deque>
#include <unordered_set>

namespace hal
{
    namespace netlist_utils
    {
        namespace
        {
            class ShortestPathInternal
            {
                Gate* m_start_gate;
                Gate* m_end_gate;
                const std::unordered_set<Gate*>& m_end_gates;
                bool m_forward;
                std::unordered_map<Gate*, Gate*> m_origin_map;
                bool (ShortestPathInternal::*m_heureka) (Gate*) const;
                std::unordered_set<Gate*> (ShortestPathInternal::*m_get_next_gates) (Gate*) const;

                bool heureka_single(Gate* test_gate) const
                {
                    return (m_end_gate == test_gate);
                }

                bool heureka_multiple(Gate* test_gate) const
                {
                    return (m_end_gates.find(test_gate) != m_end_gates.end());
                }

                std::unordered_set<Gate*> get_next_gates_forward(Gate* origin) const
                {
                    std::unordered_set<Gate*> retval;
                    for (const Net* n : origin->get_fan_out_nets())
                    {
                        for (const Endpoint* ep : n->get_destinations())
                        {
                            retval.insert(ep->get_gate());
                        }
                    }
                    return retval;
                }

                std::unordered_set<Gate*> get_next_gates_backward(Gate* origin) const
                {
                    std::unordered_set<Gate*> retval;
                    for (const Net* n : origin->get_fan_in_nets())
                    {
                        for (const Endpoint* ep : n->get_sources())
                        {
                            retval.insert(ep->get_gate());
                        }
                    }
                    return retval;
                }

            public:
                ShortestPathInternal(Gate* start_gate, const std::unordered_set<Gate*>& end_gates, bool forward=true)
                    : m_start_gate(start_gate), m_end_gate(nullptr), m_end_gates(end_gates), m_forward(forward)
                {
                    if (m_end_gates.size() == 1)
                    {
                        m_end_gate = *(m_end_gates.begin());
                        m_heureka = &ShortestPathInternal::heureka_single;
                    }
                    else
                        m_heureka = &ShortestPathInternal::heureka_multiple;

                    if (forward)
                        m_get_next_gates = &ShortestPathInternal::get_next_gates_forward;
                    else
                        m_get_next_gates = &ShortestPathInternal::get_next_gates_backward;
                }

                std::vector<Gate*> get_path() const
                {
                    if (m_end_gates.empty())
                    {
                        log_warning("netlist_utils", "Cannot find shortest path from start gate ID={}, no target given.", m_start_gate->get_id());
                        return std::vector<Gate*>();
                    }
                    if ((this->*m_heureka)(m_start_gate))
                    {
                        log_warning("netlist_utils", "Cannot find shortest path since start gate ID={} and target are the same.", m_start_gate->get_id());
                        return std::vector<Gate*>();
                    }

                    std::vector<Gate*> v0;
                    std::vector<Gate*> v1;
                    v0.push_back(m_start_gate);
                    std::unordered_map<Gate*, Gate*> originMap;

                    while (!v0.empty())
                    {
                        for (Gate* g0 : v0)
                        {
                            for (Gate* g1 : (this->*m_get_next_gates)(g0))
                            {
                                if (originMap.find(g1) != originMap.end())
                                continue;    // already routed to
                                v1.push_back(g1);
                                originMap[g1] = g0;

                                if ((this->*m_heureka)(g1))
                                {
                                    // heureka! Path found
                                    std::vector<Gate*> retval;
                                    Gate* g = g1;
                                    while (g != m_start_gate)
                                    {
                                        retval.push_back(g);
                                        auto it = originMap.find(g);
                                        assert(it != originMap.end());
                                        g = it->second;
                                    }
                                    retval.push_back(m_start_gate);
                                    if (m_forward)
                                        std::reverse(retval.begin(),retval.end());
                                    return retval;
                                }
                            }
                        }
                        v0 = v1;
                        v1.clear();
                    }

                    log_warning("netlist_utils", "It looks like there is no path from start gate ID={} to selected {} target(s).", m_start_gate->get_id(), m_end_gates.size());
                    return std::vector<Gate*>();
                }
            };

        }    // namespace

        Result<BooleanFunction> get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
        {
            if (net == nullptr)
            {
                return ERR("could not get subgraph function: net is a 'nullptr'");
            }

            if (auto res = SubgraphNetlistDecorator(*net->get_netlist()).get_subgraph_function(subgraph_gates, net, cache); res.is_ok())
            {
                return res;
            }
            else
            {
                return ERR(res.get_error());
            }
        }

        Result<BooleanFunction> get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates)
        {
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> cache;
            if (auto res = get_subgraph_function(net, subgraph_gates, cache); res.is_error())
            {
                return ERR(res.get_error());
            }
            else
            {
                return res;
            }
        }

        std::unique_ptr<Netlist> copy_netlist(const Netlist* nl)
        {
            if (auto res = nl->copy(); res.is_error())
            {
                log_error("netlist_utils", "error encountered while copying netlist:\n{}", res.get_error().get());
                return nullptr;
            }
            else
            {
                return res.get();
            }
        }

        std::pair<std::map<u32, Gate*>, std::vector<std::vector<int>>> get_ff_dependency_matrix(const Netlist* nl)
        {
            std::map<u32, Gate*> matrix_id_to_gate;
            std::map<Gate*, u32> gate_to_matrix_id;
            std::vector<std::vector<int>> matrix;

            u32 matrix_gates = 0;
            for (const auto& gate : nl->get_gates())
            {
                if (!gate->get_type()->has_property(GateTypeProperty::ff))
                {
                    continue;
                }
                gate_to_matrix_id[gate]         = matrix_gates;
                matrix_id_to_gate[matrix_gates] = gate;
                matrix_gates++;
            }

            for (const auto& [id, gate] : matrix_id_to_gate)
            {
                std::vector<int> line_of_matrix;

                std::set<u32> gates_to_add;
                for (const auto& pred_gate : netlist_utils::get_next_sequential_gates(gate, false))
                {
                    gates_to_add.insert(gate_to_matrix_id[pred_gate]);
                }

                for (u32 i = 0; i < matrix_gates; i++)
                {
                    if (gates_to_add.find(i) != gates_to_add.end())
                    {
                        line_of_matrix.push_back(1);
                    }
                    else
                    {
                        line_of_matrix.push_back(0);
                    }
                }
                matrix.push_back(line_of_matrix);
            }

            return std::make_pair(matrix_id_to_gate, matrix);
        }

        std::unique_ptr<Netlist> get_partial_netlist(const Netlist* nl, const std::vector<const Gate*>& subgraph_gates)
        {
            if (auto res = SubgraphNetlistDecorator(*nl).copy_subgraph_netlist(subgraph_gates); res.is_ok())
            {
                return res.get();
            }
            else
            {
                log_error("netlist_utils", "error encountered while copying subgraph netlist:\n{}", res.get_error().get());
                return nullptr;
            }
        }

        std::vector<Gate*> get_next_gates(const Gate* gate, bool get_successors, int depth, const std::function<bool(const Gate*)>& filter)
        {
            std::vector<Gate*> retval;
            std::unordered_map<u32, std::vector<Gate*>> cache;
            std::vector<const Gate*> v0;
            v0.push_back(gate);
            std::unordered_set<const Gate*> gats_handled;
            std::unordered_set<const Net*> nets_handled;
            gats_handled.insert(gate);

            for (int round = 0; !depth || round < depth; round++)
            {
                std::vector<const Gate*> v1;
                for (const Gate* g0 : v0)
                {
                    for (const Net* n : get_successors ? g0->get_fan_out_nets() : g0->get_fan_in_nets())
                    {
                        if (nets_handled.find(n) != nets_handled.end())
                        {
                            continue;
                        }
                        nets_handled.insert(n);

                        for (const Endpoint* ep : get_successors ? n->get_destinations() : n->get_sources())
                        {
                            Gate* g1 = ep->get_gate();
                            if (gats_handled.find(g1) != gats_handled.end())
                            {
                                continue;    // already handled
                            }
                            gats_handled.insert(g1);
                            if (!filter || filter(g1))
                            {
                                v1.push_back(g1);
                                retval.push_back(g1);
                            }
                        }
                    }
                }
                if (v1.empty())
                {
                    break;
                }
                v0 = v1;
            }
            return retval;
        }

        std::vector<Gate*> get_next_gates(const Net* net, bool get_successors, int depth, const std::function<bool(const Gate*)>& filter)
        {
            std::vector<Gate*> retval;
            std::unordered_map<u32, std::vector<Gate*>> cache;
            std::vector<const Gate*> v0;
            std::unordered_set<const Gate*> gates_handled;
            std::unordered_set<const Net*> nets_handled;
            for (const Endpoint* ep : (get_successors ? net->get_destinations() : net->get_sources()))
            {
                Gate* g = ep->get_gate();
                if (!filter || filter(g))
                {
                    v0.push_back(g);
                    gates_handled.insert(g);
                    nets_handled.insert(net);
                    retval.push_back(g);
                }
            }

            for (int round = 1; depth == 0 || round < depth; round++)
            {
                std::vector<const Gate*> v1;
                for (const Gate* g0 : v0)
                {
                    for (const Net* n : get_successors ? g0->get_fan_out_nets() : g0->get_fan_in_nets())
                    {
                        if (nets_handled.find(n) != nets_handled.end())
                        {
                            continue;
                        }
                        nets_handled.insert(n);

                        for (const Endpoint* ep : get_successors ? n->get_destinations() : n->get_sources())
                        {
                            Gate* g1 = ep->get_gate();
                            if (gates_handled.find(g1) != gates_handled.end())
                            {
                                continue;    // already handled
                            }
                            gates_handled.insert(g1);
                            if (!filter || filter(g1))
                            {
                                v1.push_back(g1);
                                retval.push_back(g1);
                            }
                        }
                    }
                }
                if (v1.empty())
                {
                    break;
                }
                v0 = v1;
            }
            return retval;
        }

        std::vector<Gate*> get_shortest_path(Gate* start_gate, Module* end_module, bool forward_direction)
        {
            std::unordered_set<Gate*> end_gates;
            for (Gate* g : end_module->get_gates(nullptr, true))
            {
                end_gates.insert(g);
            }
            ShortestPathInternal spi(start_gate, end_gates, forward_direction);
            return spi.get_path();
        }

        std::vector<std::vector<Gate*> > get_shortest_path(Module* start_module, Module* end_module)
        {
            std::unordered_set<Gate*> start_gates;
            for (Gate* g : start_module->get_gates(nullptr, true))
            {
                start_gates.insert(g);
            }

            int shortest_length = -1;
            std::vector<std::vector<Gate*> > retval;
            for (Gate* end_gate : end_module->get_gates(nullptr,true))
            {
                ShortestPathInternal spi_reverse(end_gate, start_gates, false);
                std::vector<Gate*> path_reverse = spi_reverse.get_path();
                int len = path_reverse.size();
                if (!len) continue;
                if (shortest_length < 0) shortest_length = len;
                if (len > shortest_length) continue;
                if (len < shortest_length) shortest_length = len;
                retval.push_back(path_reverse);
            }

            auto it = retval.begin();
            while (it != retval.end())
            {
                if (it->size() > shortest_length)
                    it = retval.erase(it);
                else
                    ++it;
            }
            return retval;
        }

        std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions)
        {
            std::unordered_set<Gate*> end_gates_forward;
            end_gates_forward.insert(end_gate);
            ShortestPathInternal spi_forward(start_gate, end_gates_forward, true);
            std::vector<Gate*> path_forward = spi_forward.get_path();
            if (!search_both_directions)
                return path_forward;
            std::unordered_set<Gate*> start_gates_reverse;
            start_gates_reverse.insert(start_gate);
            ShortestPathInternal spi_reverse(end_gate, start_gates_reverse, false);
            std::vector<Gate*> path_reverse = spi_reverse.get_path();
            return (path_reverse.size() < path_forward.size()) ? path_reverse : path_forward;
        }

        namespace
        {
            std::vector<Gate*> get_next_sequential_gates_internal(const Net* start_net, bool forward, std::unordered_set<u32>& seen, std::unordered_map<u32, std::vector<Gate*>>& cache)
            {
                if (auto it = cache.find(start_net->get_id()); it != cache.end())
                {
                    return it->second;
                }

                if (seen.find(start_net->get_id()) != seen.end())
                {
                    return {};
                }

                seen.insert(start_net->get_id());

                std::vector<Gate*> found_ffs;

                for (auto endpoint : forward ? start_net->get_destinations() : start_net->get_sources())
                {
                    auto next_gate = endpoint->get_gate();

                    if (next_gate->get_type()->has_property(GateTypeProperty::ff))
                    {
                        found_ffs.push_back(next_gate);
                    }
                    else
                    {
                        for (auto n : forward ? next_gate->get_fan_out_nets() : next_gate->get_fan_in_nets())
                        {
                            auto next_gates = get_next_sequential_gates_internal(n, forward, seen, cache);
                            found_ffs.insert(found_ffs.end(), next_gates.begin(), next_gates.end());
                        }
                    }
                }

                std::sort(found_ffs.begin(), found_ffs.end());
                found_ffs.erase(std::unique(found_ffs.begin(), found_ffs.end()), found_ffs.end());

                cache.emplace(start_net->get_id(), found_ffs);
                return found_ffs;
            }
        }    // namespace

        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::vector<Gate*> found_ffs;
            for (const auto& n : get_successors ? gate->get_fan_out_nets() : gate->get_fan_in_nets())
            {
                auto suc = get_next_sequential_gates(n, get_successors, cache);
                found_ffs.insert(found_ffs.end(), suc.begin(), suc.end());
            }

            std::sort(found_ffs.begin(), found_ffs.end());
            found_ffs.erase(std::unique(found_ffs.begin(), found_ffs.end()), found_ffs.end());

            return found_ffs;
        }

        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::unordered_set<u32> seen;
            return get_next_sequential_gates_internal(net, get_successors, seen, cache);
        }

        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_next_sequential_gates(gate, get_successors, cache);
        }

        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_next_sequential_gates(net, get_successors, cache);
        }

        namespace
        {
            std::vector<Gate*>
                get_path_internal(const Net* start_net, bool forward, std::set<GateTypeProperty> stop_types, std::unordered_set<u32>& seen, std::unordered_map<u32, std::vector<Gate*>>& cache)
            {
                if (auto it = cache.find(start_net->get_id()); it != cache.end())
                {
                    return it->second;
                }

                if (seen.find(start_net->get_id()) != seen.end())
                {
                    return {};
                }

                seen.insert(start_net->get_id());

                std::vector<Gate*> found_combinational;

                for (auto endpoint : forward ? start_net->get_destinations() : start_net->get_sources())
                {
                    auto next_gate = endpoint->get_gate();

                    bool stop = false;
                    for (GateTypeProperty property : next_gate->get_type()->get_properties())
                    {
                        if (stop_types.find(property) != stop_types.end())
                        {
                            stop = true;
                        }
                    }

                    if (stop == false)
                    {
                        found_combinational.push_back(next_gate);

                        for (auto n : forward ? next_gate->get_fan_out_nets() : next_gate->get_fan_in_nets())
                        {
                            auto next_gates = get_path_internal(n, forward, stop_types, seen, cache);
                            found_combinational.insert(found_combinational.end(), next_gates.begin(), next_gates.end());
                        }
                    }
                }

                std::sort(found_combinational.begin(), found_combinational.end());
                found_combinational.erase(std::unique(found_combinational.begin(), found_combinational.end()), found_combinational.end());

                cache.emplace(start_net->get_id(), found_combinational);
                return found_combinational;
            }
        }    // namespace

        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::vector<Gate*> found_combinational;
            for (const auto& n : get_successors ? gate->get_fan_out_nets() : gate->get_fan_in_nets())
            {
                auto suc = get_path(n, get_successors, stop_properties, cache);
                found_combinational.insert(found_combinational.end(), suc.begin(), suc.end());
            }

            std::sort(found_combinational.begin(), found_combinational.end());
            found_combinational.erase(std::unique(found_combinational.begin(), found_combinational.end()), found_combinational.end());

            return found_combinational;
        }

        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::unordered_set<u32> seen;
            return get_path_internal(net, get_successors, stop_properties, seen, cache);
        }

        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_path(gate, get_successors, stop_properties, cache);
        }

        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_path(net, get_successors, stop_properties, cache);
        }

        std::vector<Net*> get_nets_at_pins(Gate* gate, std::vector<GatePin*> pins)
        {
            std::vector<Net*> nets;

            for (const auto& pin : pins)
            {
                if (pin == nullptr)
                {
                    log_warning("netlist_utils", "'nullptr' given as pin.");
                    continue;
                }

                PinDirection direction = pin->get_direction();
                if (direction == PinDirection::input || direction == PinDirection::inout)
                {
                    if (auto net = gate->get_fan_in_net(pin); net != nullptr)
                    {
                        nets.push_back(net);
                    }
                    else
                    {
                        log_warning("netlist_utils", "could not retrieve fan-in net for pin '{}' of gate '{}' with ID {}.", pin->get_name(), gate->get_name(), gate->get_id());
                    }
                }
                else if (direction == PinDirection::output)
                {
                    if (auto net = gate->get_fan_out_net(pin); net != nullptr)
                    {
                        nets.push_back(net);
                    }
                    else
                    {
                        log_warning("netlist_utils", "could not retrieve fan-out net for pin '{}' of gate '{}' with ID {}.", pin->get_name(), gate->get_name(), gate->get_id());
                    }
                }
            }

            return nets;
        }

        Result<u32> remove_buffers(Netlist* netlist, bool analyze_inputs)
        {
            u32 num_gates = 0;

            for (const auto& gate : netlist->get_gates())
            {
                std::vector<Endpoint*> fan_out = gate->get_fan_out_endpoints();

                GateType* gt = gate->get_type();
                if (!gt->has_property(GateTypeProperty::combinational) || gt->has_property(GateTypeProperty::power) || gt->has_property(GateTypeProperty::ground))
                {
                    // continue if of invalid base type
                    continue;
                }

                if (fan_out.size() != 1)
                {
                    // continue if more than one fan-out net
                    continue;
                }

                std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();
                if (functions.size() != 1)
                {
                    // continue if more than one Boolean function (tri-state?)
                    continue;
                }

                Endpoint* out_endpoint = *(fan_out.begin());
                if (out_endpoint->get_pin()->get_name() != (functions.begin())->first)
                {
                    // continue if Boolean function name does not match output pin
                    continue;
                }

                std::vector<Endpoint*> fan_in = gate->get_fan_in_endpoints();
                BooleanFunction func          = functions.begin()->second;

                if (analyze_inputs)
                {
                    for (Endpoint* ep : fan_in)
                    {
                        auto sources = ep->get_net()->get_sources();
                        if (sources.size() != 1)
                        {
                            break;
                        }

                        if (sources.front()->get_gate()->is_gnd_gate())
                        {
                            if (auto substitution = func.substitute(ep->get_pin()->get_name(), BooleanFunction::Const(0, 1)); substitution.is_ok())
                            {
                                func = substitution.get();
                            }
                        }
                        else if (sources.front()->get_gate()->is_vcc_gate())
                        {
                            if (auto substitution = func.substitute(ep->get_pin()->get_name(), BooleanFunction::Const(1, 1)); substitution.is_ok())
                            {
                                func = substitution.get();
                            }
                        }
                    }

                    func = func.simplify();
                }

                std::string func_str             = func.to_string();
                std::vector<std::string> in_pins = gt->get_input_pin_names();
                if (std::find(in_pins.begin(), in_pins.end(), func_str) != in_pins.end())
                {
                    Net* out_net = out_endpoint->get_net();

                    // check all input endpoints and ...
                    for (Endpoint* in_endpoint : fan_in)
                    {
                        Net* in_net = in_endpoint->get_net();

                        if (in_endpoint->get_pin()->get_name() == func_str)
                        {
                            // reconnect outputs if the input is passed through the buffer
                            for (Endpoint* dst : out_net->get_destinations())
                            {
                                Gate* dst_gate   = dst->get_gate();
                                GatePin* dst_pin = dst->get_pin();
                                if (!out_net->remove_destination(dst))
                                {
                                    return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from output net '"
                                               + out_net->get_name() + "' with ID " + std::to_string(out_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                               + std::to_string(gate->get_id()));
                                }
                                if (!in_net->add_destination(dst_gate, dst_pin))
                                {
                                    return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to add destination to input net '"
                                               + in_net->get_name() + "' with ID " + std::to_string(in_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                               + std::to_string(gate->get_id()));
                                }
                            }
                        }
                        else
                        {
                            // remove the input endpoint otherwise
                            if (!in_net->remove_destination(gate, in_endpoint->get_pin()))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from input net '"
                                           + in_net->get_name() + "' with ID " + std::to_string(in_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                           + std::to_string(gate->get_id()));
                            }
                        }
                    }

                    // delete output net and buffer gate
                    netlist->delete_net(out_net);
                    netlist->delete_gate(gate);
                    num_gates++;
                }
                else if (func_str == "0" || func_str == "1")
                {
                    Net* out_net = out_endpoint->get_net();

                    const std::vector<Gate*>& gnd_gates = netlist->get_gnd_gates();
                    const std::vector<Gate*>& vcc_gates = netlist->get_vcc_gates();
                    if (gnd_gates.empty() || vcc_gates.empty())
                    {
                        continue;
                    }
                    Net* gnd_net = gnd_gates.front()->get_fan_out_nets().front();
                    Net* vcc_net = vcc_gates.front()->get_fan_out_nets().front();

                    for (Endpoint* in_endpoint : fan_in)
                    {
                        Net* in_net = in_endpoint->get_net();

                        // remove the input endpoint otherwise
                        if (!in_net->remove_destination(gate, in_endpoint->get_pin()))
                        {
                            return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from input net '"
                                       + in_net->get_name() + "' with ID " + std::to_string(in_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()));
                        }
                    }
                    if (func_str == "0")
                    {
                        for (Endpoint* dst : out_net->get_destinations())
                        {
                            Gate* dst_gate   = dst->get_gate();
                            GatePin* dst_pin = dst->get_pin();
                            if (!out_net->remove_destination(dst))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from output net '"
                                           + out_net->get_name() + "' with ID " + std::to_string(out_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                           + std::to_string(gate->get_id()));
                            }
                            if (!gnd_net->add_destination(dst_gate, dst_pin))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to add destination to GND net '"
                                           + gnd_net->get_name() + "' with ID " + std::to_string(gnd_net->get_id()));
                            }
                        }
                    }
                    else if (func_str == "1")
                    {
                        for (Endpoint* dst : out_net->get_destinations())
                        {
                            Gate* dst_gate   = dst->get_gate();
                            GatePin* dst_pin = dst->get_pin();
                            if (!out_net->remove_destination(dst))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from output net '"
                                           + out_net->get_name() + "' with ID " + std::to_string(out_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                           + std::to_string(gate->get_id()));
                            }
                            if (!vcc_net->add_destination(dst_gate, dst_pin))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to add destination to VCC net '"
                                           + gnd_net->get_name() + "' with ID " + std::to_string(gnd_net->get_id()));
                            }
                        }
                    }

                    // delete output net and buffer gate
                    netlist->delete_net(out_net);
                    netlist->delete_gate(gate);
                    num_gates++;
                }
            }

            return OK(num_gates);
        }

        Result<u32> remove_unused_lut_endpoints(Netlist* netlist)
        {
            u32 num_eps = 0;

            // net connected to GND
            const std::vector<Gate*>& gnd_gates = netlist->get_gnd_gates();
            if (gnd_gates.empty())
            {
                return ERR("could not completely remove unused LUT endpoints from netlist with ID " + std::to_string(netlist->get_id()) + ": no GND net available within netlist");
            }
            Net* gnd_net = gnd_gates.front()->get_fan_out_nets().front();

            // iterate all LUT gates
            for (const auto& gate : netlist->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
            {
                std::vector<Endpoint*> fan_in                              = gate->get_fan_in_endpoints();
                std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();

                // skip if more than one function
                if (functions.size() != 1)
                {
                    continue;
                }

                auto active_pins = functions.begin()->second.get_variable_names();

                // if there are more fan-in nets than there are active pins, we need to get rid of some nets
                if (fan_in.size() > active_pins.size())
                {
                    for (const auto& ep : fan_in)
                    {
                        if (std::find(active_pins.begin(), active_pins.end(), ep->get_pin()->get_name()) == active_pins.end())
                        {
                            num_eps++;
                            GatePin* pin = ep->get_pin();
                            if (!ep->get_net()->remove_destination(gate, pin))
                            {
                                return ERR("could not completely remove unused LUT endpoints from netlist with ID " + std::to_string(netlist->get_id())
                                           + ": failed to remove inactive endpoint from gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()));
                            }
                            if (!gnd_net->add_destination(gate, pin))
                            {
                                return ERR("could not completely remove unused LUT endpoints from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to connect inactive input of gate '"
                                           + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + " to GND net");
                            }
                        }
                    }
                }
            }

            return OK(num_eps);
        }

        std::vector<Net*> get_common_inputs(const std::vector<Gate*>& gates, u32 threshold)
        {
            // if threshold = 0, a net is only considered to be common if it is an input to all gates
            if (threshold == 0)
            {
                threshold = gates.size();
            }

            // count input net occurences
            std::map<Net*, u32> net_count;
            for (Gate* g : gates)
            {
                for (Endpoint* pred : g->get_predecessors())
                {
                    if (pred->get_gate()->is_gnd_gate() || pred->get_gate()->is_vcc_gate())
                    {
                        continue;
                    }

                    Net* pred_net = pred->get_net();
                    if (const auto it = net_count.find(pred_net); it != net_count.end())
                    {
                        it->second++;
                    }
                    else
                    {
                        net_count[pred_net] = 1;
                    }
                }
            }

            // consider every net that is input to at least half the gates to be a common input
            std::vector<Net*> common_inputs;
            for (const auto& [n, cnt] : net_count)
            {
                if (cnt >= threshold)
                {
                    common_inputs.push_back(n);
                }
            }

            return common_inputs;
        }

        Result<std::monostate> replace_gate(Gate* gate, GateType* target_type, std::map<GatePin*, GatePin*> pin_map)
        {
            if (auto res = NetlistModificationDecorator(*(gate->get_netlist())).replace_gate(gate, target_type, pin_map); res.is_ok())
            {
                return OK({});
            }
            else
            {
                return ERR(res.get_error());
            }
        }


    }    // namespace netlist_utils
}    // namespace hal
