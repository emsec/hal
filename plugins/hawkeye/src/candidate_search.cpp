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
        namespace
        {
            static std::set<PinType> control_types = {PinType::enable, PinType::clock, PinType::set, PinType::reset};

            bool continue_through_exit_ep(const Endpoint* exit_ep, const u32 current_depth)
            {
                if (exit_ep == nullptr)
                {
                    return false;
                }

                if (exit_ep->get_gate()->get_type()->has_property(GateTypeProperty::ff))
                {
                    if (current_depth != 0)
                    {
                        return false;
                    }
                    else if (control_types.find(exit_ep->get_pin()->get_type()) != control_types.end())
                    {
                        return false;
                    }
                }

                return true;
            }

            bool continue_through_entry_ep(const Endpoint* entry_ep, const u32 current_depth)
            {
                if (entry_ep == nullptr)
                {
                    return false;
                }

                if (control_types.find(entry_ep->get_pin()->get_type()) != control_types.end())
                {
                    return false;
                }

                const auto* gt = entry_ep->get_gate()->get_type();
                if (gt->has_property(GateTypeProperty::ram))
                {
                    return false;
                }

                return true;
            }

            struct GraphCandidate
            {
                u32 size;
                std::set<u32> in_reg;
                std::set<u32> out_reg;

                bool operator==(const GraphCandidate& rhs) const
                {
                    return this->size == rhs.size && this->in_reg == rhs.in_reg && this->out_reg == rhs.out_reg;
                }

                bool operator<(const GraphCandidate& rhs) const
                {
                    return this->size > rhs.size || (this->size == rhs.size && this->in_reg > rhs.in_reg) || (this->size == rhs.size && this->in_reg == rhs.in_reg && this->out_reg > rhs.out_reg);
                }
            };

            igraph_error_t get_saturating_neighborhoods(const igraph_t* graph, igraph_vector_int_t* in_set, igraph_vector_int_t* out_set, igraph_integer_t node, igraph_integer_t timeout)
            {
                igraph_integer_t no_of_nodes = igraph_vcount(graph);
                igraph_integer_t i, j, k;
                igraph_bool_t* added;
                igraph_vector_int_t current_hood, previous_hood;
                igraph_vector_int_t* current_hood_p  = &current_hood;
                igraph_vector_int_t* previous_hood_p = &previous_hood;
                igraph_vector_int_t tmp;

                if (timeout < 0)
                {
                    IGRAPH_ERROR("Negative timeout", IGRAPH_EINVAL);
                }

                added = IGRAPH_CALLOC(no_of_nodes, igraph_bool_t);
                IGRAPH_CHECK_OOM(added, "Cannot calculate neighborhood size.");
                IGRAPH_FINALLY(igraph_free, added);

                IGRAPH_VECTOR_INT_INIT_FINALLY(current_hood_p, 0);
                IGRAPH_VECTOR_INT_INIT_FINALLY(previous_hood_p, 0);
                IGRAPH_VECTOR_INT_INIT_FINALLY(&tmp, 0);

                IGRAPH_CHECK(igraph_vector_int_init(in_set, 0));
                IGRAPH_CHECK(igraph_vector_int_init(out_set, 0));
                igraph_vector_int_clear(in_set);
                igraph_vector_int_clear(out_set);

                IGRAPH_CHECK(igraph_vector_int_push_back(current_hood_p, node));

                igraph_integer_t previous_size, current_size;

                for (i = 0; i < timeout; i++)
                {
                    previous_size = igraph_vector_int_size(previous_hood_p);
                    current_size  = igraph_vector_int_size(current_hood_p);

                    if (previous_size < current_size)
                    {
                        IGRAPH_CHECK(igraph_vector_int_swap(previous_hood_p, current_hood_p));
                        igraph_vector_int_clear(current_hood_p);

                        memset(added, false, no_of_nodes * sizeof(igraph_bool_t));

                        for (j = 0; j < current_size; j++)
                        {
                            igraph_integer_t actnode = VECTOR(*previous_hood_p)[j];
                            igraph_vector_int_clear(&tmp);
                            IGRAPH_CHECK(igraph_neighbors(graph, &tmp, actnode, IGRAPH_OUT));

                            for (k = 0; k < igraph_vector_int_size(&tmp); k++)
                            {
                                igraph_integer_t nei = VECTOR(tmp)[k];
                                if (!added[nei])
                                {
                                    added[nei] = true;
                                    IGRAPH_CHECK(igraph_vector_int_push_back(current_hood_p, nei));
                                }
                            }
                        }
                    }
                    else
                    {
                        if (previous_size == current_size)
                        {
                            IGRAPH_CHECK(igraph_vector_int_update(in_set, previous_hood_p));
                            IGRAPH_CHECK(igraph_vector_int_update(out_set, current_hood_p));
                        }

                        break;
                    }
                }

                igraph_vector_int_destroy(current_hood_p);
                igraph_vector_int_destroy(previous_hood_p);
                igraph_vector_int_destroy(&tmp);
                IGRAPH_FREE(added);
                IGRAPH_FINALLY_CLEAN(4);

                return IGRAPH_SUCCESS;
            }

            igraph_error_t get_saturating_neighborhoods_scc(const igraph_t* graph,
                                                            igraph_vector_int_t* in_set,
                                                            igraph_vector_int_t* out_set,
                                                            igraph_integer_t node,
                                                            igraph_integer_t timeout,
                                                            std::map<std::set<u32>, igraph_vector_int_t*>& cache)
            {
                igraph_integer_t no_of_nodes = igraph_vcount(graph);
                igraph_integer_t i, j, k;
                igraph_bool_t* added;
                igraph_vector_int_t current_hood, previous_hood;
                igraph_vector_int_t current_component, previous_component;
                igraph_vector_int_t* current_hood_p       = &current_hood;
                igraph_vector_int_t* previous_hood_p      = &previous_hood;
                igraph_vector_int_t* current_component_p  = &current_component;
                igraph_vector_int_t* previous_component_p = &previous_component;
                igraph_vector_int_t tmp;

                if (timeout < 0)
                {
                    IGRAPH_ERROR("Negative timeout", IGRAPH_EINVAL);
                }

                added = IGRAPH_CALLOC(no_of_nodes, igraph_bool_t);
                IGRAPH_CHECK_OOM(added, "Cannot calculate neighborhood size.");
                IGRAPH_FINALLY(igraph_free, added);

                IGRAPH_VECTOR_INT_INIT_FINALLY(current_hood_p, 0);
                IGRAPH_VECTOR_INT_INIT_FINALLY(previous_hood_p, 0);
                IGRAPH_VECTOR_INT_INIT_FINALLY(current_component_p, 0);
                IGRAPH_VECTOR_INT_INIT_FINALLY(previous_component_p, 0);
                IGRAPH_VECTOR_INT_INIT_FINALLY(&tmp, 0);

                IGRAPH_CHECK(igraph_vector_int_init(in_set, 0));
                IGRAPH_CHECK(igraph_vector_int_init(out_set, 0));
                igraph_vector_int_clear(in_set);
                igraph_vector_int_clear(out_set);

                IGRAPH_CHECK(igraph_vector_int_push_back(current_hood_p, node));
                IGRAPH_CHECK(igraph_vector_int_push_back(current_component_p, node));

                igraph_integer_t previous_size, current_size;

                for (i = 0; i < timeout; i++)
                {
                    previous_size         = igraph_vector_int_size(previous_component_p);
                    current_size          = igraph_vector_int_size(current_component_p);
                    u32 current_hood_size = igraph_vector_int_size(current_hood_p);

                    if (previous_size < current_size || current_size == 1)
                    {
                        // move current objects to previous
                        IGRAPH_CHECK(igraph_vector_int_swap(previous_hood_p, current_hood_p));
                        IGRAPH_CHECK(igraph_vector_int_swap(previous_component_p, current_component_p));
                        igraph_vector_int_clear(current_hood_p);
                        igraph_vector_int_clear(current_component_p);

                        // clear flags of added vertices
                        memset(added, false, no_of_nodes * sizeof(igraph_bool_t));

                        std::set<u32> cache_key;
                        for (j = 0; j < current_hood_size; j++)
                        {
                            igraph_integer_t actnode = VECTOR(*previous_hood_p)[j];
                            igraph_vector_int_clear(&tmp);
                            IGRAPH_CHECK(igraph_neighbors(graph, &tmp, actnode, IGRAPH_OUT));

                            for (k = 0; k < igraph_vector_int_size(&tmp); k++)
                            {
                                igraph_integer_t nei = VECTOR(tmp)[k];
                                if (!added[nei])
                                {
                                    added[nei] = true;
                                    IGRAPH_CHECK(igraph_vector_int_push_back(current_hood_p, nei));
                                    cache_key.insert(nei);
                                }
                            }
                        }

                        if (k == 0)
                        {
                            continue;
                        }

                        if (const auto cache_it = cache.find(cache_key); cache_it != cache.end())
                        {
                            IGRAPH_CHECK(igraph_vector_int_update(current_component_p, cache_it->second));
                        }
                        else
                        {
                            igraph_t subgraph;
                            igraph_vs_t subgraph_vertices = igraph_vss_vector(current_hood_p);
                            IGRAPH_FINALLY(igraph_vs_destroy, &subgraph_vertices);
                            igraph_vector_int_t vertex_map;
                            IGRAPH_VECTOR_INT_INIT_FINALLY(&vertex_map, igraph_vector_int_size(current_hood_p));
                            IGRAPH_CHECK(igraph_induced_subgraph_map(graph, &subgraph, subgraph_vertices, IGRAPH_SUBGRAPH_CREATE_FROM_SCRATCH, nullptr, &vertex_map));
                            IGRAPH_FINALLY(igraph_destroy, &subgraph);

                            igraph_vector_int_t membership, csize;
                            IGRAPH_VECTOR_INT_INIT_FINALLY(&membership, 0);
                            IGRAPH_VECTOR_INT_INIT_FINALLY(&csize, 0);
                            IGRAPH_CHECK(igraph_connected_components(&subgraph, &membership, &csize, nullptr, IGRAPH_STRONG));

                            u32 max_id                = igraph_vector_int_which_max(&csize);
                            u32 num_subgraph_vertices = igraph_vcount(&subgraph);
                            for (i32 i = 0; i < num_subgraph_vertices; i++)
                            {
                                u32 cid = VECTOR(membership)[i];
                                if (cid == max_id)
                                {
                                    IGRAPH_CHECK(igraph_vector_int_push_back(current_component_p, VECTOR(vertex_map)[i]));
                                }
                            }

                            igraph_vs_destroy(&subgraph_vertices);
                            igraph_vector_int_destroy(&vertex_map);
                            igraph_vector_int_destroy(&membership);
                            igraph_vector_int_destroy(&csize);
                            igraph_destroy(&subgraph);
                            IGRAPH_FINALLY_CLEAN(5);

                            igraph_vector_int_t* cache_tmp = new igraph_vector_int_t;
                            IGRAPH_CHECK(igraph_vector_int_init(cache_tmp, 0));    // cleanup handled by caller
                            IGRAPH_CHECK(igraph_vector_int_update(cache_tmp, current_component_p));
                            cache[cache_key] = cache_tmp;
                        }
                    }
                    else
                    {
                        if (previous_size == current_size)
                        {
                            IGRAPH_CHECK(igraph_vector_int_update(in_set, previous_component_p));
                            IGRAPH_CHECK(igraph_vector_int_update(out_set, current_component_p));
                        }

                        break;
                    }
                }

                igraph_vector_int_destroy(current_hood_p);
                igraph_vector_int_destroy(previous_hood_p);
                igraph_vector_int_destroy(current_component_p);
                igraph_vector_int_destroy(previous_component_p);
                igraph_vector_int_destroy(&tmp);
                IGRAPH_FREE(added);
                IGRAPH_FINALLY_CLEAN(6);

                return IGRAPH_SUCCESS;
            }
        }    // namespace

        Result<std::vector<RegisterCandidate>> detect_candidates(Netlist* nl, const std::vector<DetectionConfiguration>& configs, u32 min_state_size, const std::vector<Gate*>& start_ffs)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            log_info("hawkeye", "start traversing netlist to determine flip-flop dependencies...");
            auto global_start = std::chrono::system_clock::now();
            auto start        = global_start;

            const auto nl_dec = NetlistTraversalDecorator(*nl);
            std::map<Gate*, std::set<Gate*>> ff_map;
            std::unordered_map<const Net*, std::set<Gate*>> cache = {};
            const auto start_gates                                = nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
            for (auto* sg : start_gates)
            {
                if (const auto res = nl_dec.get_next_matching_gates(
                        sg, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, continue_through_exit_ep, continue_through_entry_ep);
                    res.is_ok())
                {
                    ff_map[sg] = res.get();
                }
                else
                {
                    return ERR(res.get_error());
                }
            }

            auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "successfully traversed netlist in {} seconds", duration_in_seconds);

            log_info("hawkeye", "start constructing empty netlist graph...");
            start = std::chrono::system_clock::now();

            auto res = graph_algorithm::NetlistGraph::from_netlist_no_edges(nl, start_gates);
            if (res.is_error())
            {
                return ERR(res.get_error());
            }
            auto base_graph = res.get();

            const auto start_vertices_res = base_graph->get_vertices_from_gates(start_ffs.empty() ? start_gates : start_ffs);
            if (start_vertices_res.is_error())
            {
                return ERR(start_vertices_res.get_error());
            }
            auto start_vertices = start_vertices_res.get();

            duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "successfully created empty netlist graph in {} seconds", duration_in_seconds);

            log_info("hawkeye", "start candidate identification using {} configurations...", configs.size());
            start = std::chrono::system_clock::now();

            std::set<RegisterCandidate> candidates;
            for (const auto& config : configs)
            {
                log_info("hawkeye",
                         "start filling netlist graph with configuration [timeout={}, min_register_size={}, control={}, components={}]...",
                         config.timeout,
                         config.min_register_size,
                         enum_to_string(config.control),
                         enum_to_string(config.components));
                auto start_inner = std::chrono::system_clock::now();

                auto tmp_graph_res = base_graph->copy();
                if (tmp_graph_res.is_error())
                {
                    return ERR(tmp_graph_res.get_error());
                }
                auto tmp_graph = tmp_graph_res.get();

                std::map<Gate*, std::set<Gate*>> filtered_map;
                if (config.control == DetectionConfiguration::Control::CHECK_FF)
                {
                    filtered_map = std::move(ff_map);
                    if (const auto edge_res = tmp_graph->add_edges(ff_map); edge_res.is_error())
                    {
                        return ERR(edge_res.get_error());
                    }
                }
                else if (config.control == DetectionConfiguration::Control::CHECK_TYPE)
                {
                    for (const auto& [src, dsts] : ff_map)
                    {
                        for (auto* dst : dsts)
                        {
                            if (src->get_type() != dst->get_type())
                            {
                                continue;
                            }

                            filtered_map[src].insert(dst);
                        }
                    }
                }
                else if (config.control == DetectionConfiguration::Control::CHECK_NETS)
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
                            if (control_map.at(src) != control_map.at(dst))
                            {
                                continue;
                            }

                            filtered_map[src].insert(dst);
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

                            filtered_map[src].insert(dst);
                        }
                    }
                }

                if (const auto edge_res = tmp_graph->add_edges(filtered_map); edge_res.is_error())
                {
                    return ERR(edge_res.get_error());
                }

                duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start_inner).count();
                log_info("hawkeye", "successfully filled netlist graph in {} seconds", duration_in_seconds);

                log_info("hawkeye",
                         "start neighborhood discovery with configuration [timeout={}, min_register_size={}, control={}, components={}]...",
                         config.timeout,
                         config.min_register_size,
                         enum_to_string(config.control),
                         enum_to_string(config.components));
                start_inner = std::chrono::system_clock::now();

                igraph_vector_int_t in_set, out_set;
                if (const auto res = igraph_vector_int_init(&in_set, 0); res != IGRAPH_SUCCESS)
                {
                    return ERR(igraph_strerror(res));
                }

                if (const auto res = igraph_vector_int_init(&out_set, 0); res != IGRAPH_SUCCESS)
                {
                    igraph_vector_int_destroy(&in_set);
                    return ERR(igraph_strerror(res));
                }

                std::set<GraphCandidate> graph_candidates;

                if (config.components == DetectionConfiguration::Components::NONE)
                {
                    for (const auto v : start_vertices)
                    {
                        igraph_vector_int_clear(&in_set);
                        igraph_vector_int_clear(&out_set);

                        if (const auto res = get_saturating_neighborhoods(tmp_graph->get_graph(), &in_set, &out_set, v, config.timeout); res != IGRAPH_SUCCESS)
                        {
                            igraph_vector_int_destroy(&in_set);
                            igraph_vector_int_destroy(&out_set);
                            return ERR(igraph_strerror(res));
                        }

                        u32 size = igraph_vector_int_size(&out_set);
                        if (size < config.min_register_size)
                        {
                            continue;
                        }

                        GraphCandidate c;
                        c.size = size;
                        for (u32 i = 0; i < igraph_vector_int_size(&in_set); i++)
                        {
                            c.in_reg.insert(VECTOR(in_set)[i]);
                        }
                        for (u32 i = 0; i < igraph_vector_int_size(&out_set); i++)
                        {
                            c.out_reg.insert(VECTOR(out_set)[i]);
                        }
                        graph_candidates.insert(c);
                    }
                }
                else if (config.components == DetectionConfiguration::Components::CHECK_SCC)
                {
                    std::map<std::set<u32>, igraph_vector_int_t*> scc_cache;

                    for (const auto v : start_vertices)
                    {
                        igraph_vector_int_clear(&in_set);
                        igraph_vector_int_clear(&out_set);

                        if (const auto res = get_saturating_neighborhoods_scc(tmp_graph->get_graph(), &in_set, &out_set, v, config.timeout, scc_cache); res != IGRAPH_SUCCESS)
                        {
                            igraph_vector_int_destroy(&in_set);
                            igraph_vector_int_destroy(&out_set);
                            for (auto& [_, comp] : scc_cache)
                            {
                                igraph_vector_int_destroy(comp);
                                delete comp;
                            }
                            return ERR(igraph_strerror(res));
                        }

                        u32 size = igraph_vector_int_size(&out_set);
                        if (size < config.min_register_size)
                        {
                            continue;
                        }

                        GraphCandidate c;
                        c.size = size;
                        for (u32 i = 0; i < igraph_vector_int_size(&in_set); i++)
                        {
                            c.in_reg.insert(VECTOR(in_set)[i]);
                        }
                        for (u32 i = 0; i < igraph_vector_int_size(&out_set); i++)
                        {
                            c.out_reg.insert(VECTOR(out_set)[i]);
                        }
                        graph_candidates.insert(c);
                    }

                    for (auto& [_, comp] : scc_cache)
                    {
                        igraph_vector_int_destroy(comp);
                        delete comp;
                    }
                }

                igraph_vector_int_destroy(&in_set);
                igraph_vector_int_destroy(&out_set);

                for (const auto& gc : graph_candidates)
                {
                    std::set<Gate*> out_reg;

                    if (auto out_reg_res = tmp_graph->get_gates_set_from_vertices(gc.out_reg); out_reg_res.is_ok())
                    {
                        out_reg = out_reg_res.get();
                    }
                    else
                    {
                        return ERR(out_reg_res.get_error());
                    }

                    if (gc.in_reg == gc.out_reg)
                    {
                        candidates.insert(RegisterCandidate(out_reg));
                    }
                    else
                    {
                        std::set<Gate*> in_reg;
                        if (auto in_reg_res = tmp_graph->get_gates_set_from_vertices(gc.in_reg); in_reg_res.is_ok())
                        {
                            in_reg = in_reg_res.get();
                            candidates.insert(RegisterCandidate(in_reg, out_reg));
                        }
                        else
                        {
                            return ERR(in_reg_res.get_error());
                        }
                    }
                }

                duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start_inner).count();
                log_info("hawkeye", "successfully completed neighborhood discovery in {} seconds", duration_in_seconds);
            }

            duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "successfully completed candidate identification in {} seconds", duration_in_seconds);

            log_info("hawkeye", "start reducing candidates...", configs.size());
            start = std::chrono::system_clock::now();

            std::set<const RegisterCandidate*> candidates_to_delete;
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

            duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "successfully completed reducing candidates in {} seconds", duration_in_seconds);

            duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - global_start).count();
            log_info("hawkeye", "overall runtime: {} seconds", duration_in_seconds);

            return OK(std::vector<RegisterCandidate>(candidates.begin(), candidates.end()));
        }
    }    // namespace hawkeye

    template<>
    std::map<hawkeye::DetectionConfiguration::Control, std::string> EnumStrings<hawkeye::DetectionConfiguration::Control>::data = {
        {hawkeye::DetectionConfiguration::Control::CHECK_FF, "CHECK_FF"},
        {hawkeye::DetectionConfiguration::Control::CHECK_TYPE, "CHECK_TYPE"},
        {hawkeye::DetectionConfiguration::Control::CHECK_PINS, "CHECK_PINS"},
        {hawkeye::DetectionConfiguration::Control::CHECK_NETS, "CHECK_NETS"}};

    template<>
    std::map<hawkeye::DetectionConfiguration::Components, std::string> EnumStrings<hawkeye::DetectionConfiguration::Components>::data = {
        {hawkeye::DetectionConfiguration::Components::NONE, "NONE"},
        {hawkeye::DetectionConfiguration::Components::CHECK_SCC, "CHECK_SCC"}};
}    // namespace hal