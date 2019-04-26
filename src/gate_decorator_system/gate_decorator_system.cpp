#include "gate_decorator_system/gate_decorator_system.h"
#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/netlist.h"

#include <hal_bdd.h>

namespace gate_decorator_system
{
    namespace
    {
        struct helper
        {
            helper()
            {
                log_manager::get_instance().add_channel("netlist.decorator", {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");

                /* initialize BUDDY */
                int gate_size = 1000000, cache_size = 1000000;
                if (bdd_init(gate_size, cache_size) < 0)
                    log_critical("netlist.decorator", "cannot initialize buddy library.");

                int num_of_variables = 10000;
                if (bdd_setvarnum(num_of_variables) < 0)
                    log_critical("netlist.decorator", "cannot set the number of varibles in the buddy library.");

                int max_increase = 50000;
                if (bdd_setmaxincrease(max_increase) < 0)
                    log_critical("netlist.decorator", "cannot set max_increase in the buddy library.");
            }
            ~helper()
            {
                /* cleanup BUDDY */
                bdd_done();
            }
        } instance;

        std::map<std::string, std::tuple<decorator_availability_tester, bdd_decorator_generator>> m_bdd_generators;
        std::map<std::string, std::tuple<decorator_availability_tester, lut_decorator_generator>> m_lut_generators;
    }    // namespace

    void register_bdd_decorator_function(const std::string& gate_library, decorator_availability_tester tester, bdd_decorator_generator generator)
    {
        if (m_bdd_generators.find(gate_library) != m_bdd_generators.end())
        {
            log_error("netlist.decorator", "bdd generators already registered for gate library '{}'.", gate_library);
            return;
        }
        m_bdd_generators[gate_library] = std::make_tuple(tester, generator);
        log_debug("netlist.decorator", "registered bdd gate decorator for '{}'.", gate_library);
    }

    void register_lut_decorator_function(const std::string& gate_library, decorator_availability_tester tester, lut_decorator_generator generator)
    {
        if (m_lut_generators.find(gate_library) != m_lut_generators.end())
        {
            log_error("netlist.decorator", "lut generators already registered for gate library '{}'.", gate_library);
            return;
        }
        m_lut_generators[gate_library] = std::make_tuple(tester, generator);
        log_debug("netlist.decorator", "registered lut gate decorator for '{}'.", gate_library);
    }

    void remove_bdd_decorator_function(const std::string& gate_library)
    {
        auto it = m_bdd_generators.find(gate_library);
        if (it != m_bdd_generators.end())
        {
            m_bdd_generators.erase(it);
            log_debug("netlist.decorator", "removed bdd gate decorator for '{}'.", gate_library);
        }
    }

    void remove_lut_decorator_function(const std::string& gate_library)
    {
        auto it = m_lut_generators.find(gate_library);
        if (it != m_lut_generators.end())
        {
            m_lut_generators.erase(it);
            log_debug("netlist.decorator", "removed lut gate decorator for '{}'.", gate_library);
        }
    }

    std::shared_ptr<gate_decorator> query_decorator(std::shared_ptr<gate> g, const decorator_type& type)
    {
        auto lib_name = g->get_netlist()->get_gate_library()->get_name();
        if (type == decorator_type::BDD)
        {
            auto it = m_bdd_generators.find(lib_name);
            if (it == m_bdd_generators.end())
            {
                return nullptr;
            }
            auto [tester, generator] = it->second;
            if (tester(g))
            {
                return std::make_shared<gate_decorator_bdd>(g, generator);
            }
        }
        else if (type == decorator_type::LUT)
        {
            auto it = m_lut_generators.find(lib_name);
            if (it == m_lut_generators.end())
            {
                return nullptr;
            }
            auto [tester, generator] = it->second;
            if (tester(g))
            {
                auto [inputs, outputs, config] = generator(g);
                return std::make_shared<gate_decorator_lut>(g, inputs, outputs, config);
            }
        }
        return nullptr;
    }

    bool has_decorator_type(std::shared_ptr<gate> g, const decorator_type& type)
    {
        auto lib_name = g->get_netlist()->get_gate_library()->get_name();
        if (type == decorator_type::BDD)
        {
            auto it = m_bdd_generators.find(lib_name);
            if (it == m_bdd_generators.end())
            {
                return false;
            }
            auto tester = std::get<0>(it->second);
            return tester(g);
        }
        else if (type == decorator_type::LUT)
        {
            auto it = m_lut_generators.find(lib_name);
            if (it == m_lut_generators.end())
            {
                return false;
            }
            auto tester = std::get<0>(it->second);
            return tester(g);
        }
        return false;
    }

}    // namespace gate_decorator_system