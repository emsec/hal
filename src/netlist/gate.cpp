#include "hal_core/netlist/gate.h"

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_handler.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/utilities/log.h"

#include <assert.h>
#include <iomanip>
#include <sstream>

template<typename T, T m, int k>
static inline T swapbits(T p)
{
    T q = ((p >> k) ^ p) & m;
    return p ^ q ^ (q << k);
}

static u64 bitreverse(u64 n)
{
    static const u64 m0 = 0x5555555555555555LLU;
    static const u64 m1 = 0x0300c0303030c303LLU;
    static const u64 m2 = 0x00c0300c03f0003fLLU;
    static const u64 m3 = 0x00000ffc00003fffLLU;
    n                   = ((n >> 1) & m0) | (n & m0) << 1;
    n                   = swapbits<u64, m1, 4>(n);
    n                   = swapbits<u64, m2, 8>(n);
    n                   = swapbits<u64, m3, 20>(n);
    n                   = (n >> 34) | (n << 30);
    return n;
}

namespace hal
{
    Gate::Gate(NetlistInternalManager* mgr, EventHandler* event_handler, const u32 id, GateType* gt, const std::string& name, i32 x, i32 y)
        : m_internal_manager(mgr), m_id(id), m_name(name), m_type(gt), m_x(x), m_y(y), m_event_handler(event_handler)
    {
        ;
    }

    bool Gate::operator==(const Gate& other) const
    {
        if (m_id != other.get_id() || m_name != other.get_name() || m_type != other.get_type())
        {
            // log_info("gate", "the gates with IDs {} and {} are not equal due to an unequal ID, name, or type.", m_id, other.get_id());
            return false;
        }

        if (m_x != other.get_location_x() || m_y != other.get_location_y())
        {
            log_info("gate", "the gates with IDs {} and {} are not equal due to unequal location data.", m_id, other.get_id());
            return false;
        }

        if (is_gnd_gate() != other.is_gnd_gate() || is_vcc_gate() != other.is_vcc_gate())
        {
            log_info("gate", "the gates with IDs {} and {} are not equal as one is a GND or VCC gate and the other is not.", m_id, other.get_id());
            return false;
        }

        if (m_functions != other.get_boolean_functions(true))
        {
            log_info("gate", "the gates with IDs {} and {} are not equal due to an unequal Boolean functions.", m_id, other.get_id());
            return false;
        }

        if (!DataContainer::operator==(other))
        {
            log_info("gate", "the gates with IDs {} and {} are not equal due to unequal data.", m_id, other.get_id());
            return false;
        }

        return true;
    }

    bool Gate::operator!=(const Gate& other) const
    {
        return !operator==(other);
    }

    u32 Gate::get_id() const
    {
        return m_id;
    }

    Netlist* Gate::get_netlist() const
    {
        return m_internal_manager->m_netlist;
    }

    const std::string& Gate::get_name() const
    {
        return m_name;
    }

    void Gate::set_name(const std::string& name)
    {
        if (utils::trim(name).empty())
        {
            log_error("gate", "gate name cannot be empty.");
            return;
        }
        if (name != m_name)
        {
            log_info("net", "changed name for gate with ID {} from '{}' to '{}' in netlist with ID {}.", m_id, m_name, name, m_internal_manager->m_netlist->get_id());

            m_name = name;

            m_event_handler->notify(GateEvent::event::name_changed, this);
        }
    }

    GateType* Gate::get_type() const
    {
        return m_type;
    }

    i32 Gate::get_location_x() const
    {
        return m_x;
    }

    i32 Gate::get_location_y() const
    {
        return m_y;
    }

    std::pair<i32, i32> Gate::get_location() const
    {
        return {m_x, m_y};
    }

    bool Gate::has_location() const
    {
        return m_x != -1 && m_y != -1;
    }

    void Gate::set_location_x(i32 x)
    {
        if (x != m_x)
        {
            m_x = x;
            m_event_handler->notify(GateEvent::event::location_changed, this);
        }
    }

    void Gate::set_location_y(i32 y)
    {
        if (y != m_y)
        {
            m_y = y;
            m_event_handler->notify(GateEvent::event::location_changed, this);
        }
    }

    void Gate::set_location(const std::pair<i32, i32>& location)
    {
        set_location_x(location.first);
        set_location_y(location.second);
    }

    Module* Gate::get_module() const
    {
        return m_module;
    }

    Grouping* Gate::get_grouping() const
    {
        return m_grouping;
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

        if (m_type->has_property(GateTypeProperty::lut))
        {
            auto lut_pins = m_type->get_pins_of_type(PinType::lut);
            if (std::find(lut_pins.begin(), lut_pins.end(), name) != lut_pins.end())
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

        if (!only_custom_functions && m_type->has_property(GateTypeProperty::lut))
        {
            for (auto pin : m_type->get_pins_of_type(PinType::lut))
            {
                res.emplace(pin, get_lut_function(pin));
            }
        }

        return res;
    }

    BooleanFunction Gate::get_lut_function(const std::string& pin) const
    {
        UNUSED(pin);

        std::string category   = m_type->get_config_data_category();
        std::string key        = m_type->get_config_data_identifier();
        std::string config_str = std::get<1>(get_data(category, key));
        auto is_ascending      = m_type->is_lut_init_ascending();
        auto inputs            = get_input_pins();

        BooleanFunction result = BooleanFunction::ZERO;

        if (config_str.empty())
        {
            return result;
        }

        if (inputs.size() > 6)
        {
            log_error("gate", "LUT gate '{}' with ID {} in netlist with ID {} has more than six input pins, which is currently not supported.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return BooleanFunction();
        }

        u64 config = 0;
        try
        {
            config = std::stoull(config_str, nullptr, 16);
        }
        catch (std::invalid_argument& ex)
        {
            log_error("gate",
                      "LUT gate '{}' with ID {} in netlist with ID {} has invalid configuration string of '{}', which is not a hex value.",
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id(),
                      config_str);
            return BooleanFunction();
        }

        u32 max_config_size = 1 << inputs.size();

        if (is_ascending)
        {
            config = bitreverse(config) >> (64 - max_config_size);
        }

        auto cache_key = std::make_pair(inputs, config);
        auto& cache    = m_internal_manager->m_lut_function_cache;

        if (auto it = cache.find(cache_key); it != cache.end())
        {
            return it->second;
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

        if (config_size > max_config_size)
        {
            log_error("gate",
                      "LUT gate '{}' with ID {} in netlist with ID {} supports a configuration string of up to {} bits, but '{}' comprises {} bits instead.",
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id(),
                      max_config_size,
                      config_str,
                      config_str.size() * 4);
            return BooleanFunction();
        }

        for (u32 i = 0; config != 0 && i < max_config_size; i++)
        {
            u8 bit = (config & 1);
            config >>= 1;
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
                        clause &= ~BooleanFunction(input);
                    }
                    input_values >>= 1;
                }
                result |= clause;
            }
        }

        auto f = result.optimize();
        cache.emplace(cache_key, f);
        return f;
    }

    void Gate::add_boolean_function(const std::string& name, const BooleanFunction& func)
    {
        if (m_type->has_property(GateTypeProperty::lut))
        {
            auto output_pins = m_type->get_output_pins();
            if (!output_pins.empty() && name == output_pins[0])
            {
                auto tt = func.get_truth_table(get_input_pins());

                u64 config_value = 0;
                if (!m_type->is_lut_init_ascending())
                {
                    std::reverse(tt.begin(), tt.end());
                }
                for (auto v : tt)
                {
                    if (v == BooleanFunction::X)
                    {
                        log_error("netlist",
                                  "Boolean function '{} = {}' cannot be added to LUT gate '{}' with ID {} in netlist with ID {} as its truth table contains undefined values.",
                                  name,
                                  func.to_string(),
                                  m_name,
                                  m_id,
                                  m_internal_manager->m_netlist->get_id());
                        return;
                    }
                    config_value <<= 1;
                    config_value |= v;
                }

                std::string category = m_type->get_config_data_category();
                std::string key      = m_type->get_config_data_identifier();

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
        return m_internal_manager->m_netlist->mark_vcc_gate(this);
    }

    bool Gate::mark_gnd_gate()
    {
        return m_internal_manager->m_netlist->mark_gnd_gate(this);
    }

    bool Gate::unmark_vcc_gate()
    {
        return m_internal_manager->m_netlist->unmark_vcc_gate(this);
    }

    bool Gate::unmark_gnd_gate()
    {
        return m_internal_manager->m_netlist->unmark_gnd_gate(this);
    }

    bool Gate::is_vcc_gate() const
    {
        return m_internal_manager->m_netlist->is_vcc_gate(this);
    }

    bool Gate::is_gnd_gate() const
    {
        return m_internal_manager->m_netlist->is_gnd_gate(this);
    }

    std::vector<std::string> Gate::get_input_pins() const
    {
        return m_type->get_input_pins();
    }

    std::vector<std::string> Gate::get_output_pins() const
    {
        return m_type->get_output_pins();
    }

    std::vector<Net*> Gate::get_fan_in_nets() const
    {
        return m_in_nets;
    }

    std::vector<Endpoint*> Gate::get_fan_in_endpoints() const
    {
        return m_in_endpoints;
    }

    Net* Gate::get_fan_in_net(const std::string& pin) const
    {
        auto ep = get_fan_in_endpoint(pin);
        if (ep == nullptr)
        {
            return nullptr;
        }
        return ep->get_net();
    }

    Endpoint* Gate::get_fan_in_endpoint(const std::string& pin) const
    {
        auto it = std::find_if(m_in_endpoints.begin(), m_in_endpoints.end(), [&pin](auto& ep) { return ep->get_pin() == pin; });

        if (it == m_in_endpoints.end())
        {
            log_debug("gate", "no net is connected to input pin '{}' of gate '{}' with ID {} in netlist with ID {}.", pin, m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return *it;
    }

    std::vector<Net*> Gate::get_fan_out_nets() const
    {
        return m_out_nets;
    }

    std::vector<Endpoint*> Gate::get_fan_out_endpoints() const
    {
        return m_out_endpoints;
    }

    Net* Gate::get_fan_out_net(const std::string& pin) const
    {
        auto ep = get_fan_out_endpoint(pin);
        if (ep == nullptr)
        {
            return nullptr;
        }
        return ep->get_net();
    }

    Endpoint* Gate::get_fan_out_endpoint(const std::string& pin) const
    {
        auto it = std::find_if(m_out_endpoints.begin(), m_out_endpoints.end(), [&pin](auto& ep) { return ep->get_pin() == pin; });

        if (it == m_out_endpoints.end())
        {
            log_debug("gate", "no net is connected to output pin '{}' of gate '{}' with ID {} in netlist with ID {}.", pin, m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return *it;
    }

    std::vector<Gate*> Gate::get_unique_predecessors(const std::function<bool(const std::string& starting_pin, Endpoint*)>& filter) const
    {
        std::unordered_set<Gate*> res;
        auto endpoints = this->get_predecessors(filter);
        res.reserve(endpoints.size());
        for (auto ep : endpoints)
        {
            res.insert(ep->get_gate());
        }
        return std::vector<Gate*>(res.begin(), res.end());
    }

    std::vector<Endpoint*> Gate::get_predecessors(const std::function<bool(const std::string& starting_pin, Endpoint*)>& filter) const
    {
        std::vector<Endpoint*> result;
        for (auto ep : m_in_endpoints)
        {
            auto pin          = ep->get_pin();
            auto predecessors = ep->get_net()->get_sources();
            if (!filter)
            {
                result.insert(result.end(), predecessors.begin(), predecessors.end());
            }
            else
            {
                for (auto pre : predecessors)
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

    Endpoint* Gate::get_predecessor(const std::string& input_pin) const
    {
        auto predecessors = this->get_predecessors([&input_pin](auto& starting_pin, auto) -> bool { return starting_pin == input_pin; });
        if (predecessors.size() == 0)
        {
            return nullptr;
        }
        if (predecessors.size() > 1)
        {
            log_error("gate", "gate '{}' with ID {} has multiple predecessors at input pin '{}' in netlist with ID {}.", m_name, m_id, input_pin, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return predecessors[0];
    }

    std::vector<Gate*> Gate::get_unique_successors(const std::function<bool(const std::string& starting_pin, Endpoint*)>& filter) const
    {
        std::unordered_set<Gate*> res;
        auto endpoints = this->get_successors(filter);
        res.reserve(endpoints.size());
        for (auto ep : endpoints)
        {
            res.insert(ep->get_gate());
        }
        return std::vector<Gate*>(res.begin(), res.end());
    }

    std::vector<Endpoint*> Gate::get_successors(const std::function<bool(const std::string& starting_pin, Endpoint*)>& filter) const
    {
        std::vector<Endpoint*> result;
        for (auto ep : m_out_endpoints)
        {
            auto pin        = ep->get_pin();
            auto successors = ep->get_net()->get_destinations();
            if (!filter)
            {
                result.insert(result.end(), successors.begin(), successors.end());
            }
            else
            {
                for (auto suc : successors)
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

    Endpoint* Gate::get_successor(const std::string& output_pin) const
    {
        auto successors = this->get_successors([&output_pin](auto& starting_pin, auto) -> bool { return starting_pin == output_pin; });
        if (successors.size() == 0)
        {
            return nullptr;
        }
        if (successors.size() > 1)
        {
            log_error("gate", "gate '{}' with ID {} has multiple successors at output pin '{}' in netlist with ID {}.", m_name, m_id, output_pin, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return successors[0];
    }
}    // namespace hal
