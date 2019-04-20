#include "gate_decorator_system/decorators/gate_decorator_lut.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/netlist.h"

gate_decorator_lut::gate_decorator_lut(std::shared_ptr<gate> g, u32 input_bit_width, u32 output_bit_width, const std::string& config) : gate_decorator(g)
{
    m_input_bit_width  = input_bit_width;
    m_output_bit_width = output_bit_width;
    m_lut_config       = config;
}

gate_decorator_system::decorator_type gate_decorator_lut::get_type()
{
    return gate_decorator_system::decorator_type::LUT;
}

u32 gate_decorator_lut::get_input_bit_width()
{
    return m_input_bit_width;
}

u32 gate_decorator_lut::get_output_bit_width()
{
    return m_output_bit_width;
}

u32 gate_decorator_lut::get_lut_configuration_byte_size()
{
    return (u32)((1 << m_input_bit_width) / 8);
}

u32 gate_decorator_lut::get_lut_configuration_bit_size()
{
    return (u32)(1 << m_input_bit_width);
}

std::vector<bool> gate_decorator_lut::get_lut_configuration()
{
    auto lut_memory = this->get_lut_configuration_string();
    if (lut_memory.empty())
        return std::vector<bool>();

    std::vector<bool> lut((unsigned long)this->get_lut_configuration_bit_size(), false);
    if (this->get_lut_configuration_bit_size() < 8)
    {
        auto lut_value = (u8)std::stoul(lut_memory, 0, 16);
        for (u8 bit_index = 0; bit_index < this->get_lut_configuration_bit_size(); bit_index++)
            lut[bit_index] = (get_bit(lut_value, bit_index) != 0);
    }
    else
    {
        for (u32 byte_index = 0; byte_index < this->get_lut_configuration_byte_size(); byte_index++)
        {
            auto lut_value = (u8)stoul(lut_memory.substr(byte_index * 2, 2), 0, 16);
            for (u8 bit_index = 0; bit_index < 8; bit_index++)
            {
                u32 index  = (this->get_lut_configuration_bit_size() - 1) - 8 * byte_index - bit_index;
                lut[index] = (get_bit(lut_value, (7 - bit_index)) != 0);
            }
        }
    }
    return lut;
}

u8 gate_decorator_lut::get_lut_configuration_byte_by_index(u32 index)
{
    if (this->get_lut_configuration_byte_size() < index)
    {
        log_error("netlist.decorator", "index '{}' is larger than LUT byte size (= '{}')", index, this->get_lut_configuration_byte_size());
        return 0;
    }

    std::vector<bool> lut_memory = this->get_lut_configuration();
    u8 value                     = 0;

    u8 num_of_bits = 8;
    if (this->get_lut_configuration_bit_size() < 8)
        num_of_bits = this->get_lut_configuration_bit_size();

    for (u8 bit_index = 0; bit_index < num_of_bits; bit_index++)
    {
        if (lut_memory[8 * index + bit_index])
        {
            set_bit(value, bit_index);
        }
    }
    return value;
}

std::string gate_decorator_lut::get_lut_configuration_string()
{
    return m_lut_config;
}
