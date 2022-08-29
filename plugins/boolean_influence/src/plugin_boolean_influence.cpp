#include "boolean_influence/plugin_boolean_influence.h"

#include "hal_core/netlist/netlist_utils.h"
#include "z3_utils/include/plugin_z3_utils.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<BooleanInfluencePlugin>();
    }

    std::string BooleanInfluencePlugin::get_name() const
    {
        return std::string("boolean_influence");
    }

    std::string BooleanInfluencePlugin::get_version() const
    {
        return std::string("0.1");
    }

    void BooleanInfluencePlugin::initialize()
    {
    }

    std::pair<std::map<u32, Gate*>, std::vector<std::vector<double>>> BooleanInfluencePlugin::get_ff_dependency_matrix(const Netlist* nl, bool with_boolean_influence)
    {
        std::map<u32, Gate*> matrix_id_to_gate;
        std::map<Gate*, u32> gate_to_matrix_id;
        std::vector<std::vector<double>> matrix;

        std::unordered_map<u32, std::vector<Gate*>> cache;

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

        u32 status_counter = 0;
        for (const auto& [id, gate] : matrix_id_to_gate)
        {
            if (status_counter % 100 == 0)
            {
                log_info("boolean_influence", "status {}/{} processed", status_counter, matrix_id_to_gate.size());
            }
            status_counter++;
            std::vector<double> line_of_matrix;

            std::set<u32> gates_to_add;
            for (const auto& pred_gate : netlist_utils::get_next_sequential_gates(gate, false, cache))
            {
                gates_to_add.insert(gate_to_matrix_id[pred_gate]);
            }
            std::map<Net*, double> boolean_influence_for_gate;
            if (with_boolean_influence)
            {
                boolean_influence_for_gate = get_boolean_influences_of_gate(gate);
            }

            for (u32 i = 0; i < matrix_gates; i++)
            {
                if (gates_to_add.find(i) != gates_to_add.end())
                {
                    if (with_boolean_influence)
                    {
                        double influence = 0.0;

                        Gate* pred_ff = matrix_id_to_gate[i];

                        for (const auto& output_net : pred_ff->get_fan_out_nets())
                        {
                            if (boolean_influence_for_gate.find(output_net) != boolean_influence_for_gate.end())
                            {
                                influence += boolean_influence_for_gate[output_net];
                            }
                        }

                        line_of_matrix.push_back(influence);
                    }
                    else
                    {
                        line_of_matrix.push_back(1.0);
                    }
                }
                else
                {
                    line_of_matrix.push_back(0.0);
                }
            }
            matrix.push_back(line_of_matrix);
        }

        return std::make_pair(matrix_id_to_gate, matrix);
    }

    std::map<Net*, double> BooleanInfluencePlugin::get_boolean_influences_of_gate(const Gate* gate)
    {
        if (!gate->get_type()->has_property(GateTypeProperty::ff))
        {
            log_error("boolean_influence", "Can only handle flip flops but found gate type {}.", gate->get_type()->get_name());
            return {};
        }

        // Check for the data port pin
        std::unordered_set<std::string> d_ports = gate->get_type()->get_pins_of_type(PinType::data);
        if (d_ports.size() != 1)
        {
            log_error("boolean_influence", "Can only handle flip flops with exactly 1 data port, but found {}.", d_ports.size());
            return {};
        }
        std::string data_pin = *d_ports.begin();
        log_debug("boolean_influence", "Data pin: {}", data_pin);

        // Extract all gates in front of the data port and iterate backwards until another flip flop is found.
        std::vector<Gate*> function_gates = extract_function_gates(gate, data_pin);

        log_debug("boolean_influence", "Extracted {} gates infront of the gate.", function_gates.size());

        // Generate function for the data port

        std::unique_ptr ctx  = std::make_unique<z3::context>();
        std::unique_ptr func = std::make_unique<z3::expr>(*ctx);

        z3_utils::RecursiveSubgraphFunctionGenerator g = {*ctx, function_gates};

        log_debug("boolean_influence", "Created context, function and generator. Trying to generator function now: ");

        if (!function_gates.empty())
        {
            g.get_subgraph_z3_function_recursive(gate->get_fan_in_net(data_pin), *func);
        }
        // edge case if the function gates are empty
        else
        {
            Net* in_net = gate->get_fan_in_net(data_pin);
            *func       = ctx->bv_const(std::to_string(in_net->get_id()).c_str(), 1);
        }

        z3_utils::z3Wrapper func_wrapper = z3_utils::z3Wrapper(std::move(ctx), std::move(func));

        log_debug("boolean_influence", "Built subgraph function, now trying to extract boolean influence.");

        // Generate boolean influence
        std::unordered_map<u32, double> net_ids_to_inf = func_wrapper.get_boolean_influence();

        // translate net_ids back to nets
        std::map<Net*, double> nets_to_inf;

        Netlist* nl = gate->get_netlist();
        for (const auto& [net_id, inf] : net_ids_to_inf)
        {
            Net* net = nl->get_net_by_id(net_id);

            if (net->get_sources().size() > 1)
            {
                log_error("boolean_influence", "Net ({}) has multiple sources ({})", net->get_id(), net->get_sources().size());
                return {};
            }

            nets_to_inf.insert({net, inf});
        }

        return nets_to_inf;
    }

    std::map<Net*, double> BooleanInfluencePlugin::get_boolean_influences_of_subcircuit(const std::vector<Gate*> gates, const Net* start_net)
    {
        for (const auto* gate : gates)
        {
            if (!gate->get_type()->has_property(GateTypeProperty::combinational) || gate->is_vcc_gate() || gate->is_gnd_gate())
            {
                log_error("boolean_influence", "gate '{}' with ID {} is either not a combinational gate or is a VCC or GND gate.", gate->get_name(), gate->get_id());
                return {};
            }
        }

        // Generate function for the data port
        std::unique_ptr ctx  = std::make_unique<z3::context>();
        std::unique_ptr func = std::make_unique<z3::expr>(*ctx);

        z3_utils::RecursiveSubgraphFunctionGenerator g = {*ctx, gates};

        log_debug("boolean_influence", "Created context, function and generator. Trying to generator function now: ");

        if (!gates.empty())
        {
            g.get_subgraph_z3_function_recursive(start_net, *func);
        }
        // edge case if the gates are empty
        else
        {
            *func = ctx->bv_const(std::to_string(start_net->get_id()).c_str(), 1);
        }

        z3_utils::z3Wrapper func_wrapper = z3_utils::z3Wrapper(std::move(ctx), std::move(func));

        log_debug("boolean_influence", "Built subgraph function, now trying to extract boolean influence.");

        // Generate boolean influence
        std::unordered_map<u32, double> net_ids_to_inf = func_wrapper.get_boolean_influence();

        // translate net_ids back to nets
        std::map<Net*, double> nets_to_inf;

        Netlist* nl = gates.front()->get_netlist();
        for (const auto& [net_id, inf] : net_ids_to_inf)
        {
            Net* net = nl->get_net_by_id(net_id);

            if (net->get_sources().size() > 1)
            {
                log_error("boolean_influence", "Net ({}) has multiple sources ({})", net->get_id(), net->get_sources().size());
                return {};
            }

            nets_to_inf.insert({net, inf});
        }

        return nets_to_inf;
    }

    std::vector<Gate*> BooleanInfluencePlugin::extract_function_gates(const Gate* start, const std::string& pin)
    {
        std::unordered_set<Gate*> function_gates;

        auto pre = start->get_predecessor(pin);
        if (pre != nullptr && pre->get_gate() != nullptr)
        {
            add_inputs(pre->get_gate(), function_gates);
        }

        return {function_gates.begin(), function_gates.end()};
    }

    void BooleanInfluencePlugin::add_inputs(Gate* gate, std::unordered_set<Gate*>& gates)
    {
        if (!gate->get_type()->has_property(GateTypeProperty::combinational) || gate->is_vcc_gate() || gate->is_gnd_gate())
        {
            return;
        }

        gates.insert(gate);
        for (const auto& pre : gate->get_predecessors())
        {
            if (pre && pre->get_gate() && gates.find(pre->get_gate()) == gates.end())
            {
                add_inputs(pre->get_gate(), gates);
            }
        }
        return;
    }
}    // namespace hal
