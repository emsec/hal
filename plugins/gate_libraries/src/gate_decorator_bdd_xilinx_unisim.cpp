#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"
#include "gate_decorator_system/gate_decorator_system.h"
#include "plugin_gate_decorators.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"

#include <hal_bdd.h>

// macro to simplify BDD code expressions
#ifndef PIN_TO_BDD
#define PIN_TO_BDD(pin) (*(input_pin_type_to_bdd[pin]))
#endif

namespace bdd_xilinx_unisim_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_lut(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        for (const auto& output_type : gate->get_output_pin_types())
        {
            auto bdd_output    = std::make_shared<bdd>();
            auto lut_decorator = std::dynamic_pointer_cast<gate_decorator_lut>(gate_decorator_system::query_decorator(gate, gate_decorator_system::decorator_type::LUT));
            auto lut_memory    = lut_decorator->get_lut_configuration();

            /* special case: lut6_2 gate with o5 pin */
            if ((gate->get_type() == "LUT6_2") && (output_type == "O5"))
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
                        /* special case: lut6_2 gate with o5 pin */
                        if ((gate->get_type() == "LUT6_2") && (output_type == "O5") && (bit_index == 5))
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

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_vcc(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_VCC")
            result["O"] = std::make_shared<bdd>(bdd_true());
        if (g->get_type() == "VCC")
            result["P"] = std::make_shared<bdd>(bdd_true());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_gnd(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_GND")
            result["O"] = std::make_shared<bdd>(bdd_false());
        if (g->get_type() == "GND")
            result["G"] = std::make_shared<bdd>(bdd_false());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_inv(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "INV")
            result["O"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("I")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_and(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "AND2")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") & PIN_TO_BDD("I1"));
        if (g->get_type() == "AND3")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") & PIN_TO_BDD("I1") & PIN_TO_BDD("I2"));
        if (g->get_type() == "AND4")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") & PIN_TO_BDD("I1") & PIN_TO_BDD("I2") & PIN_TO_BDD("I3"));
        if (g->get_type() == "AND5")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") & PIN_TO_BDD("I1") & PIN_TO_BDD("I2") & PIN_TO_BDD("I3") & PIN_TO_BDD("I4"));
        if (g->get_type() == "AND6")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") & PIN_TO_BDD("I1") & PIN_TO_BDD("I2") & PIN_TO_BDD("I3") & PIN_TO_BDD("I4") & PIN_TO_BDD("I5"));
        if (g->get_type() == "AND7")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") & PIN_TO_BDD("I1") & PIN_TO_BDD("I2") & PIN_TO_BDD("I3") & PIN_TO_BDD("I4") & PIN_TO_BDD("I5") & PIN_TO_BDD("I6"));
        if (g->get_type() == "AND8")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") & PIN_TO_BDD("I1") & PIN_TO_BDD("I2") & PIN_TO_BDD("I3") & PIN_TO_BDD("I4") & PIN_TO_BDD("I5") & PIN_TO_BDD("I6") & PIN_TO_BDD("I7"));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_or(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "OR2")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") | PIN_TO_BDD("I1"));
        if (g->get_type() == "OR3")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") | PIN_TO_BDD("I1") | PIN_TO_BDD("I2"));
        if (g->get_type() == "OR4")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") | PIN_TO_BDD("I1") | PIN_TO_BDD("I2") | PIN_TO_BDD("I3"));
        if (g->get_type() == "OR5")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") | PIN_TO_BDD("I1") | PIN_TO_BDD("I2") | PIN_TO_BDD("I3") | PIN_TO_BDD("I4"));
        if (g->get_type() == "OR6")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") | PIN_TO_BDD("I1") | PIN_TO_BDD("I2") | PIN_TO_BDD("I3") | PIN_TO_BDD("I4") | PIN_TO_BDD("I5"));
        if (g->get_type() == "OR7")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") | PIN_TO_BDD("I1") | PIN_TO_BDD("I2") | PIN_TO_BDD("I3") | PIN_TO_BDD("I4") | PIN_TO_BDD("I5") | PIN_TO_BDD("I6"));
        if (g->get_type() == "OR8")
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I0") | PIN_TO_BDD("I1") | PIN_TO_BDD("I2") | PIN_TO_BDD("I3") | PIN_TO_BDD("I4") | PIN_TO_BDD("I5") | PIN_TO_BDD("I6") | PIN_TO_BDD("I7"));

        bdd_printset(*result["O"]);
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_xorcy(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "XORCY")
        {
            result["O"] = std::make_shared<bdd>(bdd_xor(PIN_TO_BDD("CI"), PIN_TO_BDD("LI")));
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_mux(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "MUXCY")
        {
            result["O"] = std::make_shared<bdd>((PIN_TO_BDD("CI") & bdd_not(PIN_TO_BDD("S"))) | (PIN_TO_BDD("DI") & PIN_TO_BDD("S")));
        }
        if ((g->get_type() == "MUXF5") || (g->get_type() == "MUXF7") || (g->get_type() == "MUXF8"))
        {
            result["O"] = std::make_shared<bdd>((PIN_TO_BDD("I0") & bdd_not(PIN_TO_BDD("S"))) | (PIN_TO_BDD("I1") & PIN_TO_BDD("S")));
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_ld_1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "LD_1")
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["D"]) & bdd_not(*(input_pin_type_to_bdd["G"]));
            result["Q"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xilinx_unisim_buf(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I"));
        return result;
    }
}    // namespace bdd_xilinx_unisim_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators = {
    {"LUT1", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},       {"LUT2", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},
    {"LUT3", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},       {"LUT3_D", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},
    {"LUT3_L", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},     {"LUT4", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},
    {"LUT4_D", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},     {"LUT4_L", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},
    {"LUT5", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},       {"LUT6", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},
    {"LUT6_2", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_lut},

    {"GLOBAL_VCC", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_vcc}, {"VCC", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_vcc},
    {"GLOBAL_GND", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_gnd}, {"GND", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_gnd},

    {"INV", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_inv},

    {"AND2", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_and},       {"AND3", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_and},
    {"AND4", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_and},       {"AND5", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_and},
    {"AND6", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_and},       {"AND7", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_and},
    {"AND8", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_and},

    {"OR2", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_or},         {"OR3", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_or},
    {"OR4", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_or},         {"OR5", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_or},
    {"OR6", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_or},         {"OR7", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_or},
    {"OR8", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_or},

    {"XORCY", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_xorcy},

    {"MUXCY", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_mux},      {"MUXF5", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_mux},
    {"MUXF7", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_mux},      {"MUXF8", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_mux},


    {"LD_1", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_ld_1},      {"BUFG", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_buf},
    {"IBUF", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_buf},       {"OBUF", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_buf},
    {"BUFGP", bdd_xilinx_unisim_helper::get_bdd_xilinx_unisim_buf},
};

std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_xilinx_unisim(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

bool plugin_gate_decorators::bdd_availability_tester_xilinx_unisim(std::shared_ptr<gate> g)
{
    auto type = g->get_type();
    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}

static std::map<std::string, std::tuple<u32, u32>> m_luts = {
    {"LUT1", {1, 1}},
    {"LUT2", {2, 1}},
    {"LUT3", {3, 1}},
    {"LUT3_D", {3, 1}},
    {"LUT3_L", {3, 1}},
    {"LUT4", {4, 1}},
    {"LUT4_D", {4, 1}},
    {"LUT4_L", {4, 1}},
    {"LUT5", {5, 1}},
    {"LUT6", {6, 1}},
    {"LUT6_2", {6, 2}},
};

std::tuple<u32, u32, std::string> plugin_gate_decorators::lut_generator_xilinx_unisim(std::shared_ptr<gate> g)
{
    auto [inputs, outputs] = m_luts[g->get_type()];
    auto config            = std::get<1>(g->get_data_by_key("generic", "init"));
    if (config.empty())
    {
        config = std::get<1>(g->get_data_by_key("generic", "INIT"));
    }
    return std::make_tuple(inputs, outputs, config);
}

bool plugin_gate_decorators::lut_availability_tester_xilinx_unisim(std::shared_ptr<gate> g)
{
    return m_luts.find(g->get_type()) != m_luts.end();
}
