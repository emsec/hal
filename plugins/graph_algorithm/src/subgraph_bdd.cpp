#include "plugin_graph_algorithm.h"

#include "core/log.h"
#include "core/plugin_manager.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/gate_decorator_system.h"

#include <queue>

namespace plugin_graph_algorithm_subgraph_bdd_helper
{
    std::map<std::shared_ptr<net>, std::set<std::shared_ptr<gate>>> get_input_nets(std::set<std::shared_ptr<gate>> gates)
    {
        std::map<std::shared_ptr<net>, std::set<std::shared_ptr<gate>>> input_nets;
        for (const auto& gate : gates)
        {
            for (const auto& input_net : gate->get_fan_in_nets())
                input_nets[input_net].insert(gate);
        }
        return input_nets;
    }

    std::set<std::shared_ptr<net>> get_output_nets(std::set<std::shared_ptr<gate>> gates)
    {
        std::set<std::shared_ptr<net>> output_nets;
        for (const auto& gate : gates)
        {
            for (const auto& output_net : gate->get_fan_out_nets())
            {
                output_nets.insert(output_net);
            }
        }
        return output_nets;
    }

    bool is_valid_net_to_bdd_id(std::map<std::shared_ptr<net>, std::set<std::shared_ptr<gate>>> computed_input_nets, std::map<std::shared_ptr<net>, int> parameter_input_net)
    {
        if (computed_input_nets.size() != parameter_input_net.size())
        {
            log_error("graph_algorithm", "parameter 'input_net_to_ids' (size = {}) does not match (computed number of net = {}).", parameter_input_net.size(), computed_input_nets.size());
            return false;
        }

        bool is_equal                    = true;
        std::shared_ptr<net> missing_net = nullptr;
        for (const auto& it : parameter_input_net)
        {
            if (computed_input_nets.find(it.first) == computed_input_nets.end())
            {
                is_equal    = false;
                missing_net = it.first;
                break;
            }
        }
        if (is_equal == false)
        {
            log_error("graph_algorithm", "net '{}' (id = {:08x}) is missing in 'input_net_to_ids' parameter.", missing_net->get_name(), missing_net->get_id());
            return false;
        }
        return true;
    }

    std::map<std::string, std::tuple<std::shared_ptr<bdd>, int>> query_gate_bdds(std::shared_ptr<gate> gate,
                                                                                 std::set<std::shared_ptr<net>> output_nets,
                                                                                 std::map<std::shared_ptr<net>, std::shared_ptr<bdd>> net_to_bdd,
                                                                                 std::map<std::shared_ptr<net>, int> net_to_bdd_id)
    {
        std::map<std::string, std::tuple<std::shared_ptr<bdd>, int>> input_pin_type_to_bdd_id;

        for (const auto& input_pin_type : gate->get_input_pin_types())
        {
            auto input_net = gate->get_fan_in_net(input_pin_type);
            if (output_nets.find(input_net) != output_nets.end())
            {
                /* chain bdd equation if present */
                if (net_to_bdd.find(input_net) == net_to_bdd.end())
                {
                    return std::map<std::string, std::tuple<std::shared_ptr<bdd>, int>>();
                }
                input_pin_type_to_bdd_id[input_pin_type] = std::make_tuple(net_to_bdd[input_net], 0);
            }
            else
            {
                /* insert bdd equation for input net */
                input_pin_type_to_bdd_id[input_pin_type] = std::make_tuple(nullptr, net_to_bdd_id[input_net]);
            }
        }
        return input_pin_type_to_bdd_id;
    }
}    // namespace plugin_graph_algorithm_subgraph_bdd_helper

std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>
    plugin_graph_algorithm::get_bdds(std::shared_ptr<netlist> const g, const std::set<std::shared_ptr<gate>> gates, const std::map<std::shared_ptr<net>, int> input_net_to_ids)
{
    if (g == nullptr)
    {
        log_error(this->get_name(), "parameter 'g' is nullptr");
        return std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>();
    }
    /* check validity of gates */
    if (gates.empty())
    {
        log_error(this->get_name(), "parameter 'gates' is empty");
        return std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>();
    }
    for (const auto& gate : gates)
    {
        if (gate != nullptr)
            continue;
        log_error(this->get_name(), "parameter 'gates' contains a nullptr");
        return std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>();
    }
    /* check validity of nets */
    for (const auto& it : input_net_to_ids)
    {
        if (it.first != nullptr)
            continue;
        log_error(this->get_name(), "parameter 'input_net_to_ids' contains a nullptr");
        return std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>();
    }

    /* local data structures to store the bdds and ids */
    std::map<std::shared_ptr<net>, int> net_to_bdd_id;
    std::map<std::shared_ptr<net>, std::shared_ptr<bdd>> net_to_bdd;

    auto input_nets  = plugin_graph_algorithm_subgraph_bdd_helper::get_input_nets(gates);
    auto output_nets = plugin_graph_algorithm_subgraph_bdd_helper::get_output_nets(gates);

    /* check validity of input net to id assignment */
    if (!input_net_to_ids.empty() && plugin_graph_algorithm_subgraph_bdd_helper::is_valid_net_to_bdd_id(input_nets, input_net_to_ids))
    {
        net_to_bdd_id = input_net_to_ids;
    }
    else
    {
        std::set<int> ordered_net_ids;
        for (const auto& it : input_nets)
            ordered_net_ids.insert(it.first->get_id());
        int id = 0;
        for (const auto& net_id : ordered_net_ids)
            net_to_bdd_id[g->get_net_by_id(net_id)] = id++;
    }
    /* insert gates into queue (ordered by id for determinism) */
    std::set<int> ordererd_gate_ids;
    for (const auto& gate : gates)
        ordererd_gate_ids.insert(gate->get_id());
    std::queue<std::shared_ptr<gate>> gate_queue;
    for (const auto& id : ordererd_gate_ids)
        gate_queue.push(g->get_gate_by_id(id));

    /* initialize counters for processing */
    int run = 0, max_runs = gates.size() * gates.size() * gates.size();

    while (!gate_queue.empty())
    {
        run++;
        /*
        if (run % 100 == 0){
                printf("run %d/%d of max. runs, %d bdds created\n", run, max_runs, bdd_counter);
                printf("gates in queue: %d\n", gate_queue.size());
                std::set<std::shared_ptr<gate>> collection;
                while(!gate_queue.empty()){
                        std::shared_ptr<gate> gate = gate_queue.front(); gate_queue.pop();
                        printf("%s\n", gate->get_name().c_str());
                        collection.insert(gate);
                }
                for (const auto &it : collection) {
                        gate_queue.push(it);
                }

        }
        */
        if (run > max_runs)
        {
            std::string gate_names = "";
            int num_of_gates       = 0;
            while (!gate_queue.empty())
            {
                auto gate = gate_queue.front();
                gate_queue.pop();
                gate_names += gate->get_name() + ", ";
                num_of_gates++;
            }
            gate_names = gate_names.substr(0, gate_names.length() - 2);
            log_error(this->get_name(), "circular dependency among '{}' gates ({}), thus bdds cannot be generated.", num_of_gates, gate_names);
            return std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>();
        }

        auto gate = gate_queue.front();
        gate_queue.pop();

        auto input_pin_type_to_bdd_id = plugin_graph_algorithm_subgraph_bdd_helper::query_gate_bdds(gate, output_nets, net_to_bdd, net_to_bdd_id);
        if (input_pin_type_to_bdd_id.empty() && (!g->is_global_vcc_gate(gate)) && (!g->is_global_gnd_gate(gate)))
        {
            gate_queue.push(gate);
            continue;
        }

        auto bdd_decorator = std::dynamic_pointer_cast<gate_decorator_bdd>(gate_decorator_system::query_decorator(gate, gate_decorator_system::decorator_type::BDD));
        if (bdd_decorator == nullptr)
        {
            log_error(this->get_name(), "cannot query graph gate decorator for gate '{}' ({}).", gate->get_name(), gate->get_type());
            return std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>();
        }
        if ((!g->is_global_vcc_gate(gate)) && (!g->is_global_gnd_gate(gate)))
        {
            auto bdd_map = bdd_decorator->get_bdd(input_pin_type_to_bdd_id);
            for (auto it : bdd_map)
            {
                net_to_bdd[gate->get_fan_out_net(it.first)] = it.second;
            }
        }
        else
        {
            for (auto it : bdd_decorator->get_bdd())
            {
                net_to_bdd[gate->get_fan_out_net(it.first)] = it.second;
            }
        }
        /** DEBUG START */
        /*
        printf("gates : ");
        for (const auto& gate : gates)
            printf("'%s' (%s), ", gate->get_name().c_str(), gate->get_type().c_str());
        printf("\n");

        printf("#input nets:\n");
        for (const auto& it : net_to_bdd_id)
                printf("\t %d : '%s'\n", it.second, it.first->get_name().c_str());
        printf("#output nets:\n");
        for (auto it : net_to_bdd)
                printf("\t '%s' : '%s'\n", it.first->get_name().c_str(), gate_decorator_bdd::get_bdd_str(it.second).c_str());
        */
        /** DEBUG END */
    }
    return std::make_tuple(net_to_bdd_id, net_to_bdd);
}
