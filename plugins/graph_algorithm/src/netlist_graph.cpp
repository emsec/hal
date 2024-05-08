#include "graph_algorithm/netlist_graph.h"

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    namespace graph_algorithm
    {

        NetlistGraph::NetlistGraph(Netlist* nl) : m_nl(nl)
        {
        }

        NetlistGraph::~NetlistGraph()
        {
            igraph_destroy(&m_graph);
        }

        Result<std::unique_ptr<NetlistGraph>> NetlistGraph::from_netlist(Netlist* nl, bool create_dummy_nodes, const std::function<bool(const Net*)>& filter)
        {
            if (!nl)
            {
                return ERR("netlist is a nullptr");
            }

            auto graph = std::unique_ptr<NetlistGraph>(new NetlistGraph(nl));

            // count all edges as this number is needed to create a new graph
            u32 edge_counter = 0;
            for (const auto* net : graph->m_nl->get_nets(filter))
            {
                std::vector<Gate*> src_gates;
                for (const auto* src_ep : net->get_sources())
                {
                    src_gates.push_back(src_ep->get_gate());
                }

                std::vector<Gate*> dst_gates;
                for (const auto* dst_ep : net->get_destinations())
                {
                    dst_gates.push_back(dst_ep->get_gate());
                }

                if (src_gates.empty() && create_dummy_nodes)
                {
                    // if no sources, add one dummy edge for every destination
                    // all dummy edges will come from the same dummy node
                    edge_counter += dst_gates.size();
                }
                else if (dst_gates.empty() && create_dummy_nodes)
                {
                    // if no destinations, add one dummy edge for every source
                    // all dummy edges will go to the same dummy node
                    edge_counter += src_gates.size();
                }
                else
                {
                    // add one edge for every source-destination pair
                    edge_counter += dst_gates.size() * src_gates.size();
                }
            }

            // initialize edge vector
            igraph_vector_int_t edges;
            auto err = igraph_vector_int_init(&edges, 2 * edge_counter);
            if (err != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(err));
            }

            // we need dummy gates for input/outputs
            u32 node_counter = 0;
            u32 edge_index   = 0;

            for (auto* g : graph->m_nl->get_gates())
            {
                const u32 node                = node_counter++;
                graph->m_gates_to_nodes[g]    = node;
                graph->m_nodes_to_gates[node] = g;
            }

            for (const auto* net : graph->m_nl->get_nets(filter))
            {
                std::vector<Gate*> src_gates;
                for (const auto* src_ep : net->get_sources())
                {
                    src_gates.push_back(src_ep->get_gate());
                }

                std::vector<Gate*> dst_gates;
                for (const auto* dst_ep : net->get_destinations())
                {
                    dst_gates.push_back(dst_ep->get_gate());
                }

                if (src_gates.empty() && create_dummy_nodes)
                {
                    // if no sources, add one dummy node
                    const u32 dummy_node                = node_counter++;
                    graph->m_nodes_to_gates[dummy_node] = nullptr;
                    for (auto* dst_gate : dst_gates)
                    {
                        VECTOR(edges)[edge_index++] = dummy_node;
                        VECTOR(edges)[edge_index++] = graph->m_gates_to_nodes.at(dst_gate);
                    }
                }
                else if (dst_gates.empty() && create_dummy_nodes)
                {
                    // if no destinations, add one dummy node
                    const u32 dummy_node                = node_counter++;
                    graph->m_nodes_to_gates[dummy_node] = nullptr;
                    for (auto* src_gate : src_gates)
                    {
                        VECTOR(edges)[edge_index++] = dummy_node;
                        VECTOR(edges)[edge_index++] = graph->m_gates_to_nodes.at(src_gate);
                    }
                }
                else
                {
                    for (auto* dst_gate : dst_gates)
                    {
                        for (auto* src_gate : src_gates)
                        {
                            VECTOR(edges)[edge_index++] = graph->m_gates_to_nodes.at(src_gate);
                            VECTOR(edges)[edge_index++] = graph->m_gates_to_nodes.at(dst_gate);
                        }
                    }
                }
            }

            err = igraph_create(&(graph->m_graph), &edges, node_counter, IGRAPH_DIRECTED);
            igraph_vector_int_destroy(&edges);

            if (err != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(err));
            }

            return OK(std::move(graph));
        }

        Result<std::unique_ptr<NetlistGraph>> NetlistGraph::from_netlist_no_edges(Netlist* nl)
        {
            if (!nl)
            {
                return ERR("netlist is a nullptr");
            }

            auto graph = std::unique_ptr<NetlistGraph>(new NetlistGraph(nl));

            u32 node_counter = 0;
            for (auto* g : graph->m_nl->get_gates())
            {
                const u32 node                = node_counter++;
                graph->m_gates_to_nodes[g]    = node;
                graph->m_nodes_to_gates[node] = g;
            }

            auto err = igraph_empty(&(graph->m_graph), node_counter, IGRAPH_DIRECTED);
            if (err != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(err));
            }

            return OK(std::move(graph));
        }

        Netlist* NetlistGraph::get_netlist() const
        {
            return m_nl;
        }

        igraph_t* NetlistGraph::get_graph()
        {
            return &m_graph;
        }

        Result<std::vector<Gate*>> NetlistGraph::get_gates_from_vertices(const std::vector<u32>& vertices) const
        {
            std::vector<Gate*> res;
            for (const auto& vertex : vertices)
            {
                if (const auto it = m_nodes_to_gates.find(vertex); it != m_nodes_to_gates.end())
                {
                    Gate* g = it->second;

                    res.push_back(g);
                    if (!g)
                    {
                        log_warning("graph_algorithm", "no gate exists for dummy node {}, added nullptr", vertex);
                    }
                }
                else
                {
                    return ERR("no gate for node " + std::to_string(vertex) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(res);
        }

        Result<std::vector<Gate*>> NetlistGraph::get_gates_from_vertices(const std::set<u32>& vertices) const
        {
            std::vector<Gate*> res;
            for (const auto& vertex : vertices)
            {
                if (const auto it = m_nodes_to_gates.find(vertex); it != m_nodes_to_gates.end())
                {
                    Gate* g = it->second;

                    res.push_back(g);
                    if (!g)
                    {
                        log_warning("graph_algorithm", "no gate exists for dummy node {}, added nullptr", vertex);
                    }
                }
                else
                {
                    return ERR("no gate for node " + std::to_string(vertex) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(res);
        }

        Result<std::vector<Gate*>> NetlistGraph::get_gates_from_vertices_igraph(const igraph_vector_int_t* vertices) const
        {
            std::vector<Gate*> res;
            for (u32 i = 0; i < igraph_vector_int_size(vertices); i++)
            {
                u32 vertex = VECTOR(*vertices)[i];
                if (const auto it = m_nodes_to_gates.find(vertex); it != m_nodes_to_gates.end())
                {
                    Gate* g = it->second;

                    res.push_back(g);
                    if (!g)
                    {
                        log_warning("graph_algorithm", "no gate exists for dummy node {}, added nullptr", vertex);
                    }
                }
                else
                {
                    return ERR("no gate for node " + std::to_string(vertex) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(res);
        }

        Result<Gate*> NetlistGraph::get_gate_from_vertex(const u32 vertex) const
        {
            const auto res = get_gates_from_vertices(std::vector<u32>({vertex}));
            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return OK(res.get().front());
        }

        Result<std::vector<u32>> NetlistGraph::get_vertices_from_gates(const std::vector<Gate*>& gates) const
        {
            std::vector<u32> res;
            for (u32 i = 0; i < gates.size(); i++)
            {
                auto* g = gates.at(i);

                if (!g)
                {
                    return ERR("gate at index " + std::to_string(i) + " is a nullptr");
                }

                if (const auto it = m_gates_to_nodes.find(g); it != m_gates_to_nodes.end())
                {
                    res.push_back(it->second);
                }
                else
                {
                    return ERR("no node for gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " exists in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(res);
        }

        Result<std::vector<u32>> NetlistGraph::get_vertices_from_gates(const std::set<Gate*>& gates) const
        {
            std::vector<u32> res;
            for (auto* g : gates)
            {
                if (!g)
                {
                    return ERR("set of gates contains a nullptr");
                }

                if (const auto it = m_gates_to_nodes.find(g); it != m_gates_to_nodes.end())
                {
                    res.push_back(it->second);
                }
                else
                {
                    return ERR("no node for gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " exists in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(res);
        }

        Result<igraph_vector_int_t> NetlistGraph::get_vertices_from_gates_igraph(const std::vector<Gate*>& gates) const
        {
            igraph_vector_int_t out;
            if (auto res = igraph_vector_int_init(&out, gates.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            for (u32 i = 0; i < gates.size(); i++)
            {
                auto* g = gates.at(i);

                if (!g)
                {
                    return ERR("gate at index " + std::to_string(i) + " is a nullptr");
                }

                if (const auto it = m_gates_to_nodes.find(g); it != m_gates_to_nodes.end())
                {
                    VECTOR(out)[i] = it->second;
                }
                else
                {
                    return ERR("no node for gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " exists in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(std::move(out));
        }

        Result<u32> NetlistGraph::get_vertex_from_gate(Gate* g) const
        {
            const auto res = get_vertices_from_gates(std::vector<Gate*>({g}));
            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return OK(res.get().front());
        }

        u32 NetlistGraph::get_num_vertices(bool only_connected) const
        {
            u32 num_vertices = igraph_vcount(&m_graph);

            if (!only_connected)
            {
                return num_vertices;
            }
            else
            {
                u32 num_connected_vertices = 0;

                igraph_vector_int_t degrees;
                igraph_vector_int_init(&degrees, num_vertices);

                igraph_vs_t v_sel;
                igraph_vs_all(&v_sel);

                igraph_degree(&m_graph, &degrees, v_sel, IGRAPH_ALL, IGRAPH_LOOPS);

                for (u32 i = 0; i < num_vertices; i++)
                {
                    if (VECTOR(degrees)[i] != 0)
                    {
                        num_connected_vertices++;
                    }
                }

                igraph_vector_int_destroy(&degrees);
                igraph_vs_destroy(&v_sel);

                return num_connected_vertices;
            }
        }

        u32 NetlistGraph::get_num_edges() const
        {
            return igraph_ecount(&m_graph);
        }

        Result<std::vector<std::pair<u32, u32>>> NetlistGraph::get_edges() const
        {
            const u32 ecount = igraph_ecount(&m_graph);

            igraph_vector_int_t edges;
            if (auto res = igraph_vector_int_init(&edges, 2 * ecount); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_get_edgelist(&m_graph, &edges, false); res != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&edges);
                return ERR(igraph_strerror(res));
            }

            std::vector<std::pair<u32, u32>> e_vec(ecount);
            for (u32 i = 0; i < ecount; i++)
            {
                const u32 src_vertex = (u32)VECTOR(edges)[2 * i];
                const u32 dst_vertex = (u32)VECTOR(edges)[2 * i + 1];

                e_vec[i] = std::make_pair(src_vertex, dst_vertex);
            }

            return OK(e_vec);
        }

        Result<std::vector<std::pair<Gate*, Gate*>>> NetlistGraph::get_edges_in_netlist() const
        {
            const u32 ecount = igraph_ecount(&m_graph);

            igraph_vector_int_t edges;
            if (auto res = igraph_vector_int_init(&edges, 2 * ecount); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_get_edgelist(&m_graph, &edges, false); res != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&edges);
                return ERR(igraph_strerror(res));
            }

            std::vector<std::pair<Gate*, Gate*>> e_vec(ecount);
            for (u32 i = 0; i < ecount; i++)
            {
                const u32 src_vertex = (u32)VECTOR(edges)[2 * i];
                const u32 dst_vertex = (u32)VECTOR(edges)[2 * i + 1];
                Gate *src_gate, *dst_gate;

                if (const auto it = m_nodes_to_gates.find(src_vertex); it != m_nodes_to_gates.end())
                {
                    src_gate = it->second;
                    if (src_gate == nullptr)
                    {
                        log_warning("graph_algorithm",
                                    "ignored edge (" + std::to_string(src_vertex) + "," + std::to_string(dst_vertex) + ") at dummy source vertex '" + std::to_string(src_vertex) + "'");
                        continue;
                    }
                }

                if (const auto it = m_nodes_to_gates.find(dst_vertex); it != m_nodes_to_gates.end())
                {
                    dst_gate = it->second;
                    if (dst_gate == nullptr)
                    {
                        log_warning("graph_algorithm",
                                    "ignored edge (" + std::to_string(src_vertex) + "," + std::to_string(dst_vertex) + ") at dummy destination vertex '" + std::to_string(dst_vertex) + "'");
                        continue;
                    }
                }

                e_vec[i] = std::make_pair(src_gate, dst_gate);
            }

            return OK(e_vec);
        }

        Result<std::monostate> NetlistGraph::add_edges(const std::vector<std::pair<Gate*, Gate*>>& edges)
        {
            igraph_vector_int_t e_vec;
            if (auto res = igraph_vector_int_init(&e_vec, 2 * edges.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            u32 edge_index = 0;
            for (const auto& [src_gate, dst_gate] : edges)
            {
                if (auto it = m_gates_to_nodes.find(src_gate); it != m_gates_to_nodes.end())
                {
                    VECTOR(e_vec)[edge_index++] = it->second;
                }
                else
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("no node for gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + " exists in graph for netlist with ID "
                               + std::to_string(m_nl->get_id()));
                }

                if (auto it = m_gates_to_nodes.find(dst_gate); it != m_gates_to_nodes.end())
                {
                    VECTOR(e_vec)[edge_index++] = it->second;
                }
                else
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("no node for gate '" + dst_gate->get_name() + "' with ID " + std::to_string(dst_gate->get_id()) + " exists in graph for netlist with ID "
                               + std::to_string(m_nl->get_id()));
                }
            }

            if (auto res = igraph_add_edges(&m_graph, &e_vec, nullptr); res != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&e_vec);
                return ERR(igraph_strerror(res));
            }

            return OK({});
        }

        Result<std::monostate> NetlistGraph::add_edges(const std::vector<std::pair<u32, u32>>& edges)
        {
            igraph_vector_int_t e_vec;
            if (auto err = igraph_vector_int_init(&e_vec, 2 * edges.size()); err != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(err));
            }

            u32 vcount = igraph_vcount(&m_graph);

            u32 edge_index = 0;
            for (const auto& [src_vertex, dst_vertex] : edges)
            {
                if (src_vertex >= vcount)
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("source vertex '" + std::to_string(src_vertex) + "' does not exist in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                }
                if (dst_vertex >= vcount)
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("destination vertex '" + std::to_string(dst_vertex) + "' does not exist in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                }

                VECTOR(e_vec)[edge_index++] = src_vertex;
                VECTOR(e_vec)[edge_index++] = dst_vertex;
            }

            if (auto err = igraph_add_edges(&m_graph, &e_vec, nullptr); err != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&e_vec);
                return ERR(igraph_strerror(err));
            }

            return OK({});
        }

        Result<std::monostate> NetlistGraph::delete_edges(const std::vector<std::pair<Gate*, Gate*>>& edges)
        {
            igraph_vector_int_t e_vec;
            if (auto err = igraph_vector_int_init(&e_vec, 2 * edges.size()); err != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(err));
            }

            u32 vcount = igraph_vcount(&m_graph);

            u32 edge_index = 0;
            for (const auto& [src_gate, dst_gate] : edges)
            {
                if (auto it = m_gates_to_nodes.find(src_gate); it != m_gates_to_nodes.end())
                {
                    VECTOR(e_vec)[edge_index++] = it->second;
                }
                else
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("no node for gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + " exists in graph for netlist with ID "
                               + std::to_string(m_nl->get_id()));
                }

                if (auto it = m_gates_to_nodes.find(dst_gate); it != m_gates_to_nodes.end())
                {
                    VECTOR(e_vec)[edge_index++] = it->second;
                }
                else
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("no node for gate '" + dst_gate->get_name() + "' with ID " + std::to_string(dst_gate->get_id()) + " exists in graph for netlist with ID "
                               + std::to_string(m_nl->get_id()));
                }
            }

            igraph_es_t e_sel;
            if (auto res = igraph_es_pairs(&e_sel, &e_vec, IGRAPH_DIRECTED); res != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&e_vec);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_delete_edges(&m_graph, e_sel); res != IGRAPH_SUCCESS)
            {
                igraph_es_destroy(&e_sel);
                igraph_vector_int_destroy(&e_vec);
                return ERR(igraph_strerror(res));
            }

            igraph_es_destroy(&e_sel);
            igraph_vector_int_destroy(&e_vec);

            return OK({});
        }

        Result<std::monostate> NetlistGraph::delete_edges(const std::vector<std::pair<u32, u32>>& edges)
        {
            igraph_vector_int_t e_vec;
            if (auto err = igraph_vector_int_init(&e_vec, 2 * edges.size()); err != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(err));
            }

            u32 vcount = igraph_vcount(&m_graph);

            u32 edge_index = 0;
            for (const auto& [src_vertex, dst_vertex] : edges)
            {
                if (src_vertex >= vcount)
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("source vertex '" + std::to_string(src_vertex) + "' does not exist in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                }
                if (dst_vertex >= vcount)
                {
                    igraph_vector_int_destroy(&e_vec);
                    return ERR("destination vertex '" + std::to_string(dst_vertex) + "' does not exist in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                }

                VECTOR(e_vec)[edge_index++] = src_vertex;
                VECTOR(e_vec)[edge_index++] = dst_vertex;
            }

            igraph_es_t e_sel;
            if (auto res = igraph_es_pairs(&e_sel, &e_vec, IGRAPH_DIRECTED); res != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&e_vec);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_delete_edges(&m_graph, e_sel); res != IGRAPH_SUCCESS)
            {
                igraph_es_destroy(&e_sel);
                igraph_vector_int_destroy(&e_vec);
                return ERR(igraph_strerror(res));
            }

            igraph_es_destroy(&e_sel);
            igraph_vector_int_destroy(&e_vec);

            return OK({});
        }

        void NetlistGraph::print() const
        {
            igraph_write_graph_edgelist(&m_graph, stdout);
        }
    }    // namespace graph_algorithm

    template<>
    std::map<graph_algorithm::NetlistGraph::Direction, std::string> EnumStrings<graph_algorithm::NetlistGraph::Direction>::data = {{graph_algorithm::NetlistGraph::Direction::NONE, "NONE"},
                                                                                                                                   {graph_algorithm::NetlistGraph::Direction::IN, "IN"},
                                                                                                                                   {graph_algorithm::NetlistGraph::Direction::OUT, "OUT"},
                                                                                                                                   {graph_algorithm::NetlistGraph::Direction::ALL, "ALL"}};
}    // namespace hal