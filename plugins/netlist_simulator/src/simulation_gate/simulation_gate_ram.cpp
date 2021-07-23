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
        // TODO do init stuff here
        // no reading of init data here, will be outsourced to differnet function
        // make sure to read INIT data from right to left in 64 bit chunks
        // length of init needs to be determined somewhere
    }

    bool NetlistSimulator::SimulationGateRAM::simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

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

            u32 address = values_to_int(address_values);

            if (!port.is_write)
            {
                // read data from internal memory
                u32 port_size                                   = port.data_pins.size();
                u32 read_data                                   = get_data_word(m_data, address, port_size);
                std::vector<BooleanFunction::Value> data_values = int_to_values(read_data, port_size);

                assert(data_values.size() == port.data_pins.size());

                // generate events
                for (u32 i = 0; i < port.data_pins.size(); i++)
                {
                    const Net* out_net                                        = m_gate->get_fan_out_net(port.data_pins.at(i));
                    new_events[std::make_pair(out_net, current_time + delay)] = data_values.at(i);
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
                set_data_word(m_data, write_data, address, port.data_pins.size());
            }
        }

        m_clocked_port_indices.clear();
    }
}    // namespace hal