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
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C8;
            }
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C9;
            }
        }
        if (gate->has_data("generic", "TOPADDSUB_LOWERINPUT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "TOPADDSUB_LOWERINPUT"));
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C10;
            }
            if (data_entry.at(1) == '1')
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
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C13;
            }
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C14;
            }
        }
        if (gate->has_data("generic", "BOTOUTPUT_SELECT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "BOTOUTPUT_SELECT"));
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C15;
            }
            if (data_entry.at(1) == '1')
            {
                m_configuration |= Configuration::C16;
            }
        }
        if (gate->has_data("generic", "BOTADDSUB_LOWERINPUT"))
        {
            std::string data_entry = std::get<1>(gate->get_data("generic", "BOTADDSUB_LOWERINPUT"));
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C17;
            }
            if (data_entry.at(1) == '1')
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
            if (data_entry.at(0) == '1')
            {
                m_configuration |= Configuration::C20;
            }
            if (data_entry.at(1) == '1')
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
            std::vector<std::string> group_pins;
            std::vector<const Net*> group_nets;
            for (const auto& [index, pin] : gate_type->get_pins_of_group(group))
            {
                UNUSED(index);
                group_pins.push_back(pin);
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

        if (m_configuration == 0x184C98F)    // 1100001001100100110001111
        {
            if (m_input_values.at("IRSTTOP") == BooleanFunction::Value::ONE)
            {
                m_in_reg_a = 0;
                m_in_reg_c = 0;
            }
            else
            {
                if (m_input_values.at("AHOLD") != BooleanFunction::Value::ONE)
                {
                    m_in_reg_a = simulation_utils::get_int_bus_value(m_input_values, m_data_in_pins.at(0));
                }
                if (m_input_values.at("CHOLD") != BooleanFunction::Value::ONE)
                {
                    m_in_reg_c = simulation_utils::get_int_bus_value(m_input_values, m_data_in_pins.at(2));
                }
            }
            if (m_input_values.at("IRSTBOT") == BooleanFunction::Value::ONE)
            {
                m_in_reg_b = 0;
                m_in_reg_d = 0;
            }
            else
            {
                if (m_input_values.at("BHOLD") != BooleanFunction::Value::ONE)
                {
                    m_in_reg_b = simulation_utils::get_int_bus_value(m_input_values, m_data_in_pins.at(1));
                }
                if (m_input_values.at("DHOLD") != BooleanFunction::Value::ONE)
                {
                    m_in_reg_d = simulation_utils::get_int_bus_value(m_input_values, m_data_in_pins.at(3));
                }
            }

            // generate output events
            std::vector<BooleanFunction::Value> output_values = simulation_utils::int_to_values(((u32)m_acc_out_reg1 << 16) | m_acc_out_reg2, 32);

            for (u32 i = 0; i < output_values.size(); i++)
            {
                new_events[std::make_pair(m_data_out_nets.at(i), current_time + delay)] = output_values.at(i);
            }

            // compute new accumulator result
            // TODO deal with reset, hold, load, add/sub
            u32 tmp        = m_mul_out_reg + ((m_acc_out_reg1 << 16) | m_acc_out_reg2);
            m_acc_out_reg1 = tmp >> 16;
            m_acc_out_reg2 = tmp & 0xFFFF;

            // compute new multiplier result
            // TODO deal with reset
            m_mul_out_reg = m_in_reg_a * m_in_reg_b;
        }
        else
        {
            // unsupported configuration
            log_error("netlist_simulator",
                      "MAC configuration '{:07X}' of gate '{}' with ID {} of type '{}' is not supported.",
                      m_configuration,
                      m_gate->get_name(),
                      m_gate->get_id(),
                      m_gate->get_type()->get_name());
            return;
        }

        // 1100001001100100110001111
        // .B_SIGNED(1'b1),
        // .A_SIGNED(1'b1),
        // .MODE_8x8(1'b0),
        // .BOTADDSUB_CARRYSELECT(2'b00),
        // .BOTADDSUB_UPPERINPUT(1'b0),
        // .BOTADDSUB_LOWERINPUT(2'b10),
        // .BOTOUTPUT_SELECT(2'b01),
        // .TOPADDSUB_CARRYSELECT(2'b10),
        // .TOPADDSUB_UPPERINPUT(1'b0),
        // .TOPADDSUB_LOWERINPUT(2'b10),
        // .TOPOUTPUT_SELECT(2'b01),
        // .PIPELINE_16x16_MULT_REG2(1'b1),
        // .PIPELINE_16x16_MULT_REG1(1'b0),
        // .BOT_8x8_MULT_REG(1'b0),
        // .TOP_8x8_MULT_REG(1'b0),
        // .D_REG(1'b1),
        // .B_REG(1'b1),
        // .A_REG(1'b1),
        // .C_REG(1'b1)
    }
}    // namespace hal