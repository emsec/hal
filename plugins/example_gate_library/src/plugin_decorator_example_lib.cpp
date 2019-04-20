#include "plugin_decorator_example_lib.h"

#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"
#include "gate_decorator_system/gate_decorator_system.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"

std::string plugin_decorator_example_lib::get_name()
{
    return std::string("plugin_decorator_example_lib");
}

std::string plugin_decorator_example_lib::get_version()
{
    return std::string("1.0");
}

// macro to simplify BDD code expressions
#ifndef PIN_TO_BDD
#define PIN_TO_BDD(pin) (*(input_pin_type_to_bdd[pin]))
#endif

namespace bdd_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_lut(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;

        auto bdd_output    = std::make_shared<bdd>();
        auto lut_decorator = std::dynamic_pointer_cast<gate_decorator_lut>(gate_decorator_system::query_decorator(gate, gate_decorator_system::decorator_type::LUT));
        auto lut_memory    = lut_decorator->get_lut_configuration();

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
                    auto& bdd_i = it.second;
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
        result["O"] = bdd_output;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_vcc(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(bdd_true());
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gnd(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(bdd_false());
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_inv(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("I")));
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_and(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_or(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_xor(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(bdd_xor(PIN_TO_BDD("I0"), PIN_TO_BDD("I1")));
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_mux(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>((PIN_TO_BDD("I0") & bdd_not(PIN_TO_BDD("S"))) | (PIN_TO_BDD("I1") & PIN_TO_BDD("S")));
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_buf(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I"));
        return result;
    }
}    // namespace bdd_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators = {
    {"LUT1", bdd_helper::get_bdd_lut}, {"LUT2", bdd_helper::get_bdd_lut}, {"LUT3", bdd_helper::get_bdd_lut}, {"LUT4", bdd_helper::get_bdd_lut},
    {"LUT5", bdd_helper::get_bdd_lut}, {"LUT6", bdd_helper::get_bdd_lut},

    {"VCC", bdd_helper::get_bdd_vcc},  {"GND", bdd_helper::get_bdd_gnd},

    {"INV", bdd_helper::get_bdd_inv},

    {"AND2", bdd_helper::get_bdd_and}, {"AND3", bdd_helper::get_bdd_and}, {"AND4", bdd_helper::get_bdd_and}, {"AND5", bdd_helper::get_bdd_and},
    {"AND6", bdd_helper::get_bdd_and}, {"AND7", bdd_helper::get_bdd_and}, {"AND8", bdd_helper::get_bdd_and},

    {"OR2", bdd_helper::get_bdd_or},   {"OR3", bdd_helper::get_bdd_or},   {"OR4", bdd_helper::get_bdd_or},   {"OR5", bdd_helper::get_bdd_or},
    {"OR6", bdd_helper::get_bdd_or},   {"OR7", bdd_helper::get_bdd_or},   {"OR8", bdd_helper::get_bdd_or},

    {"XOR", bdd_helper::get_bdd_xor},  {"MUX", bdd_helper::get_bdd_mux},

    {"BUF", bdd_helper::get_bdd_buf},
};

static std::map<std::string, std::shared_ptr<bdd>> bdd_generator(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

static bool bdd_availability_tester(std::shared_ptr<gate> g)
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
    {"LUT4", {4, 1}},
    {"LUT5", {5, 1}},
    {"LUT6", {6, 1}},
};

static std::tuple<u32, u32, std::string> lut_generator(std::shared_ptr<gate> g)
{
    auto [inputs, outputs] = m_luts[g->get_type()];
    auto config            = std::get<1>(g->get_data_by_key("generic", "init"));
    if (config.empty())
    {
        config = std::get<1>(g->get_data_by_key("generic", "INIT"));
    }
    return std::make_tuple(inputs, outputs, config);
}

static bool lut_availability_tester(std::shared_ptr<gate> g)
{
    return m_luts.find(g->get_type()) != m_luts.end();
}

void plugin_decorator_example_lib::on_load()
{
    gate_decorator_system::register_bdd_decorator_function("EXAMPLE_GATE_LIBRARY", &bdd_availability_tester, &bdd_generator);
    gate_decorator_system::register_lut_decorator_function("EXAMPLE_GATE_LIBRARY", &lut_availability_tester, &lut_generator);
}

void plugin_decorator_example_lib::on_unload()
{
    gate_decorator_system::remove_bdd_decorator_function("EXAMPLE_GATE_LIBRARY");
    gate_decorator_system::remove_lut_decorator_function("EXAMPLE_GATE_LIBRARY");
}
