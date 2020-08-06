#include "netlist/gate.h"

#include "core/log.h"
#include "netlist/event_system/gate_event_handler.h"
#include "netlist/gate_library/gate_type/gate_type_lut.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <assert.h>
#include <iomanip>
#include <sstream>

namespace hal
{
    Gate::Gate(std::shared_ptr<Netlist> const g, const u32 id, const GateType* gt, const std::string& name, float x, float y)
    {
        assert(g != nullptr);
        m_netlist = g;
        m_id      = id;
        m_type    = gt;
        m_name    = name;
        m_x       = x;
        m_y       = y;
    }

    u32 Gate::get_id() const
    {
        return m_id;
    }

    std::shared_ptr<Netlist> Gate::get_netlist() const
    {
        return m_netlist;
    }

    std::string Gate::get_name() const
    {
        return m_name;
    }

    void Gate::set_name(const std::string& name)
    {
        if (core_utils::trim(name).empty())
        {
            log_error("netlist.internal", "gate::set_name: empty name is not allowed");
            return;
        }
        if (name != m_name)
        {
            log_info("netlist.internal", "changed name for gate (id = {}, type = {}) from '{}' to '{}'.", m_id, m_type->get_name(), m_name, name);

            m_name = name;

            gate_event_handler::notify(gate_event_handler::event::name_changed, shared_from_this());
        }
    }

    const GateType* Gate::get_type() const
    {
        return m_type;
    }

    float Gate::get_location_x() const
    {
        return m_x;
    }

    float Gate::get_location_y() const
    {
        return m_y;
    }

    std::pair<float, float> Gate::get_location() const
    {
        return {m_x, m_y};
    }

    bool Gate::has_location() const
    {
        return m_x >= 0 && m_y >= 0;
    }

    void Gate::set_location_x(float x)
    {
        if (x != m_x)
        {
            m_x = x;
            gate_event_handler::notify(gate_event_handler::event::location_changed, shared_from_this());
        }
    }

    void Gate::set_location_y(float y)
    {
        if (y != m_y)
        {
            m_y = y;
            gate_event_handler::notify(gate_event_handler::event::location_changed, shared_from_this());
        }
    }

    void Gate::set_location(const std::pair<float, float>& location)
    {
        set_location_x(location.first);
        set_location_y(location.second);
    }

    std::shared_ptr<Module> Gate::get_module() const
    {
        return m_module;
    }

    BooleanFunction Gate::get_boolean_function(std::string name) const
    {
        if (name.empty())
        {
            auto output_pins = m_type->get_output_pins();
            if (output_pins.empty())
            {
                return BooleanFunction();
            }
            name = output_pins[0];
        }

        if (m_type->get_base_type() == GateType::BaseType::lut)
        {
            auto lut_type = static_cast<const GateTypeLut*>(m_type);
            auto lut_pins = lut_type->get_output_from_init_string_pins();
            if (lut_pins.find(name) != lut_pins.end())
            {
                return get_lut_function(name);
            }
        }

        if (auto it = m_functions.find(name); it != m_functions.end())
        {
            return it->second;
        }

        auto map = m_type->get_boolean_functions();
        if (auto it = map.find(name); it != map.end())
        {
            return it->second;
        }

        return BooleanFunction();
    }

    std::unordered_map<std::string, BooleanFunction> Gate::get_boolean_functions(bool only_custom_functions) const
    {
        std::unordered_map<std::string, BooleanFunction> res;

        if (!only_custom_functions)
        {
            res = m_type->get_boolean_functions();
        }

        for (const auto& it : m_functions)
        {
            res.emplace(it.first, it.second);
        }

        if (!only_custom_functions && m_type->get_base_type() == GateType::BaseType::lut)
        {
            auto lut_type = static_cast<const GateTypeLut*>(m_type);
            for (auto pin : lut_type->get_output_from_init_string_pins())
            {
                res.emplace(pin, get_lut_function(pin));
            }
        }

        return res;
    }

    BooleanFunction Gate::get_lut_function(const std::string& pin) const
    {
        UNUSED(pin);

        auto lut_type = static_cast<const GateTypeLut*>(m_type);

        std::string category   = lut_type->get_config_data_category();
        std::string key        = lut_type->get_config_data_identifier();
        std::string config_str = std::get<1>(get_data_by_key(category, key));
        auto is_ascending      = lut_type->is_config_data_ascending_order();
        auto inputs            = get_input_pins();

        BooleanFunction result = BooleanFunction::ZERO;

        if (config_str.empty())
        {
            return result;
        }

        if (inputs.size() > 6)
        {
            log_error("netlist.internal", "{}-gate '{}' (id = {}) has more than six input pins (unsupported)", get_type()->get_name(), get_name(), get_id());
            return BooleanFunction();
        }

        u64 config = 0;
        try
        {
            config = std::stoull(config_str, nullptr, 16);
        }
        catch (std::invalid_argument& ex)
        {
            log_error("netlist.internal", "{}-gate '{}' (id = {}) has invalid config string: '{}' is not a hex value", get_type()->get_name(), get_name(), get_id(), config_str);
            return BooleanFunction();
        }
        u32 config_size = 0;
        {
            u64 tmp = config;
            while (tmp != 0)
            {
                config_size++;
                tmp >>= 1;
            }
        }

        u32 max_config_size = 1 << inputs.size();

        if (config_size > max_config_size)
        {
            log_error("netlist.internal",
                      "{}-gate '{}' (id = {}) supports a config of up to {} bits, but config string {} contains {} bits.",
                      get_type()->get_name(),
                      get_name(),
                      get_id(),
                      max_config_size,
                      config_str,
                      config_str.size() * 4);
            return BooleanFunction();
        }

        for (u32 i = 0; config != 0 && i < max_config_size; i++)
        {
            u8 bit;
            if (is_ascending)
            {
                bit = (config >> (max_config_size - 1)) & 1;
                config <<= 1;
            }
            else
            {
                bit = (config & 1);
                config >>= 1;
            }
            if (bit == 1)
            {
                BooleanFunction clause;
                auto input_values = i;
                for (auto input : inputs)
                {
                    if ((input_values & 1) == 1)
                    {
                        clause &= BooleanFunction(input);
                    }
                    else
                    {
                        clause &= !BooleanFunction(input);
                    }
                    input_values >>= 1;
                }
                result |= clause;
            }
        }

        return result.optimize();
    }

    void Gate::add_boolean_function(const std::string& name, const BooleanFunction& func)
    {
        if (m_type->get_base_type() == GateType::BaseType::lut)
        {
            auto output_pins = m_type->get_output_pins();
            if (!output_pins.empty() && name == output_pins[0])
            {
                auto lut_type = static_cast<const GateTypeLut*>(m_type);
                auto tt       = func.get_truth_table(get_input_pins());

                u64 config_value = 0;
                if (lut_type->is_config_data_ascending_order())
                {
                    std::reverse(tt.begin(), tt.end());
                }
                for (auto v : tt)
                {
                    if (v == BooleanFunction::X)
                    {
                        log_error("netlist", "function truth table contained undefined values");
                        return;
                    }
                    config_value <<= 1;
                    config_value |= v;
                }

                std::string category = lut_type->get_config_data_category();
                std::string key      = lut_type->get_config_data_identifier();

                std::stringstream stream;
                stream << std::hex << config_value;
                set_data(category, key, "bit_vector", stream.str());

                return;
            }
        }

        m_functions.emplace(name, func);
    }

    bool Gate::mark_vcc_gate()
    {
        return m_netlist->mark_vcc_gate(shared_from_this());
    }

    bool Gate::mark_gnd_gate()
    {
        return m_netlist->mark_gnd_gate(shared_from_this());
    }

    bool Gate::unmark_vcc_gate()
    {
        return m_netlist->unmark_vcc_gate(shared_from_this());
    }

    bool Gate::unmark_gnd_gate()
    {
        return m_netlist->unmark_gnd_gate(shared_from_this());
    }

    bool Gate::is_vcc_gate() const
    {
        return m_netlist->is_vcc_gate(const_cast<Gate*>(this)->shared_from_this());
    }

    bool Gate::is_gnd_gate() const
    {
        return m_netlist->is_gnd_gate(const_cast<Gate*>(this)->shared_from_this());
    }

    std::vector<std::string> Gate::get_input_pins() const
    {
        return m_type->get_input_pins();
    }

    std::vector<std::string> Gate::get_output_pins() const
    {
        return m_type->get_output_pins();
    }

    std::set<std::shared_ptr<Net>> Gate::get_fan_in_nets() const
    {
        std::set<std::shared_ptr<Net>> nets;

        for (const auto& it : m_in_nets)
        {
            nets.insert(it.second);
        }

        return nets;
    }

    std::shared_ptr<Net> Gate::get_fan_in_net(const std::string& pin_type) const
    {
        auto it = m_in_nets.find(pin_type);

        if (it == m_in_nets.end())
        {
            log_debug("netlist.internal", "gate ('{}',  type = {}) has no net connected to input pin '{}'.", get_name(), get_type()->get_name(), pin_type);
            return nullptr;
        }

        return it->second;
    }

    std::set<std::shared_ptr<Net>> Gate::get_fan_out_nets() const
    {
        std::set<std::shared_ptr<Net>> nets;

        for (const auto& it : m_out_nets)
        {
            nets.insert(it.second);
        }

        return nets;
    }

    std::shared_ptr<Net> Gate::get_fan_out_net(const std::string& pin_type) const
    {
        auto it = m_out_nets.find(pin_type);

        if (it == m_out_nets.end())
        {
            log_debug("netlist.internal", "gate ('{}',  type = {}) has no net connected to output pin '{}'.", get_name(), get_type()->get_name(), pin_type);
            return nullptr;
        }

        return it->second;
    }

    std::vector<std::shared_ptr<Gate>> Gate::get_unique_predecessors(const std::function<bool(const std::string& starting_pin, const Endpoint&)>& filter) const
    {
        std::unordered_set<std::shared_ptr<Gate>> res;
        auto endpoints = this->get_predecessors(filter);
        res.reserve(endpoints.size());
        for (const auto& ep : endpoints)
        {
            res.insert(ep.get_gate());
        }
        return std::vector<std::shared_ptr<Gate>>(res.begin(), res.end());
    }

    std::vector<Endpoint> Gate::get_predecessors(const std::function<bool(const std::string& starting_pin, const Endpoint&)>& filter) const
    {
        std::vector<Endpoint> result;
        for (const auto& it : m_in_nets)
        {
            auto& pin         = it.first;
            auto& net         = it.second;
            auto predecessors = net->get_sources();
            if (!filter)
            {
                result.insert(result.end(), predecessors.begin(), predecessors.end());
            }
            else
            {
                for (const auto& pre : predecessors)
                {
                    if (!filter(pin, pre))
                    {
                        continue;
                    }
                    result.push_back(pre);
                }
            }
        }
        return result;
    }

    Endpoint Gate::get_predecessor(const std::string& which_pin) const
    {
        auto predecessors = this->get_predecessors([&which_pin](auto& starting_pin, auto&) -> bool { return starting_pin == which_pin; });
        if (predecessors.size() == 0)
        {
            return Endpoint(nullptr, "", false);
        }
        if (predecessors.size() > 1)
        {
            log_error("netlist", "internal error: multiple predecessors for '{}' at pin '{}'.", get_name(), which_pin);
            return Endpoint(nullptr, "", false);
        }

        return predecessors[0];
    }

    std::vector<std::shared_ptr<Gate>> Gate::get_unique_successors(const std::function<bool(const std::string& starting_pin, const Endpoint&)>& filter) const
    {
        std::unordered_set<std::shared_ptr<Gate>> res;
        auto endpoints = this->get_successors(filter);
        res.reserve(endpoints.size());
        for (const auto& ep : endpoints)
        {
            res.insert(ep.get_gate());
        }
        return std::vector<std::shared_ptr<Gate>>(res.begin(), res.end());
    }

    std::vector<Endpoint> Gate::get_successors(const std::function<bool(const std::string& starting_pin, const Endpoint&)>& filter) const
    {
        std::vector<Endpoint> result;
        for (const auto& it : m_out_nets)
        {
            auto& pin       = it.first;
            auto& net       = it.second;
            auto successors = net->get_destinations();
            if (!filter)
            {
                result.insert(result.end(), successors.begin(), successors.end());
            }
            else
            {
                for (const auto& suc : successors)
                {
                    if (!filter(pin, suc))
                    {
                        continue;
                    }
                    result.push_back(suc);
                }
            }
        }
        return result;
    }
}    // namespace hal
