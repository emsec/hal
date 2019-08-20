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

namespace bdd_synopsys_nand_nor_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_nand_nor_vcc(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_VCC")
            result["O"] = std::make_shared<bdd>(bdd_true());
        if (g->get_type() == "VCC")
            result["P"] = std::make_shared<bdd>(bdd_true());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_nand_nor_gnd(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_GND")
            result["O"] = std::make_shared<bdd>(bdd_false());
        if (g->get_type() == "GND")
            result["G"] = std::make_shared<bdd>(bdd_false());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_nand_nor_not(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "NOT")
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_nand_nor_nand(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;       

        result["Y"] = std::make_shared<bdd>(bdd_not((*(input_pin_type_to_bdd["A"]) & *(input_pin_type_to_bdd["B"]))));
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_nand_nor_nor(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(g);
        std::map<std::string, std::shared_ptr<bdd>> result;

        result["Y"] = std::make_shared<bdd>(bdd_not((*(input_pin_type_to_bdd["A"]) | *(input_pin_type_to_bdd["B"]))));
        return result;
    }



}    // namespace bdd_synopsys_nand_nor_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators =
    {
        {"GLOBAL_VCC", bdd_synopsys_nand_nor_helper::get_bdd_synopsys_nand_nor_vcc}, {"VCC", bdd_synopsys_nand_nor_helper::get_bdd_synopsys_nand_nor_vcc},
        {"GLOBAL_GND", bdd_synopsys_nand_nor_helper::get_bdd_synopsys_nand_nor_gnd}, {"GND", bdd_synopsys_nand_nor_helper::get_bdd_synopsys_nand_nor_gnd},

        {"NAND", bdd_synopsys_nand_nor_helper::get_bdd_synopsys_nand_nor_nand},      {"NOR", bdd_synopsys_nand_nor_helper::get_bdd_synopsys_nand_nor_nor},
        {"NOT", bdd_synopsys_nand_nor_helper::get_bdd_synopsys_nand_nor_not},
};

std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_synopsys_nand_nor(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
{
    auto type = g->get_type();

    gate_decorator_system::bdd_decorator_generator generator;

   if (m_bbd_generators.find(type) == m_bbd_generators.end())
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

bool plugin_gate_decorators::bdd_availability_tester_synopsys_nand_nor(std::shared_ptr<gate> g)
{
    auto type = g->get_type();

    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}

