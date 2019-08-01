#include "plugin_gate_decorators.h"
#include "gate_decorator_system/gate_decorator_system.h"
#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"

// macro to simplify BDD code expressions
#ifndef PIN_TO_BDD
#define PIN_TO_BDD(pin) (*(input_pin_type_to_bdd[pin]))
#endif

namespace bdd_YOSYS_MYCELL_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_lut(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_vcc(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_VCC")
            result["O"] = std::make_shared<bdd>(bdd_true());
        if (g->get_type() == "VCC")
            result["P"] = std::make_shared<bdd>(bdd_true());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_gnd(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_GND")
            result["O"] = std::make_shared<bdd>(bdd_false());
        if (g->get_type() == "GND")
            result["G"] = std::make_shared<bdd>(bdd_false());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_inv(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "INV")
            result["O"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("I")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_and(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(bdd_true());

        for (const auto pin : g->get_input_pin_types())
        {
            *result["O"] &= PIN_TO_BDD(pin);
        }

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_or(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(bdd_false());

        for (const auto pin : g->get_input_pin_types())
        {
            *result["O"] |= PIN_TO_BDD(pin);
        }

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_xorcy(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "XORCY")
        {
            result["O"] = std::make_shared<bdd>(bdd_xor(PIN_TO_BDD("CI"), PIN_TO_BDD("LI")));
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_mux(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "MUXCY")
        {
            result["O"] = std::make_shared<bdd>((PIN_TO_BDD("CI") & bdd_not(PIN_TO_BDD("S"))) | (PIN_TO_BDD("DI") & PIN_TO_BDD("S")));
        }
        if ((g->get_type() == "MUXF5") || (g->get_type() == "MUXF7"))
        {
            result["O"] = std::make_shared<bdd>((PIN_TO_BDD("I0") & bdd_not(PIN_TO_BDD("S"))) | (PIN_TO_BDD("I1") & PIN_TO_BDD("S")));
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_ld_1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_YOSYS_MYCELL_buf(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if ((g->get_type() == "BUFG") || (g->get_type() == "IBUF") || (g->get_type() == "OBUF"))
        {
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I"));
        }
        return result;
    }
}    // namespace bdd_YOSYS_MYCELL_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators =
    {
        {"GLOBAL_VCC", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_vcc}, {"VCC", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_vcc},
        {"GLOBAL_GND", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_gnd}, {"GND", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_gnd},

        {"INV", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_inv},

        {"XORCY", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_xorcy},

        {"MUXCY", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_mux},      {"MUXF5", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_mux},
        {"MUXF7", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_mux},

        {"LD_1", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_ld_1},      {"BUFG", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_buf},
        {"IBUF", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_buf},       {"OBUF", bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_buf},
};

std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_yosys_mycell(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
{
    auto type = g->get_type();

    gate_decorator_system::bdd_decorator_generator generator;

    if (type.find("AND") != std::string::npos)
    {
        generator = bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_and;
    }
    else if (type.find("OR") != std::string::npos)
    {
        generator = bdd_YOSYS_MYCELL_helper::get_bdd_YOSYS_MYCELL_or;
    }
    else if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        log_error("netlist.decorator", "not implemented reached for gate type '{}'.", type);
        return std::map<std::string, std::shared_ptr<bdd>>();
    }
    else
    {
        generator = m_bbd_generators.at(type);
    }

    return generator(g, input_pin_type_to_bdd);
}

bool plugin_gate_decorators::bdd_availability_tester_yosys_mycell(std::shared_ptr<gate> g)
{
    auto type = g->get_type();

    if (type.find("AND") != std::string::npos)
    {
        return true;
    }

    if (type.find("OR") != std::string::npos)
    {
        return true;
    }

    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}

