#include "hawkeye/candidate_search.h"

#include "graph_algorithm/algorithms/neighborhood.h"
#include "graph_algorithm/netlist_graph.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    namespace hawkeye
    {
        static std::set<PinType> control_types = {PinType::enable, PinType::clock, PinType::set, PinType::reset};

        Result<std::vector<Candidate>> detect_candidates(Netlist* nl, const std::vector<DetectionConfiguration>& configs, u32 min_state_size, const std::vector<Gate*>& start_ffs)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            const auto start = std::chrono::system_clock::now();

            const auto nl_dec = NetlistTraversalDecorator(*nl);

            std::map<Gate*, std::set<Gate*>> ff_map;
            std::unordered_map<const Net*, std::set<Gate*>> cache = {};
            const auto start_gates                                = nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
            for (auto* sg : start_gates)
            {
                if (const auto res = nl_dec.get_next_matching_gates(
                        sg,
                        true,
                        [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); },
                        false,
                        nullptr,
                        [](const Endpoint* ep, u32 _) { return control_types.find(ep->get_pin()->get_type()) == control_types.end(); },
                        &cache);
                    res.is_ok())
                {
                    ff_map[sg] = res.get();
                }
                else
                {
                    return ERR(res.get_error());
                }
            }

            std::set<Candidate> candidates;

            for (const auto& config : configs)
            {
                auto res = graph_algorithm::NetlistGraph::from_netlist_no_edges(nl);
                if (res.is_error())
                {
                    return ERR(res.get_error());
                }

                auto nl_graph = res.get();

                if (config.control == DetectionConfiguration::Control::CHECK_FF)
                {
                    if (const auto edge_res = nl_graph->add_edges(ff_map); edge_res.is_error())
                    {
                        return ERR(edge_res.get_error());
                    }
                }
                else if (config.control == DetectionConfiguration::Control::CHECK_TYPE)
                {
                    std::map<Gate*, std::set<Gate*>> tmp_map;
                    for (const auto& [src, dsts] : ff_map)
                    {
                        for (auto* dst : dsts)
                        {
                            if (src->get_type() != dst->get_type())
                            {
                                continue;
                            }

                            tmp_map[src].insert(dst);
                        }
                    }

                    if (const auto edge_res = nl_graph->add_edges(tmp_map); edge_res.is_error())
                    {
                        return ERR(edge_res.get_error());
                    }
                }
                else
                {
                    std::map<Gate*, std::set<Gate*>> tmp_map;

                    if (config.control == DetectionConfiguration::Control::CHECK_NETS)
                    {
                        std::unordered_map<const Gate*, std::map<PinType, const Net*>> control_map;
                        for (const auto* gate : start_gates)
                        {
                            for (const auto& ep : gate->get_fan_in_endpoints())
                            {
                                if (auto pin_type = ep->get_pin()->get_type(); control_types.find(pin_type) != control_types.end())
                                {
                                    control_map[gate][pin_type] = ep->get_net();
                                }
                            }
                        }

                        for (const auto& [src, dsts] : ff_map)
                        {
                            for (auto* dst : dsts)
                            {
                                if (src->get_type() != dst->get_type())
                                {
                                    continue;
                                }

                                if (control_map.at(src) != control_map.at(dst))
                                {
                                    continue;
                                }

                                tmp_map[src].insert(dst);
                            }
                        }
                    }
                    else if (config.control == DetectionConfiguration::Control::CHECK_PINS)
                    {
                        std::unordered_map<const Gate*, std::set<PinType>> control_map;
                        for (const auto* gate : start_gates)
                        {
                            for (const auto& ep : gate->get_fan_in_endpoints())
                            {
                                auto sources = ep->get_net()->get_sources();
                                if (sources.size() != 1)
                                {
                                    continue;
                                }
                                if (sources.at(0)->get_gate()->is_gnd_gate() || sources.at(0)->get_gate()->is_vcc_gate())
                                {
                                    continue;
                                }

                                if (auto pin_type = ep->get_pin()->get_type(); control_types.find(pin_type) != control_types.end())
                                {
                                    control_map[gate].insert(pin_type);
                                }
                            }
                        }

                        for (const auto& [src, dsts] : ff_map)
                        {
                            for (auto* dst : dsts)
                            {
                                if (src->get_type() != dst->get_type())
                                {
                                    continue;
                                }

                                if (control_map.at(src) != control_map.at(dst))
                                {
                                    continue;
                                }

                                tmp_map[src].insert(dst);
                            }
                        }
                    }

                    if (const auto edge_res = nl_graph->add_edges(tmp_map); edge_res.is_error())
                    {
                        return ERR(edge_res.get_error());
                    }
                }

                const auto vertices_res = nl_graph->get_vertices_from_gates(start_gates);
                if (vertices_res.is_error())
                {
                    return ERR(vertices_res.get_error());
                }
                auto for_vertices = vertices_res.get();

                std::unordered_map<u32, std::vector<u32>> prev_hoods;
                for (u32 i = 1; i <= config.timeout; i++)
                {
                    const auto neighborhood_res = graph_algorithm::get_neighborhood(nl_graph.get(), for_vertices, i, graph_algorithm::NetlistGraph::Direction::OUT, 0);
                    if (neighborhood_res.is_error())
                    {
                        return ERR(neighborhood_res.get_error());
                    }
                    const auto neighborhoods = neighborhood_res.get();

                    std::vector<u32> new_vertices;
                    for (u32 i = 0; i < for_vertices.size(); i++)
                    {
                        u32 v            = for_vertices.at(i);
                        const auto& hood = neighborhoods.at(i);
                        auto& prev_hood  = prev_hoods[v];

                        if (prev_hood.size() < hood.size())
                        {
                            new_vertices.push_back(v);
                            prev_hood = hood;
                        }
                        else
                        {
                            if (prev_hood.size() == hood.size() && hood.size() >= config.min_register_size)
                            {
                                auto gates_res = nl_graph->get_gates_from_vertices(hood).map<std::set<Gate*>>(
                                    [](const auto& gates) -> Result<std::set<Gate*>> { return OK(std::set<Gate*>(gates.begin(), gates.end())); });
                                if (gates_res.is_error())
                                {
                                    return ERR(gates_res.get_error());
                                }

                                if (prev_hood == hood)
                                {
                                    candidates.insert(Candidate(gates_res.get()));
                                }
                                else
                                {
                                    auto prev_gates_res = nl_graph->get_gates_from_vertices(prev_hood).map<std::set<Gate*>>(
                                        [](const auto& gates) -> Result<std::set<Gate*>> { return OK(std::set<Gate*>(gates.begin(), gates.end())); });
                                    if (prev_gates_res.is_error())
                                    {
                                        return ERR(prev_gates_res.get_error());
                                    }
                                    candidates.insert(Candidate(prev_gates_res.get(), gates_res.get()));
                                }
                            }
                        }
                    }

                    if (new_vertices.empty())
                    {
                        break;
                    }

                    for_vertices = std::move(new_vertices);
                }

                // TODO implement SCC method
            }

            std::set<const Candidate*> candidates_to_delete;
            for (auto outer_it = candidates.begin(); outer_it != candidates.end(); outer_it++)
            {
                for (auto inner_it = std::next(outer_it, 1); inner_it != candidates.end(); inner_it++)
                {
                    if (std::includes(outer_it->get_output_reg().begin(), outer_it->get_output_reg().end(), inner_it->get_output_reg().begin(), inner_it->get_output_reg().end()))
                    {
                        candidates_to_delete.insert(&(*outer_it));
                        break;
                    }
                }

                if (outer_it->get_size() < min_state_size)
                {
                    candidates_to_delete.insert(&(*outer_it));
                }
            }

            for (const auto* c : candidates_to_delete)
            {
                candidates.erase(*c);
            }

            // TODO remove debug prints
            const auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            std::cout << duration_in_seconds << std::endl;

            return OK(std::vector<Candidate>(candidates.begin(), candidates.end()));
        }
    }    // namespace hawkeye
}    // namespace hal