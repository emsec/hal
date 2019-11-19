#include "netlist/gate.h"

#include "netlist/gate_library/gate_type/gate_type_lut.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/event_system/gate_event_handler.h"

#include "core/log.h"

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

std::ostream& operator<<(std::ostream& os, const gate& gate)
{
    os << "\t\'" << gate.get_name() << "\'"
       << " (type = '" << gate.get_type()->get_name() << "', id = " << gate.get_id() << ")" << std::endl;
    for (const auto& input_pin_type : gate.get_input_pins())
    {
        os << "\t\t" << input_pin_type << " => ";
        auto net = gate.get_fan_in_net(input_pin_type);
        if (net == nullptr)
            os << "nullptr";
        else
            os << net->get_name();
        os << std::endl;
    }
    for (const auto& output_pin_type : gate.get_output_pins())
    {
        os << "\t\t" << output_pin_type << " => ";
        auto net = gate.get_fan_out_net(output_pin_type);
        if (net == nullptr)
            os << "nullptr";
        else
            os << net->get_name();
        os << std::endl;
    }

    return os;
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

boolean_function gate::get_boolean_function(const std::string& name) const
{
    if (name.empty())
    {
        auto output_pins = m_type->get_output_pins();
        if (!output_pins.empty())
        {
            return get_boolean_function(output_pins[0]);
        }
        return boolean_function::X;
    }

    if (m_type->get_base_type() == gate_type::base_type::lut && name == m_type->get_output_pins()[0])
    {
        return get_lut_function();
    }

    auto it = m_functions.find(name);
    if (it != m_functions.end())
    {
        return it->second;
    }
    auto map = m_type->get_boolean_functions();
    it       = m_functions.find(name);
    if (it != m_functions.end())
    {
        return it->second;
    }
    return boolean_function::X;
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

    if (m_type->get_base_type() == gate_type::base_type::lut)
    {
        res.emplace(get_output_pins()[0], get_lut_function());
    }

    return res;
}

boolean_function gate::get_lut_function() const
{
    auto lut_type = std::static_pointer_cast<const gate_type_lut>(m_type);

    std::string category   = lut_type->get_data_category();
    std::string key        = lut_type->get_data_identifier();
    std::string config_str = std::get<1>(get_data_by_key(category, key));

    if (config_str.empty())
    {
        return boolean_function::ZERO;
    }
    u64 config = std::stoull(config_str, nullptr, 16);

    boolean_function result;

    for (u32 i = 0; config != 0; i++)
    {
        u8 bit;
        if (lut_type->is_ascending_order())
        {
            bit = (config & 1);
            config >>= 1;
        }
        else
        {
            bit = config >> 63;
            config <<= 1;
        }
        if (bit == 1)
        {
            boolean_function clause;
            auto input_values = i;
            for (auto input : get_input_pins())
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

void gate::set_boolean_function(const std::string& name, const boolean_function& func)
{
    if (m_type->get_base_type() == gate_type::base_type::lut)
    {
        auto output_pins = m_type->get_output_pins();
        if (!output_pins.empty() && name == output_pins[0])
        {
            auto lut_type = std::static_pointer_cast<const gate_type_lut>(m_type);
            auto tt       = func.get_truth_table(get_input_pins());

            u64 config_value = 0;
            if (lut_type->is_ascending_order())
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
                config_value |= v;
                config_value <<= 1;
            }

            std::string category = lut_type->get_data_category();
            std::string key      = lut_type->get_data_identifier();

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
        log_debug("netlist.internal", "gate ('{},  type = {}) has no net connected to input pin '{}'.", get_name(), get_type()->get_name(), pin_type);
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
        log_debug("netlist.internal", "gate ('{},  type = {}) has no net connected to output pin '{}'.", get_name(), get_type()->get_name(), pin_type);
        return nullptr;
    }

    return it->second;
}

std::set<endpoint> gate::get_unique_predecessors(const std::string& this_pin_type_filter, const std::string& pred_pin_type_filter, const std::string& gate_type_filter) const
{
    auto predecessors = this->get_predecessors(this_pin_type_filter, pred_pin_type_filter, gate_type_filter);
    return std::set<endpoint>(predecessors.begin(), predecessors.end());
}

std::vector<endpoint> gate::get_predecessors(const std::string& this_pin_type_filter, const std::string& pred_pin_type_filter, const std::string& gate_type_filter) const
{
    if (this_pin_type_filter != DONT_CARE)
    {
        auto it = m_in_nets.find(this_pin_type_filter);
        if (it == m_in_nets.end())
        {
            return {};
        }
        auto net  = it->second;
        auto pred = net->get_src();
        if (pred.gate == nullptr)
        {
            log_debug("netlist", "predecessor on pin '{}' of gate '{}' (id = {:08x}) is unrouted.", this_pin_type_filter, this->get_name(), this->get_id());
            return {};
        }
        if ((pred_pin_type_filter != DONT_CARE) && (pred.pin_type != pred_pin_type_filter))
        {
            log_debug("netlist", "pin type of predecessor gate '{}' (id = {:08x}) does not match", pred.gate->get_name(), pred.gate->get_id());
            return {};
        }
        if ((gate_type_filter != DONT_CARE) && (pred.gate->get_type()->get_name() != gate_type_filter))
        {
            log_debug("netlist", "type of predecessor gate '{}' (id = {:08x}) does not match", pred.gate->get_name(), pred.gate->get_id());
            return {};
        }
        log_debug(
            "netlist", "predecessor of ('{}', {}) is ('{}', {}) in net '{}' (id = {:08x})", get_name(), pred_pin_type_filter, pred.gate->get_name(), pred.pin_type, net->get_name(), net->get_id());
        return {pred};
    }
    else
    {
        std::vector<endpoint> result;
        for (const auto& it : m_in_nets)
        {
            auto net  = it.second;
            auto pred = net->get_src();
            if (pred.gate == nullptr)
            {
                log_debug("netlist", "predecessor on pin '{}' of gate '{}' (id = {:08x}) is unrouted.", it.first, this->get_name(), this->get_id());
                continue;
            }
            if ((pred_pin_type_filter != DONT_CARE) && (pred.pin_type != pred_pin_type_filter))
            {
                log_debug("netlist", "pin type of predecessor gate '{}' (id = {:08x}) does not match", pred.gate->get_name(), pred.gate->get_id());
                continue;
            }
            if ((gate_type_filter != DONT_CARE) && (pred.gate->get_type()->get_name() != gate_type_filter))
            {
                log_debug("netlist", "type of predecessor gate '{}' (id = {:08x}) does not match", pred.gate->get_name(), pred.gate->get_id());
                continue;
            }

            log_debug(
                "netlist", "predecessor of ('{}', {}) is ('{}', {}) in net '{}' (id = {:08x})", get_name(), pred_pin_type_filter, pred.gate->get_name(), pred.pin_type, net->get_name(), net->get_id());
            result.push_back(pred);
        }
        return result;
    }
}

endpoint gate::get_predecessor(const std::string& this_pin_type_filter, const std::string& pred_pin_type_filter, const std::string& gate_type_filter) const
{
    auto predecessors = this->get_predecessors(this_pin_type_filter, pred_pin_type_filter, gate_type_filter);
    if (predecessors.size() == 0)
    {
        return {nullptr, ""};
    }
    if (predecessors.size() > 1)
    {
        log_error("netlist", "internal error: multiple predecessors for '{}' at pin '{}'.", get_name(), pred_pin_type_filter);
        return {nullptr, ""};
    }

    return *(predecessors.begin());
}

std::set<endpoint> gate::get_unique_successors(const std::string& this_pin_type_filter, const std::string& suc_pin_type_filter, const std::string& gate_type_filter) const
{
    auto successors = this->get_successors(this_pin_type_filter, suc_pin_type_filter, gate_type_filter);
    return std::set<endpoint>(successors.begin(), successors.end());
}

std::vector<endpoint> gate::get_successors(const std::string& this_pin_type_filter, const std::string& suc_pin_type_filter, const std::string& gate_type_filter) const
{
    std::vector<endpoint> result;
    if (this_pin_type_filter != DONT_CARE)
    {
        auto it = m_out_nets.find(this_pin_type_filter);
        if (it == m_out_nets.end())
        {
            return result;
        }
        auto net        = it->second;
        auto successors = net->get_dsts();
        for (const auto& suc : successors)
        {
            if ((suc_pin_type_filter != DONT_CARE) && (suc.pin_type != suc_pin_type_filter))
            {
                log_debug("netlist", "pin type of successor gate '{}' (id = {:08x}) does not match", suc.gate->get_name(), suc.gate->get_id());
                continue;
            }
            if ((gate_type_filter != DONT_CARE) && (suc.gate->get_type()->get_name() != gate_type_filter))
            {
                log_debug("netlist", "type of successor gate '{}' (id = {:08x}) does not match", suc.gate->get_name(), suc.gate->get_id());
                continue;
            }

            log_debug(
                "netlist", "successor of ('{}', {}) is ('{}', {}) in net '{}' (id = {:08x})", get_name(), suc_pin_type_filter, suc.gate->get_name(), suc.pin_type, net->get_name(), net->get_id());
            result.push_back(suc);
        }
    }
    else
    {
        for (const auto& it : m_out_nets)
        {
            auto net        = it.second;
            auto successors = net->get_dsts();
            for (const auto& suc : successors)
            {
                if ((suc_pin_type_filter != DONT_CARE) && (suc.pin_type != suc_pin_type_filter))
                {
                    log_debug("netlist", "pin type of successor gate '{}' (id = {:08x}) does not match", suc.gate->get_name(), suc.gate->get_id());
                    continue;
                }
                if ((gate_type_filter != DONT_CARE) && (suc.gate->get_type()->get_name() != gate_type_filter))
                {
                    log_debug("netlist", "type of successor gate '{}' (id = {:08x}) does not match", suc.gate->get_name(), suc.gate->get_id());
                    continue;
                }

                log_debug(
                    "netlist", "successor of ('{}', {}) is ('{}', {}) in net '{}' (id = {:08x})", get_name(), suc_pin_type_filter, suc.gate->get_name(), suc.pin_type, net->get_name(), net->get_id());
                result.push_back(suc);
            }
        }
    }
    return result;
}
