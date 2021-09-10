#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"
#include "netlist_simulator/netlist_simulator.h"
#include "netlist_simulator/simulation_utils.h"

namespace hal
{
    NetlistSimulator::SimulationGateLatticeMAC::SimulationGateLatticeMAC(const Gate* gate) : SimulationGateSequential(gate)
    {
        const GateType* gate_type = gate->get_type();

        m_clock_net = gate->get_fan_in_net("CLK");
        assert(m_clock_net != nullptr);
        m_clock_func = BooleanFunction::from_string("CLK & CE");

        if (gate->has_data("generic", "C_REG") && std::get<1>(gate->get_data("generic", "C_REG")) == "1")
        {
            m_configuration |= Configuration::C0;
        }
        if (gate->has_data("generic", "A_REG") && std::get<1>(gate->get_data("generic", "A_REG")) == "1")
        {
            m_configuration |= Configuration::C1;
        }
        if (gate->has_data("generic", "B_REG") && std::get<1>(gate->get_data("generic", "B_REG")) == "1")
        {
            m_configuration |= Configuration::C2;
        }
        if (gate->has_data("generic", "D_REG") && std::get<1>(gate->get_data("generic", "D_REG")) == "1")
        {
            m_configuration |= Configuration::C3;
        }
        if (gate->has_data("generic", "TOP_8x8_MULT_REG") && std::get<1>(gate->get_data("generic", "TOP_8x8_MULT_REG")) == "1")
        {
            m_configuration |= Configuration::C4;
        }
        if (gate->has_data("generic", "BOT_8x8_MULT_REG") && std::get<1>(gate->get_data("generic", "BOT_8x8_MULT_REG")) == "1")
        {
            m_configuration |= Configuration::C5;
        }
        if (gate->has_data("generic", "PIPELINE_16x16_MULT_REG1") && std::get<1>(gate->get_data("generic", "PIPELINE_16x16_MULT_REG1")) == "1")
        {
            m_configuration |= Configuration::C6;
        }
        if (gate->has_data("generic", "PIPELINE_16x16_MULT_REG2") && std::get<1>(gate->get_data("generic", "PIPELINE_16x16_MULT_REG2")) == "1")
        {
            m_configuration |= Configuration::C7;
        }
        if (gate->has_data("generic", "TOPOUTPUT_SELECT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "TOPOUTPUT_SELECT"));
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C8;
            }
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C9;
            }
        }
        if (gate->has_data("generic", "TOPADDSUB_LOWERINPUT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "TOPADDSUB_LOWERINPUT"));
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C10;
            }
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C11;
            }
        }
        if (gate->has_data("generic", "TOPADDSUB_UPPERINPUT") && std::get<1>(gate->get_data("generic", "TOPADDSUB_UPPERINPUT")) == "1")
        {
            m_configuration |= Configuration::C12;
        }
        if (gate->has_data("generic", "TOPADDSUB_CARRYSELECT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "TOPADDSUB_CARRYSELECT"));
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C13;
            }
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C14;
            }
        }
        if (gate->has_data("generic", "BOTOUTPUT_SELECT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "BOTOUTPUT_SELECT"));
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C15;
            }
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C16;
            }
        }
        if (gate->has_data("generic", "BOTADDSUB_LOWERINPUT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "BOTADDSUB_LOWERINPUT"));
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C17;
            }
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C18;
            }
        }
        if (gate->has_data("generic", "BOTADDSUB_UPPERINPUT") && std::get<1>(gate->get_data("generic", "BOTADDSUB_UPPERINPUT")) == "1")
        {
            m_configuration |= Configuration::C19;
        }
        if (gate->has_data("generic", "BOTADDSUB_CARRYSELECT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "BOTADDSUB_CARRYSELECT"));
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C20;
            }
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C21;
            }
        }
        if (gate->has_data("generic", "MODE_8x8") && std::get<1>(gate->get_data("generic", "MODE_8x8")) == "1")
        {
            m_configuration |= Configuration::C22;
        }
        if (gate->has_data("generic", "A_SIGNED") && std::get<1>(gate->get_data("generic", "A_SIGNED")) == "1")
        {
            m_configuration |= Configuration::C23;
        }
        if (gate->has_data("generic", "B_SIGNED") && std::get<1>(gate->get_data("generic", "B_SIGNED")) == "1")
        {
            m_configuration |= Configuration::C24;
        }

        for (const std::string& group : {"A", "B", "C", "D"})
        {
            std::vector<const Net*> group_nets;
            for (const auto& [index, pin] : gate_type->get_pins_of_group(group))
            {
                UNUSED(index);
                group_nets.push_back(gate->get_fan_in_net(pin));
            }
            m_data_in_nets.push_back(group_nets);
        }

        for (const auto& [index, pin] : gate_type->get_pins_of_group("O"))
        {
            UNUSED(index);
            m_data_out_nets.push_back(gate->get_fan_out_net(pin));
        }

        for (const std::string& pin : {"AHOLD", "BHOLD", "CHOLD", "DHOLD"})
        {
            m_hold_nets.push_back(gate->get_fan_in_net(pin));
        }
    }

    void NetlistSimulator::SimulationGateLatticeMAC::initialize(std::map<const Net*, BooleanFunction::Value>& new_events, bool from_netlist, BooleanFunction::Value value)
    {
        UNUSED(new_events);
        UNUSED(from_netlist);
        UNUSED(value);
    }

    bool NetlistSimulator::SimulationGateLatticeMAC::simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        UNUSED(simulation);
        UNUSED(new_events);

        // is the event triggering the clock pin? if so, remember and process later
        if (event.affected_net == m_clock_net && m_clock_func.evaluate(m_input_values) == BooleanFunction::ONE)
        {
            return false;
        }

        // gate fully handled
        return true;
    }

    void NetlistSimulator::SimulationGateLatticeMAC::clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        // TODO implement
    }
}    // namespace hal