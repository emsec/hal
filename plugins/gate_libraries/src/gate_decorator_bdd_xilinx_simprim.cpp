#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"
#include "gate_decorator_system/gate_decorator_system.h"
#include "plugin_gate_decorators.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"

// macro to simplify BDD code expressions
#ifndef PIN_TO_BDD
#define PIN_TO_BDD(pin) (*(input_pin_type_to_bdd[pin]))
#endif

namespace bdd_xilinx_simprim_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_lut(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        for (const auto& output_type : gate->get_output_pin_types())
        {
            auto bdd_output    = std::make_shared<bdd>();
            auto lut_decorator = std::dynamic_pointer_cast<gate_decorator_lut>(gate_decorator_system::query_decorator(gate, gate_decorator_system::decorator_type::LUT));
            auto lut_memory    = lut_decorator->get_lut_configuration();

            /* special case: x_lut6_2 gate with o5 pin */
            if ((gate->get_type() == "X_LUT6_2") && (output_type == "O5"))
                lut_memory.resize(32);

            for (u32 addr = 0; addr < lut_memory.size(); addr++)
            {
                auto tmp = new bdd();
                *tmp     = bdd_false();
                if (lut_memory[addr])
                {
                    *tmp          = bdd_true();
                    u32 bit_index = 0;
                    for (const auto& it : input_pin_type_to_bdd)
                    {
                        /* special case: x_lut6_2 gate with o5 pin */
                        if ((gate->get_type() == "X_LUT6_2") && (output_type == "O5") && (bit_index == 5))
                            break;

                        auto bdd_i = it.second;
                        if (get_bit(addr, bit_index) == 0)
                            *tmp = bdd_and(*tmp, bdd_not(*bdd_i));
                        else
                            *tmp = bdd_and(*tmp, *bdd_i);
                        bit_index++;
                    }
                }
                *bdd_output = bdd_or(*bdd_output, *tmp);
                delete tmp;
            }
            result[output_type] = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_mux(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "X_MUX2")
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (*(input_pin_type_to_bdd["IA"]) & bdd_not(*(input_pin_type_to_bdd["SEL"]))) | (*(input_pin_type_to_bdd["IB"]) & (*(input_pin_type_to_bdd["SEL"])));
            result["O"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_vcc(std::shared_ptr<gate> gate, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if ((gate->get_type() == "X_ONE") || (gate->get_type() == "GLOBAL_VCC"))
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_true();
            result["O"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_gnd(std::shared_ptr<gate> gate, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if ((gate->get_type() == "X_ZERO") || (gate->get_type() == "GLOBAL_GND"))
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_false();
            result["O"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_inv(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "X_INV")
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(*(input_pin_type_to_bdd["I"]));
            result["O"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_xor2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "X_XOR2")
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_xor(*(input_pin_type_to_bdd["I0"]), *(input_pin_type_to_bdd["I1"]));
            result["O"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_carry4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "X_CARRY4")
        {
            auto o_output  = std::make_shared<bdd>();
            auto co_output = std::make_shared<bdd>();
            *co_output     = bdd_or(*(input_pin_type_to_bdd["CYINIT"]), *(input_pin_type_to_bdd["CI"]));
            *o_output      = bdd_xor(*(input_pin_type_to_bdd["S_0"]), *co_output);
            result["O_0"]  = o_output;

            *co_output &= *(input_pin_type_to_bdd["S_0"]);
            *co_output |= bdd_and(*(input_pin_type_to_bdd["DI_0"]), bdd_not(*(input_pin_type_to_bdd["S_0"])));
            result["CO_0"] = co_output;

            *o_output     = bdd_xor(*(input_pin_type_to_bdd["S_1"]), *co_output);
            result["O_1"] = o_output;

            *co_output &= *(input_pin_type_to_bdd["S_1"]);
            *co_output |= bdd_and(*(input_pin_type_to_bdd["DI_1"]), bdd_not(*(input_pin_type_to_bdd["S_1"])));
            result["CO_1"] = co_output;

            *o_output     = bdd_xor(*(input_pin_type_to_bdd["S_2"]), *co_output);
            result["O_2"] = o_output;

            *co_output &= *(input_pin_type_to_bdd["S_2"]);
            *co_output |= bdd_and(*(input_pin_type_to_bdd["DI_2"]), bdd_not(*(input_pin_type_to_bdd["S_2"])));
            result["CO_2"] = co_output;

            *o_output     = bdd_xor(*(input_pin_type_to_bdd["S_3"]), *co_output);
            result["O_3"] = o_output;

            *co_output &= *(input_pin_type_to_bdd["S_3"]);
            *co_output |= bdd_and(*(input_pin_type_to_bdd["DI_3"]), bdd_not(*(input_pin_type_to_bdd["S_3"])));
            result["CO_3"] = co_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_simprim_buf(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "X_BUF")
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["I"]);
            result["O"]     = bdd_output;
        }
        return result;
    }
}    // namespace bdd_xilinx_simprim_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators = {
    {"X_LUT2", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_LUT3", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_LUT4", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_LUT5", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_LUT6", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_LUT6_2", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_LUT7", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_LUT8", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_lut},
    {"X_MUX2", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_mux},
    {"X_ONE", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_vcc},
    {"X_INV", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_inv},
    {"X_ZERO", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_gnd},
    {"GLOBAL_VCC", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_vcc},
    {"GLOBAL_GND", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_gnd},
    {"X_CARRY4", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_carry4},
    {"X_XOR2", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_xor2},
    {"X_BUF", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_buf},
    {"X_OBUF", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_buf},
    {"X_CKBUF", bdd_xilinx_simprim_helper::get_bdd_xilinx_simprim_buf},
};

std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_xilinx_simprim(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
{
    auto type = g->get_type();

    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        log_error("netlist.decorator", "not implemented reached for gate type '{}'.", type);
        return std::map<std::string, std::shared_ptr<bdd>>();
    }

    gate_decorator_system::bdd_decorator_generator generator = m_bbd_generators.at(type);
    return generator(g, input_pin_type_to_bdd);
}

bool plugin_gate_decorators::bdd_availability_tester_xilinx_simprim(std::shared_ptr<gate> g)
{
    auto type = g->get_type();
    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}

static std::map<std::string, std::tuple<u32, u32>> m_luts = {
    {"X_LUT2", {2, 1}},
    {"X_LUT3", {3, 1}},
    {"X_LUT4", {4, 1}},
    {"X_LUT5", {5, 1}},
    {"X_LUT6", {6, 1}},
    {"X_LUT6_2", {6, 2}},
    {"X_LUT7", {7, 1}},
    {"X_LUT8", {8, 1}},
    {"X_SRLC16E", {4, 1}},
    {"X_SRLC32E", {5, 1}},
};

std::tuple<u32, u32, std::string> plugin_gate_decorators::lut_generator_xilinx_simprim(std::shared_ptr<gate> g)
{
    auto [inputs, outputs] = m_luts[g->get_type()];
    auto config            = std::get<1>(g->get_data_by_key("generic", "init"));
    if (config.empty())
    {
        config = std::get<1>(g->get_data_by_key("generic", "INIT"));
    }
    return std::make_tuple(inputs, outputs, config);
}

bool plugin_gate_decorators::lut_availability_tester_xilinx_simprim(std::shared_ptr<gate> g)
{
    return m_luts.find(g->get_type()) != m_luts.end();
}
