#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "graph_algorithm/plugin_graph_algorithm.h"

#include <boost/graph/graphviz.hpp>
#include <boost/graph/strong_components.hpp>

namespace hal
{
    std::vector<std::vector<Gate*>> GraphAlgorithmPlugin::get_strongly_connected_components(Netlist* g, std::vector<Gate*> gates)
    {
        if (g == nullptr)
        {
            log_error(this->get_name(), "{}", "parameter 'g' is nullptr");
            return {};
        }
        /* check validity of gates */
        if (gates.empty())
        {
            gates = g->get_gates();
        }
        for (const auto& current_gate : gates)
        {
            if (current_gate != nullptr)
                continue;
            log_error(this->get_name(), "{}", "parameter 'gates' contains a nullptr");
            return {};
        }

        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> boost_graph;
        typedef boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>>::vertex_descriptor vertex_t;

        /* add ordered gates to directed boost graph */
        std::set<u32> gate_ids;
        for (const auto& current_gate : gates)
            gate_ids.insert(current_gate->get_id());

        std::map<u32, vertex_t> gate_id_to_vertex;
        std::map<vertex_t, u32> vertex_id_to_gate_id;
        u32 vertex_id_cnt = 0;
        for (auto gate_id : gate_ids)
        {
            auto vertex_descriptor                = boost::add_vertex(boost_graph);
            gate_id_to_vertex[gate_id]            = vertex_descriptor;
            vertex_id_to_gate_id[vertex_id_cnt++] = gate_id;
        }

        /* add ordered edges to directed boost graph */
        std::map<u32, Net*> ordered_nets;
        for (const auto& current_gate : gates)
        {
            for (auto net : current_gate->get_fan_out_nets())
                ordered_nets[net->get_id()] = net;
        }

        for (auto it_net : ordered_nets)
        {
            auto src = it_net.second->get_source();
            if (src->get_gate() == nullptr)
                continue;

            std::set<u32> dst_ids;
            for (auto dst : it_net.second->get_destinations())
                dst_ids.insert(dst->get_gate()->get_id());

            for (const auto& dst_id : dst_ids)
                boost::add_edge(gate_id_to_vertex[src->get_gate()->get_id()], gate_id_to_vertex[dst_id], boost_graph);
        }

        /* initialize parameters for strong_components() */
        std::vector<int> component(g->get_gates().size());
        std::vector<int> discover_time(g->get_gates().size());
        std::vector<boost::default_color_type> color(g->get_gates().size());
        std::vector<vertex_t> root(g->get_gates().size());
        int num = strong_components(boost_graph,
                                    make_iterator_property_map(component.begin(), boost::get(boost::vertex_index, boost_graph)),
                                    root_map(make_iterator_property_map(root.begin(), boost::get(boost::vertex_index, boost_graph)))
                                        .color_map(make_iterator_property_map(color.begin(), boost::get(boost::vertex_index, boost_graph)))
                                        .discover_time_map(make_iterator_property_map(discover_time.begin(), boost::get(boost::vertex_index, boost_graph))));

        log("SCC BOOST: {}", num);

        /* post process boost result */
        std::map<int, std::set<u32>> component_to_gate_ids;
        for (std::vector<int>::size_type i = 0; i != component.size(); ++i)
            component_to_gate_ids[component[i]].insert(vertex_id_to_gate_id[i]);

        std::vector<std::vector<Gate*>> result;
        for (auto it_component : component_to_gate_ids)
        {
            std::vector<Gate*> component_set;
            for (auto gate_id : it_component.second)
                component_set.push_back(g->get_gate_by_id(gate_id));
            if (!component_set.empty())
                result.push_back(component_set);
        }
        log_debug(this->get_name(), "found {} components in graph ", num);
        return result;
    }
}    // namespace hal
