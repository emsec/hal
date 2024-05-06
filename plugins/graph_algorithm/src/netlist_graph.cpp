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
            igraph_destroy(m_graph);
        }

        Result<std::unique_ptr<NetlistGraph>> NetlistGraph::from_netlist(Netlist* nl, bool create_dummy_nodes, const std::function<bool(const Net*)>& filter)
        {
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
                igraph_vector_int_destroy(&edges);
                return ERR(igraph_strerror(err));
            }

            // we need dummy gates for input/outputs
            u32 node_counter = 0;
            u32 edge_index   = 0;

            for (auto* g : graph->m_nl->get_gates())
            {
                u32 node                      = node_counter++;
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
                    u32 dummy_node                      = ++node_counter;
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
                    u32 dummy_node                      = ++node_counter;
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

            err = igraph_create(graph->m_graph, &edges, node_counter, IGRAPH_DIRECTED);
            igraph_vector_int_destroy(&edges);

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

        igraph_t* NetlistGraph::get_graph() const
        {
            return m_graph;
        }

        Result<Gate*> NetlistGraph::get_gate_of_vertex(const u32 node) const
        {
            if (!m_nl)
            {
                return ERR("graph does not correspond to a netlist");
            }

            if (const auto it = m_nodes_to_gates.find(node); it != m_nodes_to_gates.end())
            {
                Gate* g = it->second;
                if (g != nullptr)
                {
                    return OK(g);
                }
                else
                {
                    log_warning("graph_algorithm", "no gate exists for dummy node {}", node);
                }
            }
            else
            {
                return ERR("no gate for node " + std::to_string(node) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
            }
        }

        Result<std::vector<Gate*>> NetlistGraph::get_gates_of_vertices(const std::set<u32>& nodes) const
        {
            if (!m_nl)
            {
                return ERR("graph does not correspond to a netlist");
            }

            std::vector<Gate*> res;
            for (const auto& node : nodes)
            {
                if (const auto it = m_nodes_to_gates.find(node); it != m_nodes_to_gates.end())
                {
                    Gate* g = it->second;
                    if (g != nullptr)
                    {
                        res.push_back(g);
                    }
                    else
                    {
                        log_warning("graph_algorithm", "no gate exists for dummy node {}", node);
                    }
                }
                else
                {
                    return ERR("no gate for node " + std::to_string(node) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(res);
        }

        Result<std::vector<Gate*>> NetlistGraph::get_gates_of_vertices(const std::vector<u32>& nodes) const
        {
            if (!m_nl)
            {
                return ERR("graph does not correspond to a netlist");
            }

            std::vector<Gate*> res;
            for (const auto& node : nodes)
            {
                if (const auto it = m_nodes_to_gates.find(node); it != m_nodes_to_gates.end())
                {
                    Gate* g = it->second;
                    if (g != nullptr)
                    {
                        res.push_back(g);
                    }
                    else
                    {
                        log_warning("graph_algorithm", "no gate exists for dummy node {}", node);
                    }
                }
                else
                {
                    return ERR("no gate for node " + std::to_string(node) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
                }
            }
            return OK(res);
        }

        Result<u32> NetlistGraph::get_vertex_of_gate(Gate* g) const
        {
            if (!m_nl)
            {
                return ERR("graph does not correspond to a netlist");
            }

            if (const auto it = m_gates_to_nodes.find(g); it != m_gates_to_nodes.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("no node for gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
            }
        }
    }    // namespace graph_algorithm
}    // namespace hal