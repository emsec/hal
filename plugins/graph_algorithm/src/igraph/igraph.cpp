#include "plugin_graph_algorithm.h"

#include <igraph/igraph.h>
#include <tuple>

#include "core/log.h"
#include "core/plugin_manager.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

std::tuple<igraph_t, std::map<int, std::shared_ptr<gate>>> plugin_graph_algorithm::get_igraph_directed(std::shared_ptr<netlist> nl)
{
    igraph_t graph;
    auto global_output_nets = nl->get_global_output_nets();
    auto global_input_nets  = nl->get_global_input_nets();

    // count all edges, remember in HAL one net(edge) has multiple sinks
    u32 edge_counter = 0;
    for (const auto& net : nl->get_nets())
    {
        auto successors = net->get_dsts();
        edge_counter += successors.size();

        if (global_output_nets.find(net) != global_output_nets.end())
        {
            edge_counter++;
        }
    }

    // initialize edge vector
    igraph_vector_t edges;
    igraph_vector_init(&edges, 2 * edge_counter);

    // we need dummy gates for input/outputs
    u32 dummy_gate_counter   = nl->get_gates().size() - 1;
    u32 edge_vertice_counter = 0;

    for (const auto& net : nl->get_nets())
    {
        // if input net we need to add one dummy input gate
        if (global_input_nets.find(net) != global_input_nets.end())
        {
            u32 dummy_gate = ++dummy_gate_counter;
            for (const auto& successor : net->get_dsts())
            {
                VECTOR(edges)[edge_vertice_counter++] = dummy_gate;
                VECTOR(edges)[edge_vertice_counter++] = successor.get_gate()->get_id() - 1;

                printf("input_gate: %d --> ", dummy_gate);
                printf("%d: %s\n", successor.get_gate()->get_id() - 1, successor.get_gate()->get_name().c_str());
            }
        }
        // if output net we need to add one dummy output gate
        else if (global_output_nets.find(net) != global_output_nets.end())
        {
            auto predecessor                      = net->get_src().gate;
            VECTOR(edges)[edge_vertice_counter++] = predecessor->get_id() - 1;
            VECTOR(edges)[edge_vertice_counter++] = ++dummy_gate_counter;

            printf("%s: %d --> ", predecessor->get_name().c_str(), predecessor->get_id() - 1);
            printf("%d output\n", dummy_gate_counter);

            for (const auto& successor : net->get_dsts())
            {
                VECTOR(edges)[edge_vertice_counter++] = predecessor->get_id() - 1;
                VECTOR(edges)[edge_vertice_counter++] = successor.gate->get_id() - 1;

                printf("%s: %d --> ", predecessor->get_name().c_str(), predecessor->get_id() - 1);
                printf("%d: %s\n", edge_vertice_counter, successor.get_gate()->get_name().c_str());
            }
        }
        else
        {
            auto predecessor = net->get_src().gate;
            for (const auto& successor : net->get_dsts())
            {
                VECTOR(edges)[edge_vertice_counter++] = predecessor->get_id() - 1;
                VECTOR(edges)[edge_vertice_counter++] = successor.gate->get_id() - 1;

                printf("%s: %d --> ", predecessor->get_name().c_str(), predecessor->get_id() - 1);
                printf("%d: %s\n", successor.get_gate()->get_id() - 1, successor.get_gate()->get_name().c_str());
            }
        }
    }

    igraph_create(&graph, &edges, 0, IGRAPH_DIRECTED);

    // map with vertice id to hal-gate
    std::map<int, std::shared_ptr<gate>> vertice_to_gate;
    for (auto const& gate : nl->get_gates())
    {
        vertice_to_gate[gate->get_id() - 1] = gate;
    }

    return std::make_tuple(graph, vertice_to_gate);
}

std::map<int, std::set<std::shared_ptr<gate>>> plugin_graph_algorithm::get_memberships_for_hal(igraph_t graph, igraph_vector_t membership, std::map<int, std::shared_ptr<gate>> vertex_to_gate)
{
    // map back to HAL structures
    int vertices_num = (int)igraph_vcount(&graph);
    std::map<int, std::set<std::shared_ptr<gate>>> community_sets;

    for (int i = 0; i < vertices_num; i++)
    {
        auto gate = vertex_to_gate[i];
        if (gate == nullptr)
            continue;
        community_sets[VECTOR(membership)[i]].insert(gate);
    }
    return community_sets;
}
