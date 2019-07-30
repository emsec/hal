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

namespace bdd_SCAN_FF_LIB_helper
{
    

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_SCAN_FF_LIB_vcc(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_VCC")
            result["O"] = std::make_shared<bdd>(bdd_true());
        if (g->get_type() == "VCC")
            result["P"] = std::make_shared<bdd>(bdd_true());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_SCAN_FF_LIB_gnd(std::shared_ptr<gate> g, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "GLOBAL_GND")
            result["O"] = std::make_shared<bdd>(bdd_false());
        if (g->get_type() == "GND")
            result["G"] = std::make_shared<bdd>(bdd_false());

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_SCAN_FF_LIB_inv(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (g->get_type() == "inv")
            result["O"] = std::make_shared<bdd>(bdd_not(PIN_TO_BDD("I")));

        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_SCAN_FF_LIB_and(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;       
        result["O"] = std::make_shared<bdd>(bdd_true());

        for (const auto pin : g->get_input_pin_types())
        {
            *result["O"] &= PIN_TO_BDD(pin);
        }
       
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_SCAN_FF_LIB_or(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        result["O"] = std::make_shared<bdd>(bdd_false());

        for (const auto pin : g->get_input_pin_types())
        {
            *result["O"] |= PIN_TO_BDD(pin);
        }

        return result;
    }

 
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_SCAN_FF_LIB_buf(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if ((g->get_type() == "BUFG") || (g->get_type() == "IBUF") || (g->get_type() == "OBUF"))
        {
            result["O"] = std::make_shared<bdd>(PIN_TO_BDD("I"));
        }
        return result;
    }
}    // namespace bdd_SCAN_FF_LIB_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators =
    {
        {"GLOBAL_VCC", bdd_SCAN_FF_LIB_helper::get_bdd_SCAN_FF_LIB_vcc}, {"VCC", bdd_SCAN_FF_LIB_helper::get_bdd_SCAN_FF_LIB_vcc},
        {"GLOBAL_GND", bdd_SCAN_FF_LIB_helper::get_bdd_SCAN_FF_LIB_gnd}, {"GND", bdd_SCAN_FF_LIB_helper::get_bdd_SCAN_FF_LIB_gnd},

        {"inv", bdd_SCAN_FF_LIB_helper::get_bdd_SCAN_FF_LIB_inv},
};

std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_scan_ff_lib(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
{
    auto type = g->get_type();

    gate_decorator_system::bdd_decorator_generator generator;

    if (type.find("and") != std::string::npos)
    {
        generator = bdd_SCAN_FF_LIB_helper::get_bdd_SCAN_FF_LIB_and;
    }
    else if (type.find("or") != std::string::npos)
    {
        generator = bdd_SCAN_FF_LIB_helper::get_bdd_SCAN_FF_LIB_or;
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

bool plugin_gate_decorators::bdd_availability_tester_scan_ff_lib(std::shared_ptr<gate> g)
{
    auto type = g->get_type();

    if (type.find("and") != std::string::npos)
    {
        return true;
    }

    if (type.find("or") != std::string::npos)
    {
        return true;
    }
    
    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}

