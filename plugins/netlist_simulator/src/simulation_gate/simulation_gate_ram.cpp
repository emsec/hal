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
                return 0;
            }

            // TODO extract data
            return (u32)((data.at(row) >> (64 - start_bit - width)) & ((1 << width) - 1));
        }

        void set_data_word(std::vector<u64>& data, u32 new_data, u32 address, u32 width)
        {
            u32 start_pos = address * width;
            u32 row       = start_pos >> 6;
            u32 start_bit = start_pos & 0x3F;

            if (row >= data.size() || 64 % width != 0 || width > 32)
            {
                return;
            }

            // TODO overwrite data
        }
    }    // namespace

    bool NetlistSimulator::SimulationGateRAM::simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        // is the event triggering a clock pin? if so, remember the clocked port and process later
        for (size_t i = 0; i < ports.size(); i++)
        {
            Port& port = ports.at(i);
            if (event.affected_net == port.clock_net && port.clock_func.evaluate(input_values) == BooleanFunction::ONE)
            {
                clocked_port_indices.push_back(i);
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

        for (size_t i : clocked_port_indices)
        {
            Port& port = ports.at(i);

            if (port.enable_func.evaluate(input_values) != BooleanFunction::ONE)
            {
                continue;
            }
        }

        // TODO for each port, check whether read or write is enabled
        // TODO assemble read/write adresses
        // TODO read/write data/to from memory
        // TODO set data output

        clocked_port_indices.clear();
    }
}    // namespace hal