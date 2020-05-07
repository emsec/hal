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

gate::gate(std::shared_ptr<netlist> const g, const u32 id, std::shared_ptr<const gate_type> gt, const std::string& name, float x, float y)
{
    assert(g != nullptr);
    m_netlist = g;
    m_id      = id;
    m_type    = gt;
    m_name    = name;
    m_x       = x;
    m_y       = y;
}

u32 gate::get_id() const
{
    return m_id;
}

std::shared_ptr<netlist> gate::get_netlist() const
{
    return m_netlist;
}

std::string gate::get_name() const
{
    return m_name;
}

void gate::set_name(const std::string& name)
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

std::shared_ptr<const gate_type> gate::get_type() const
{
    return m_type;
}

float gate::get_location_x() const
{
    return m_x;
}

float gate::get_location_y() const
{
    return m_y;
}

std::pair<float, float> gate::get_location() const
{
    return {m_x, m_y};
}

bool gate::has_location() const
{
    return m_x >= 0 && m_y >= 0;
}

void gate::set_location_x(float x)
{
    if (x != m_x)
    {
        m_x = x;
        gate_event_handler::notify(gate_event_handler::event::location_changed, shared_from_this());
    }
}

void gate::set_location_y(float y)
{
    if (y != m_y)
    {
        m_y = y;
        gate_event_handler::notify(gate_event_handler::event::location_changed, shared_from_this());
    }
}

void gate::set_location(const std::pair<float, float>& location)
{
    set_location_x(location.first);
    set_location_y(location.second);
}

std::shared_ptr<module> gate::get_module() const
{
    return m_module;
}

boolean_function gate::get_boolean_function(std::string name) const
{
    if (name.empty())
    {
        auto output_pins = m_type->get_output_pins();
        if (output_pins.empty())
        {
            return boolean_function();
        }
        name = output_pins[0];
    }

    if (m_type->get_base_type() == gate_type::base_type::lut)
    {
        auto lut_type = std::static_pointer_cast<const gate_type_lut>(m_type);
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

    return boolean_function();
}

std::unordered_map<std::string, boolean_function> gate::get_boolean_functions(bool only_custom_functions) const
{
    std::unordered_map<std::string, boolean_function> res;

    if (!only_custom_functions)
    {
        res = m_type->get_boolean_functions();
    }

    for (const auto& it : m_functions)
    {
        res.emplace(it.first, it.second);
    }

    if (!only_custom_functions && m_type->get_base_type() == gate_type::base_type::lut)
    {
        auto lut_type = std::static_pointer_cast<const gate_type_lut>(m_type);
        for (auto pin : lut_type->get_output_from_init_string_pins())
        {
            res.emplace(pin, get_lut_function(pin));
        }
    }

    return res;
}

boolean_function gate::get_lut_function(const std::string& pin) const
{
    UNUSED(pin);

    auto lut_type = std::static_pointer_cast<const gate_type_lut>(m_type);

    std::string category   = lut_type->get_config_data_category();
    std::string key        = lut_type->get_config_data_identifier();
    std::string config_str = std::get<1>(get_data_by_key(category, key));
    auto is_ascending      = lut_type->is_config_data_ascending_order();
    auto inputs            = get_input_pins();

    boolean_function result = boolean_function::ZERO;

    if (config_str.empty())
    {
        return result;
    }

    if (inputs.size() > 6)
    {
        log_error("netlist.internal", "LUT-gate '{}' (id = {}) has more than six input pins (unsupported)", get_name(), get_id());
        return boolean_function();
    }

    u32 config_size = 1 << inputs.size();

    if (config_str.size() > config_size / 4)
    {
        log_error("netlist.internal",
                  "LUT-gate '{}' (id = {}) supports a config of up to {} bits, but config string {} contains {} bits",
                  get_name(),
                  get_id(),
                  config_size,
                  config_str,
                  config_str.size() * 4);
        return boolean_function();
    }

    u64 config = 0;
    try
    {
        config = std::stoull(config_str, nullptr, 16);
    }
    catch (std::invalid_argument& ex)
    {
        log_error("netlist.internal", "LUT-gate '{}' (id = {}) has invalid config string: '{}' is not a hex value", get_name(), get_id(), config_str);
        return boolean_function();
    }

    for (u32 i = 0; config != 0 && i < config_size; i++)
    {
        u8 bit;
        if (is_ascending)
        {
            bit = (config >> (config_size - 1)) & 1;
            config <<= 1;
        }
        else
        {
            bit = (config & 1);
            config >>= 1;
        }
        if (bit == 1)
        {
            boolean_function clause;
            auto input_values = i;
            for (auto input : inputs)
            {
                if ((input_values & 1) == 1)
                {
                    clause &= boolean_function(input);
                }
                else
                {
                    clause &= !boolean_function(input);
                }
                input_values >>= 1;
            }
            result |= clause;
        }
    }

    return result.optimize();
}

void gate::add_boolean_function(const std::string& name, const boolean_function& func)
{
    if (m_type->get_base_type() == gate_type::base_type::lut)
    {
        auto output_pins = m_type->get_output_pins();
        if (!output_pins.empty() && name == output_pins[0])
        {
            auto lut_type = std::static_pointer_cast<const gate_type_lut>(m_type);
            auto tt       = func.get_truth_table(get_input_pins());

            u64 config_value = 0;
            if (lut_type->is_config_data_ascending_order())
            {
                std::reverse(tt.begin(), tt.end());
            }
            for (auto v : tt)
            {
                if (v == boolean_function::X)
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

bool gate::mark_vcc_gate()
{
    return m_netlist->mark_vcc_gate(shared_from_this());
}

bool gate::mark_gnd_gate()
{
    return m_netlist->mark_gnd_gate(shared_from_this());
}

bool gate::unmark_vcc_gate()
{
    return m_netlist->unmark_vcc_gate(shared_from_this());
}

bool gate::unmark_gnd_gate()
{
    return m_netlist->unmark_gnd_gate(shared_from_this());
}

bool gate::is_vcc_gate() const
{
    return m_netlist->is_vcc_gate(const_cast<gate*>(this)->shared_from_this());
}

bool gate::is_gnd_gate() const
{
    return m_netlist->is_gnd_gate(const_cast<gate*>(this)->shared_from_this());
}

std::vector<std::string> gate::get_input_pins() const
{
    return m_type->get_input_pins();
}

std::vector<std::string> gate::get_output_pins() const
{
    return m_type->get_output_pins();
}

std::set<std::shared_ptr<net>> gate::get_fan_in_nets() const
{
    std::set<std::shared_ptr<net>> nets;

    for (const auto& it : m_in_nets)
    {
        nets.insert(it.second);
    }

    return nets;
}

std::shared_ptr<net> gate::get_fan_in_net(const std::string& pin_type) const
{
    auto it = m_in_nets.find(pin_type);

    if (it == m_in_nets.end())
    {
        log_debug("netlist.internal", "gate ('{}',  type = {}) has no net connected to input pin '{}'.", get_name(), get_type()->get_name(), pin_type);
        return nullptr;
    }

    return it->second;
}

std::set<std::shared_ptr<net>> gate::get_fan_out_nets() const
{
    std::set<std::shared_ptr<net>> nets;

    for (const auto& it : m_out_nets)
    {
        nets.insert(it.second);
    }

    return nets;
}

std::shared_ptr<net> gate::get_fan_out_net(const std::string& pin_type) const
{
    auto it = m_out_nets.find(pin_type);

    if (it == m_out_nets.end())
    {
        log_debug("netlist.internal", "gate ('{}',  type = {}) has no net connected to output pin '{}'.", get_name(), get_type()->get_name(), pin_type);
        return nullptr;
    }

    return it->second;
}

std::set<endpoint> gate::get_unique_predecessors(const std::function<bool(const std::string& starting_pin, const endpoint&)>& filter) const
{
    auto predecessors = this->get_predecessors(filter);
    return std::set<endpoint>(predecessors.begin(), predecessors.end());
}

std::vector<endpoint> gate::get_predecessors(const std::function<bool(const std::string& starting_pin, const endpoint&)>& filter) const
{
    std::vector<endpoint> result;
    for (const auto& it : m_in_nets)
    {
        auto& net = it.second;
        auto& pin = it.first;
        auto pred = net->get_src();
        if (pred.gate == nullptr)
        {
            log_debug("netlist", "predecessor on pin '{}' of gate '{}' (id = {:08x}) is unrouted.", pin, this->get_name(), this->get_id());
            continue;
        }
        if (filter && !filter(pin, pred))
        {
            continue;
        }
        result.push_back(pred);
    }
    return result;
}

endpoint gate::get_predecessor(const std::string& which_pin) const
{
    auto predecessors = this->get_predecessors([&which_pin](auto& starting_pin, auto&) -> bool { return starting_pin == which_pin; });
    if (predecessors.size() == 0)
    {
        return {nullptr, ""};
    }
    if (predecessors.size() > 1)
    {
        log_error("netlist", "internal error: multiple predecessors for '{}' at pin '{}'.", get_name(), which_pin);
        return {nullptr, ""};
    }

    return predecessors[0];
}

std::set<endpoint> gate::get_unique_successors(const std::function<bool(const std::string& starting_pin, const endpoint&)>& filter) const
{
    auto successors = this->get_successors(filter);
    return std::set<endpoint>(successors.begin(), successors.end());
}

std::vector<endpoint> gate::get_successors(const std::function<bool(const std::string& starting_pin, const endpoint&)>& filter) const
{
    std::vector<endpoint> result;
    for (const auto& it : m_out_nets)
    {
        auto& pin       = it.first;
        auto& net       = it.second;
        auto successors = net->get_dsts();
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
