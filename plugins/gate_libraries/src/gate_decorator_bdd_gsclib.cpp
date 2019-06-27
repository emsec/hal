#include "plugin_gate_decorators.h"

#include "gate_decorator_system/gate_decorator_system.h"
#include "gate_decorator_system/decorators/gate_decorator_bdd.h"

#include "core/log.h"
#include "netlist/gate.h"

// macro to simplify BDD code expressions
#ifndef PIN_TO_BDD
#define PIN_TO_BDD(pin) (*(input_pin_type_to_bdd[pin]))
#endif

namespace bdd_xilinx_gsclib_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_xor(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "XOR2X1")
            result["Y"] = std::make_shared<bdd>(PIN_TO_BDD("A") ^ PIN_TO_BDD("B"));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_and(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "AND2X1")
            result["Y"] = std::make_shared<bdd>(PIN_TO_BDD("A") & PIN_TO_BDD("B"));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_andoriv(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "AOI21X1")
            result["Y"] = std::make_shared<bdd>(bdd_not((PIN_TO_BDD("A0") & PIN_TO_BDD("A1")) | PIN_TO_BDD("B0")));
        if (g->get_type() == "AOI22X1")
            result["Y"] = std::make_shared<bdd>(bdd_not((PIN_TO_BDD("A0") & PIN_TO_BDD("A1")) | (PIN_TO_BDD("B0") & PIN_TO_BDD("B1"))));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_inv(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        std::set<std::string> gate_types = {
            "INVX1",
            "INVX2",
            "INVX4",
            "INVX8",
            "TINVX1",
        };
        if (gate_types.find(g->get_type()) != gate_types.end())
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_mx2x1(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "MX2X1")
            result["Y"] = std::make_shared<bdd>((PIN_TO_BDD("S0") & PIN_TO_BDD("B")) | (bdd_not(PIN_TO_BDD("S0")) & PIN_TO_BDD("A")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_nand(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if ((g->get_type() == "NAND2X1") || (g->get_type() == "NAND2X2"))
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A") & PIN_TO_BDD("B")));
        if (g->get_type() == "NAND3X1")
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A") & PIN_TO_BDD("B") & PIN_TO_BDD("C")));
        if (g->get_type() == "NAND4X1")
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A") & PIN_TO_BDD("B") & PIN_TO_BDD("C") & PIN_TO_BDD("D")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_nor(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "NOR2X1")
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A") | PIN_TO_BDD("B")));
        if (g->get_type() == "NOR3X1")
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A") | PIN_TO_BDD("B") | PIN_TO_BDD("C")));
        if (g->get_type() == "NOR4X1")
            result["Y"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("A") | PIN_TO_BDD("B") | PIN_TO_BDD("C") | PIN_TO_BDD("D")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_oai(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "OAI21X1")
            result["Y"] = std::make_shared<bdd>(bdd_not((PIN_TO_BDD("A0") | PIN_TO_BDD("A1")) & PIN_TO_BDD("B0")));
        if (g->get_type() == "OAI22X1")
            result["Y"] = std::make_shared<bdd>(bdd_not((PIN_TO_BDD("A0") | PIN_TO_BDD("A1")) & (PIN_TO_BDD("B0") | PIN_TO_BDD("B1"))));
        if (g->get_type() == "OAI33X1")
            result["Y"] = std::make_shared<bdd>(bdd_not((PIN_TO_BDD("A0") | PIN_TO_BDD("A1") | PIN_TO_BDD("A2")) & (PIN_TO_BDD("B0") | PIN_TO_BDD("B1") | PIN_TO_BDD("B2"))));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_or(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "OR2X1")
            result["Y"] = std::make_shared<bdd>(PIN_TO_BDD("A") | PIN_TO_BDD("B"));
        if (g->get_type() == "OR4X1")
            result["Y"] = std::make_shared<bdd>(PIN_TO_BDD("A") | PIN_TO_BDD("B") | PIN_TO_BDD("C") | PIN_TO_BDD("D"));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_gsclib_add(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "ADDHX1")
        {
            result["CO"] = std::make_shared<bdd>(PIN_TO_BDD("A") & PIN_TO_BDD("B"));
            result["S"]  = std::make_shared<bdd>(PIN_TO_BDD("A") ^ PIN_TO_BDD("B"));
        }
        if (g->get_type() == "ADDFX1")
        {
            result["CO"] = std::make_shared<bdd>((PIN_TO_BDD("A") & PIN_TO_BDD("B")) | (PIN_TO_BDD("B") & PIN_TO_BDD("CI")) | (PIN_TO_BDD("A")));
            result["S"]  = std::make_shared<bdd>(PIN_TO_BDD("A") ^ PIN_TO_BDD("B") ^ PIN_TO_BDD("CI"));
        }
        return result;
    }
}    // namespace bdd_xilinx_gsclib_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators = {
    {"ADDFX1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_add},      {"ADDHX1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_add},      {"AND2X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_and},
    {"AOI21X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_andoriv}, {"AOI22X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_andoriv}, {"INVX1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_inv},
    {"INVX2", bdd_xilinx_gsclib_helper::get_bdd_gsclib_inv},       {"INVX4", bdd_xilinx_gsclib_helper::get_bdd_gsclib_inv},       {"INVX8", bdd_xilinx_gsclib_helper::get_bdd_gsclib_inv},
    {"MX2X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_mx2x1},     {"NAND2X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_nand},    {"NAND2X2", bdd_xilinx_gsclib_helper::get_bdd_gsclib_nand},
    {"NAND3X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_nand},    {"NAND4X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_nand},    {"NOR2X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_nor},
    {"NOR3X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_nor},      {"NOR4X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_nor},      {"OAI21X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_oai},
    {"OAI22X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_oai},     {"OAI33X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_oai},     {"OR2X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_or},
    {"OR4X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_or},        {"TINVX1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_inv},      {"XOR2X1", bdd_xilinx_gsclib_helper::get_bdd_gsclib_xor},
};


std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_gsclib(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

bool plugin_gate_decorators::bdd_availability_tester_gsclib(std::shared_ptr<gate> g)
{
    auto type = g->get_type();
    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}