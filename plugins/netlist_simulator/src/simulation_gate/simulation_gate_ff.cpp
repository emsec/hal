#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "netlist_simulator/netlist_simulator.h"
#include "netlist_simulator/simulation_utils.h"

namespace hal
{
    NetlistSimulator::SimulationGateFF::SimulationGateFF(const Gate* gate) : SimulationGateSequential(gate)
    {
        const GateType* gate_type       = gate->get_type();
        const FFComponent* ff_component = gate_type->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
        assert(ff_component != nullptr);
        m_clock_func      = ff_component->get_clock_function();
        m_next_state_func = ff_component->get_next_state_function();
        m_preset_func     = ff_component->get_async_set_function();
        m_clear_func      = ff_component->get_async_reset_function();
        for (const std::string& pin : gate_type->get_pins_of_type(PinType::state))
        {
            if (const Net* net = gate->get_fan_out_net(pin); net != nullptr)
            {
                m_state_output_nets.push_back(gate->get_fan_out_net(pin));
            }
        }
        for (const std::string& pin : gate_type->get_pins_of_type(PinType::neg_state))
        {
            if (const Net* net = gate->get_fan_out_net(pin); net != nullptr)
            {
                m_state_inverted_output_nets.push_back(gate->get_fan_out_net(pin));
            }
        }
        for (const std::string& pin : gate_type->get_pins_of_type(PinType::clock))
        {
            m_clock_nets.push_back(gate->get_fan_in_net(pin));
        }
        auto behavior              = ff_component->get_async_set_reset_behavior();
        m_sr_behavior_out          = behavior.first;
        m_sr_behavior_out_inverted = behavior.second;
    }

    void NetlistSimulator::SimulationGateFF::initialize(std::map<const Net*, BooleanFunction::Value>& new_events, bool from_netlist, BooleanFunction::Value value = BooleanFunction::Value::X)
    {
        GateType* gate_type                                       = m_gate->get_type();
        const std::unordered_map<std::string, PinType>& pin_types = gate_type->get_pin_types();

        BooleanFunction::Value inv_value = simulation_utils::toggle(value);

        if (from_netlist)
        {
            // extract init string
            const InitComponent* init_component = gate_type->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
            if (init_component == nullptr)
            {
                log_error("netlist_simulator", "cannot find initialization data for flip-flop '{}' with ID {} of type '{}'.", m_gate->get_name(), m_gate->get_id(), gate_type->get_name());
                return;
            }
            const std::string& init_str = std::get<1>(m_gate->get_data(init_component->get_init_category(), init_component->get_init_identifiers().front()));

            if (!init_str.empty())
            {
                // parse init string
                value = BooleanFunction::Value::X;
                if (init_str == "1")
                {
                    value = BooleanFunction::Value::ONE;
                }
                else if (init_str == "0")
                {
                    value = BooleanFunction::Value::ZERO;
                }
                else
                {
                    log_error("netlist_simulator", "init value of flip-flop '{}' with ID {} of type '{}' is neither '1' or '0'.", m_gate->get_name(), m_gate->get_id(), gate_type->get_name());
                }

                inv_value = simulation_utils::toggle(value);
            }
        }

        // generate events
        for (Endpoint* ep : m_gate->get_fan_out_endpoints())
        {
            if (pin_types.at(ep->get_pin()) == PinType::state)
            {
                new_events[ep->get_net()] = value;
            }
            else if (pin_types.at(ep->get_pin()) == PinType::neg_state)
            {
                new_events[ep->get_net()] = inv_value;
            }
        }
    }

    bool NetlistSimulator::SimulationGateFF::simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        BooleanFunction::Value async_set   = m_preset_func.evaluate(m_input_values);
        BooleanFunction::Value async_reset = m_clear_func.evaluate(m_input_values);

        // check whether an asynchronous set or reset ist triggered
        if (async_set == BooleanFunction::ONE || async_reset == BooleanFunction::ONE)
        {
            BooleanFunction::Value result     = BooleanFunction::Value::X;
            BooleanFunction::Value inv_result = BooleanFunction::Value::X;

            if (async_set == BooleanFunction::ONE && async_reset == BooleanFunction::ONE)
            {
                // both signals set? -> evaluate special behavior
                BooleanFunction::Value old_output = BooleanFunction::Value::X;
                if (!m_state_output_nets.empty())
                {
                    const Net* out_net = m_state_output_nets[0];
                    if (auto it = simulation.m_events.find(out_net); it != simulation.m_events.end())
                    {
                        old_output = it->second.back().new_value;
                    }
                }
                BooleanFunction::Value old_output_inv = BooleanFunction::Value::X;
                if (!m_state_inverted_output_nets.empty())
                {
                    auto out_net = m_state_inverted_output_nets[0];
                    if (auto it = simulation.m_events.find(out_net); it != simulation.m_events.end())
                    {
                        old_output_inv = it->second.back().new_value;
                    }
                }
                result     = simulation_utils::process_clear_preset_behavior(m_sr_behavior_out, old_output);
                inv_result = simulation_utils::process_clear_preset_behavior(m_sr_behavior_out_inverted, old_output_inv);
            }
            else if (async_set == BooleanFunction::ONE)
            {
                // only asynch set is 1
                result     = BooleanFunction::Value::ONE;
                inv_result = simulation_utils::toggle(result);
            }
            else if (async_reset == BooleanFunction::ONE)
            {
                // only asynch reset is 1
                result     = BooleanFunction::Value::ZERO;
                inv_result = simulation_utils::toggle(result);
            }

            // generate events
            for (auto out_net : m_state_output_nets)
            {
                new_events[std::make_pair(out_net, event.time + delay)] = result;
            }
            for (auto out_net : m_state_inverted_output_nets)
            {
                new_events[std::make_pair(out_net, event.time + delay)] = inv_result;
            }

            return true;
        }
        else if (std::find(m_clock_nets.begin(), m_clock_nets.end(), event.affected_net) != m_clock_nets.end())
        {
            // return true if the event was completely handled -> true if the gate is NOT clocked at this point
            return (m_clock_func.evaluate(m_input_values) != BooleanFunction::ONE);
        }

        return true;
    }

    void NetlistSimulator::SimulationGateFF::clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        // compute output
        BooleanFunction::Value result     = m_next_state_func.evaluate(m_input_values);
        BooleanFunction::Value inv_result = simulation_utils::toggle(result);

        // generate events
        for (const Net* out_net : m_state_output_nets)
        {
            new_events[std::make_pair(out_net, current_time + delay)] = result;
        }
        for (const Net* out_net : m_state_inverted_output_nets)
        {
            new_events[std::make_pair(out_net, current_time + delay)] = inv_result;
        }
    }
}    // namespace hal