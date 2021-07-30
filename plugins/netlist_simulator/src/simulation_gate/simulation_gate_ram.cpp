#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"
#include "netlist_simulator/netlist_simulator.h"
#include "netlist_simulator/simulation_utils.h"

namespace hal
{
    namespace
    {
        u32 get_data_word(const std::vector<u64>& data, u32 address, u32 width)
        {
            u32 start_pos = address * width;
            u32 row       = start_pos >> 6;
            u32 start_bit = start_pos & 0x3F;

            if (row >= data.size() || 64 % width != 0 || width > 32)
            {
                log_error("netlist_simulator", "cannot read data word of width {} at address {0:x}.", width, address);
                return 0;
            }

            // reads right to left
            return (u32)((data.at(row) >> start_bit) & ((1 << width) - 1));
        }

        void set_data_word(std::vector<u64>& data, u32 in_data, u32 address, u32 width)
        {
            u32 start_pos = address * width;
            u32 row       = start_pos >> 6;
            u32 start_bit = start_pos & 0x3F;

            if (row >= data.size() || 64 % width != 0 || width > 32)
            {
                log_error("netlist_simulator", "cannot write data word {0:x} of width {} to address {0:x}.", in_data, width, address);
                return;
            }

            // reads right to left
            u64 mask     = ~((((u64)1 << width) - 1) << start_bit);
            data.at(row) = (data.at(row) & mask) ^ ((u64)in_data << start_bit);
        }

        std::vector<BooleanFunction::Value> int_to_values(u32 integer, u32 len)
        {
            if (len > 32)
            {
                return {};
            }

            std::vector<BooleanFunction::Value> res;
            for (u32 i = 0; i < len; i++)
            {
                res.push_back((BooleanFunction::Value)((integer >> i) & 1));
            }

            return res;
        }

        u32 values_to_int(const std::vector<BooleanFunction::Value>& values)
        {
            u32 len = values.size();

            if (len > 32)
            {
                return 0;
            }

            u32 res = 0;
            for (u32 i = 0; i < len; i++)
            {
                BooleanFunction::Value val = values.at(i);
                if (val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z)
                {
                    return 0;
                }

                res ^= (u32)val << i;
            }

            return res;
        }
    }    // namespace

    NetlistSimulator::SimulationGateRAM::SimulationGateRAM(const Gate* gate) : SimulationGateSequential(gate)
    {
        const GateType* gate_type         = gate->get_type();
        const RAMComponent* ram_component = gate_type->get_component_as<RAMComponent>([](const GateTypeComponent* c) { return RAMComponent::is_class_of(c); });
        assert(ram_component != nullptr);

        m_bit_size = ram_component->get_bit_size();

        for (const GateTypeComponent* component : ram_component->get_components([](const GateTypeComponent* c) { return RAMPortComponent::is_class_of(c); }))
        {
            const RAMPortComponent* port_component = component->convert_to<RAMPortComponent>();

            Port simulation_port;
            simulation_port.clock_func  = port_component->get_clock_function();
            simulation_port.enable_func = port_component->get_enable_function();
            simulation_port.is_write    = port_component->is_write_port();

            for (const auto& [index, pin] : gate_type->get_pins_of_group(port_component->get_data_group()))
            {
                simulation_port.data_pins.push_back(pin);
            }

            for (const auto& [index, pin] : gate_type->get_pins_of_group(port_component->get_address_group()))
            {
                simulation_port.address_pins.push_back(pin);
            }

            // determine clock net
            for (const std::string& var : simulation_port.clock_func.get_variables())
            {
                if (gate_type->get_pin_type(var) == PinType::clock)
                {
                    simulation_port.clock_net = gate->get_fan_out_net(var);
                }
            }

            assert(simulation_port.clock_net != nullptr);
        }
    }

    void NetlistSimulator::SimulationGateRAM::initialize(std::map<const Net*, BooleanFunction::Value>& new_events, bool from_netlist, BooleanFunction::Value value)
    {
        GateType* gate_type = m_gate->get_type();

        if (from_netlist)
        {
            const RAMComponent* ram_component = gate_type->get_component_as<RAMComponent>([](const GateTypeComponent* c) { return RAMComponent::is_class_of(c); });
            if (ram_component == nullptr)
            {
                log_error("netlist_simulator", "cannot find RAM properties for RAM gate '{}' with ID {} of type '{}'.", m_gate->get_name(), m_gate->get_id(), gate_type->get_name());
                return;
            }

            const InitComponent* init_component = gate_type->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
            if (init_component == nullptr)
            {
                log_error("netlist_simulator", "cannot find initialization data for RAM gate '{}' with ID {} of type '{}'.", m_gate->get_name(), m_gate->get_id(), gate_type->get_name());
                return;
            }

            const std::string& category = init_component->get_init_category();

            for (const std::string& identifier : init_component->get_init_identifiers())
            {
                const std::string& data = std::get<1>(m_gate->get_data(category, identifier));

                u32 data_len = data.size();
                assert(data_len % 16 == 0);

                for (u32 i = 0; i < data_len; i += 16)
                {
                    m_data.push_back(strtoull(data.substr(data_len - i - 16, 16).c_str(), nullptr, 16));
                }
            }

            if (ram_component->get_bit_size() != m_data.size() * 64)
            {
                log_error("netlist_simulator", "initialization data does not fit memory size for RAM gate '{}' with ID {} of type '{}'.", m_gate->get_name(), m_gate->get_id(), gate_type->get_name());
                m_data.clear();
                return;
            }
        }
        else
        {
            u64 init_val;

            switch (value)
            {
                case BooleanFunction::Value::ONE:
                    init_val = 0xFFFFFFFFFFFFFFFF;
                    break;
                case BooleanFunction::Value::ZERO:
                    init_val = 0x0;
                    break;
                case BooleanFunction::Value::X:
                case BooleanFunction::Value::Z:
                    break;
                    // TODO handle
            }
            // INIT with fixed value
        }
    }

    bool NetlistSimulator::SimulationGateRAM::simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        UNUSED(simulation);
        UNUSED(new_events);

        // is the event triggering a clock pin? if so, remember the clocked port and process later
        for (size_t i = 0; i < m_ports.size(); i++)
        {
            Port& port = m_ports.at(i);
            if (event.affected_net == port.clock_net && port.clock_func.evaluate(m_input_values) == BooleanFunction::ONE)
            {
                m_clocked_port_indices.push_back(i);
                return false;
            }
        }

        // gate fully handled
        return true;
    }

    void NetlistSimulator::SimulationGateRAM::clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        for (size_t i : m_clocked_port_indices)
        {
            Port& port = m_ports.at(i);

            if (port.enable_func.evaluate(m_input_values) != BooleanFunction::ONE)
            {
                continue;
            }

            if (port.clock_func.evaluate(m_input_values) != BooleanFunction::ONE)
            {
                continue;
            }

            std::vector<BooleanFunction::Value> address_values;
            for (const std::string& pin : port.address_pins)
            {
                address_values.push_back(m_input_values.at(pin));
            }

            u32 address   = values_to_int(address_values);
            u32 data_size = port.data_pins.size();

            if (!port.is_write)
            {
                // read data from internal memory

                u32 read_data                                   = get_data_word(m_data, address, data_size);
                std::vector<BooleanFunction::Value> data_values = int_to_values(read_data, data_size);

                assert(data_values.size() == port.data_pins.size());

                // generate events
                for (u32 j = 0; j < port.data_pins.size(); j++)
                {
                    const Net* out_net                                        = m_gate->get_fan_out_net(port.data_pins.at(j));
                    new_events[std::make_pair(out_net, current_time + delay)] = data_values.at(j);
                }
            }
            else
            {
                // write data to internal memory
                std::vector<BooleanFunction::Value> data_values;
                for (const std::string& pin : port.data_pins)
                {
                    data_values.push_back(m_input_values.at(pin));
                }
                u32 write_data = values_to_int(data_values);
                set_data_word(m_data, write_data, address, data_size);
            }
        }

        m_clocked_port_indices.clear();
    }
}    // namespace hal