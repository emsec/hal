#include "boolean_influence/plugin_boolean_influence.h"

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

    std::map<Gate*, double> BooleanInfluencePlugin::get_boolean_influences_of_gate(const Gate* gate) 
    {
        if (gate->get_type()->get_base_type() != hal::GateType::BaseType::ff)
        {
            log_error("boolean_influence", "Can only handle flip flops with exactly 1 data port, but found {}.");
            return {};
        }


        // Check for the data port pin
        std::unordered_set<std::string> d_ports = gate->get_type()->get_pins_of_type(GateType::PinType::data);
        if (d_ports.size() != 1)
        {
            log_error("boolean_influence", "Can only handle flip flops with exactly 1 data port, but found {}.");
            return {};
        }
        std::string data_pin = *d_ports.begin();

        // Extract all gates in front of the data port and iterate backwards until another flip flop is found.
        std::vector<Gate*> function_gates = extract_function_gates(gate, data_pin);

        // Generate function for the data port
        z3_utils::SubgraphFunctionGenerator g;

        std::unique_ptr ctx = std::make_unique<z3::context>();
        z3::expr func(*ctx);
        std::unordered_set<u32> net_ids;
        g.get_subgraph_z3_function(gate->get_fan_in_net(data_pin), function_gates, *ctx, func, net_ids);

        z3_utils::z3Wrapper func_wrapper = z3_utils::z3Wrapper(std::move(ctx), func);

        log_info("boolean_influence", "boolean function generated, starting analysis");

        // Generate boolean influence
        std::unordered_map<u32, double> net_ids_to_inf = func_wrapper.get_boolean_influence();

        // translate net_ids back to gates
        std::map<Gate*, double> gates_to_inf;

        Netlist* nl = gate->get_netlist();
        for (const auto& [net_id, inf] : net_ids_to_inf) {
            Net* net = nl->get_net_by_id(net_id);

            if (net->get_sources().size() != 1) {
                log_error("boolean_influence", "Net ({}) has either multiple or 0 sources ({})", net->get_id(), net->get_sources().size());
                return {};
            }

            if (net->get_sources().front()->get_gate() == nullptr) {
                log_error("boolean_influence", "Net ({}) does not end in a gate.", net->get_id());
                return {};
            }

            Gate* gate = net->get_sources().front()->get_gate();
            gates_to_inf.insert({gate, inf});
        }

        return gates_to_inf;
    }

    std::vector<Gate*> BooleanInfluencePlugin::extract_function_gates(const Gate* start, const std::string& pin) 
    {
        std::unordered_set<Gate*> function_gates;
        
        add_inputs(start->get_predecessor(pin)->get_gate(), function_gates);

        return {function_gates.begin(), function_gates.end()};
    }

    void BooleanInfluencePlugin::add_inputs(Gate* gate, std::unordered_set<Gate*>& gates)
    {
        if (gate->is_vcc_gate() || gate->is_gnd_gate() || gate->get_type()->get_base_type() == hal::GateType::BaseType::ff || gate->get_type()->get_name().find("RAM") != std::string::npos)
        {
            return;
        }

        gates.insert(gate);
        for (const auto& pre : gate->get_predecessors())
        {
            if (pre->get_gate() && gates.find(pre->get_gate()) == gates.end())
            {
                add_inputs(pre->get_gate(), gates);
            }
        }
        return;
    }
}
