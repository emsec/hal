#include "hal_core/netlist/gate.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/netlist/pins/gate_pin.h"
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
            log_info("gate", "the gates with IDs {} and {} are not equal due to an unequal ID, name, or type.", m_id, other.get_id());
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

    ssize_t Gate::get_hash() const
    {
        return (uintptr_t)this;
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

    BooleanFunction Gate::get_boolean_function(const std::string& name) const
    {
        std::string internal_name = name;
        if (internal_name.empty())
        {
            auto output_pins = m_type->get_output_pins();
            if (output_pins.empty())
            {
                return BooleanFunction();
            }
            internal_name = output_pins.front()->get_name();
        }

        if (m_type->has_component_of_type(GateTypeComponent::ComponentType::lut))
        {
            auto lut_pins = m_type->get_pins([internal_name](const GatePin* pin) { return pin->get_type() == PinType::lut && pin->get_name() == internal_name; });
            if (!lut_pins.empty())
            {
                return get_lut_function(lut_pins.front());
            }
        }

        if (auto it = m_functions.find(internal_name); it != m_functions.end())
        {
            return it->second;
        }

        auto map = m_type->get_boolean_functions();
        if (auto it = map.find(internal_name); it != map.end())
        {
            return it->second;
        }

        log_warning("gate", "could not get Boolean function '{}' of gate '{}' with ID {}: no function with that name exists", internal_name, m_name, std::to_string(m_id));
        return BooleanFunction();
    }

    BooleanFunction Gate::get_boolean_function(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            auto output_pins = m_type->get_output_pins();
            if (output_pins.empty())
            {
                log_warning("gate", "could not get Boolean function of gate '{}' with ID {}: gate type '{}' with ID {} has no output pins", m_name, m_id, m_type->get_name(), m_type->get_id());
                return BooleanFunction();
            }
            pin = output_pins.front();
        }

        return get_boolean_function(pin->get_name());
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
            res[it.first] = it.second;
        }

        if (!only_custom_functions && m_type->has_component_of_type(GateTypeComponent::ComponentType::lut))
        {
            for (auto pin : m_type->get_pins([](const GatePin* pin) { return pin->get_type() == PinType::lut; }))
            {
                res[pin->get_name()] = get_lut_function(pin);
            }
        }

        return res;
    }

    Result<BooleanFunction> Gate::get_resolved_boolean_function(const GatePin* pin) const
    {
        const std::function<Result<BooleanFunction>(const GatePin*, std::unordered_set<std::string>&)> get_resolved_boolean_function_internal =
            [this, &get_resolved_boolean_function_internal](const GatePin* output_pin, std::unordered_set<std::string>& on_stack) -> Result<BooleanFunction> {
            if (output_pin == nullptr)
            {
                return ERR("could not get resolved Boolean function of gate '" + this->get_name() + "' with ID " + std::to_string(this->get_id()) + ": given output pin is null.");
            }

            if (on_stack.find(output_pin->get_name()) != on_stack.end())
            {
                return ERR("could not get resolved Boolean function of gate '" + this->get_name() + "' with ID " + std::to_string(this->get_id())
                           + ": boolean functions of gate contain an endless recursion including pin '" + output_pin->get_name() + "'");
            }
            on_stack.insert(output_pin->get_name());

            BooleanFunction bf = this->get_boolean_function(output_pin);

            std::map<std::string, BooleanFunction> input_to_bf;
            std::vector<std::string> input_vars = utils::to_vector(bf.get_variable_names());
            for (const auto& var : bf.get_variable_names())
            {
                const GatePin* pin = this->get_type()->get_pin_by_name(var);
                if (pin == nullptr)
                {
                    return ERR("could not get resolved Boolean function of gate '" + this->get_name() + "' with ID " + std::to_string(this->get_id()) + ": failed to get input pin '" + var
                               + "' by name");
                }

                const PinDirection pin_dir = pin->get_direction();
                if (pin_dir == PinDirection::input)
                {
                    const Net* const input_net = this->get_fan_in_net(var);
                    if (input_net == nullptr)
                    {
                        // if no net is connected, the input pin name cannot be replaced
                        return ERR("could not get resolved Boolean function of gate '" + this->get_name() + "' with ID " + std::to_string(this->get_id()) + ": failed to get fan-in net at pin '"
                                   + pin->get_name() + "'");
                    }

                    const auto net_dec = BooleanFunctionNetDecorator(*input_net);
                    input_to_bf.insert({var, net_dec.get_boolean_variable()});
                }
                else if ((pin_dir == PinDirection::internal) || (pin_dir == PinDirection::output))
                {
                    const auto bf_interal_res = get_resolved_boolean_function_internal(pin, on_stack);
                    if (bf_interal_res.is_error())
                    {
                        return ERR_APPEND(bf_interal_res.get_error(),
                                          "could not get resolved Boolean function of gate '" + this->get_name() + "' with ID " + std::to_string(this->get_id())
                                              + ": failed to get Boolean function at output pin '" + pin->get_name() + "'");
                    }

                    input_to_bf.insert({pin->get_name(), bf_interal_res.get()});
                }
            }

            if (auto substituted = bf.substitute(input_to_bf); substituted.is_error())
            {
                return ERR_APPEND(substituted.get_error(),
                                  "could not get resolved Boolean function of gate '" + this->get_name() + "' with ID " + std::to_string(this->get_id())
                                      + ": failed to substitute variable inputs with other Boolean functions");
            }
            else
            {
                bf = substituted.get();
            }

            on_stack.erase(output_pin->get_name());

            return OK(bf);
        };

        std::unordered_set<std::string> on_stack;

        return get_resolved_boolean_function_internal(pin, on_stack);
    }

    BooleanFunction Gate::get_lut_function(const GatePin* pin) const
    {
        UNUSED(pin);

        LUTComponent* lut_component = m_type->get_component_as<LUTComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::lut; });
        if (lut_component == nullptr)
        {
            return BooleanFunction();
        }

        InitComponent* init_component =
            lut_component->get_component_as<InitComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::init; });
        if (init_component == nullptr)
        {
            return BooleanFunction();
        }

        const std::string& category  = init_component->get_init_category();
        const std::string& key       = init_component->get_init_identifiers().front();
        std::string config_str       = std::get<1>(get_data(category, key));
        auto is_ascending            = lut_component->is_init_ascending();
        std::vector<GatePin*> inputs = m_type->get_input_pins();

        auto result = BooleanFunction::Const(BooleanFunction::Value::ZERO);

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
                auto conjunction  = BooleanFunction::Const(1, 1);
                auto input_values = i;
                for (auto input : inputs)
                {
                    if ((input_values & 1) == 1)
                    {
                        conjunction &= BooleanFunction::Var(input->get_name());
                    }
                    else
                    {
                        conjunction &= ~BooleanFunction::Var(input->get_name());
                    }
                    input_values >>= 1;
                }
                result |= conjunction;
            }
        }

        auto f           = result.simplify();
        cache[cache_key] = f;
        return f;
    }

    bool Gate::add_boolean_function(const std::string& name, const BooleanFunction& func)
    {
        LUTComponent* lut_component = m_type->get_component_as<LUTComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::lut; });
        if (lut_component != nullptr)
        {
            InitComponent* init_component =
                lut_component->get_component_as<InitComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::init; });
            if (init_component != nullptr)
            {
                auto output_pins = m_type->get_output_pins();
                if (!output_pins.empty() && name == output_pins.front()->get_name())
                {
                    auto tt = func.compute_truth_table(m_type->get_input_pin_names());
                    if (tt.is_error())
                    {
                        log_error("netlist", "Boolean function '{} = {}' cannot be added to LUT gate '{}' wiht ID {}.", name, func.to_string(), m_name, m_id);
                        return false;
                    }
                    auto truth_table = tt.get();
                    if (truth_table.size() > 1)
                    {
                        log_error("netlist", "Boolean function '{} = {}' cannot be added to LUT gate '{}' with ID {} (= function is > 1-bit in output size). ", name, func.to_string(), m_name, m_id);
                        return false;
                    }

                    u64 config_value = 0;
                    if (!lut_component->is_init_ascending())
                    {
                        std::reverse(truth_table[0].begin(), truth_table[0].end());
                    }
                    for (auto v : truth_table[0])
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
                            return false;
                        }
                        config_value <<= 1;
                        config_value |= v;
                    }

                    const std::string& category = init_component->get_init_category();
                    const std::string& key      = init_component->get_init_identifiers().front();

                    std::stringstream stream;
                    stream << std::hex << config_value;
                    set_data(category, key, "bit_vector", stream.str());
                }
            }
        }

        m_functions[name] = func;
        m_event_handler->notify(GateEvent::event::boolean_function_changed, this);
        return true;
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

    std::vector<Net*> Gate::get_fan_in_nets() const
    {
        return m_in_nets;
    }

    Net* Gate::get_fan_in_net(const std::string& pin_name) const
    {
        auto ep = get_fan_in_endpoint(pin_name);
        if (ep == nullptr)
        {
            return nullptr;
        }
        return ep->get_net();
    }

    Net* Gate::get_fan_in_net(const GatePin* pin) const
    {
        auto ep = get_fan_in_endpoint(pin);
        if (ep == nullptr)
        {
            return nullptr;
        }
        return ep->get_net();
    }

    bool Gate::is_fan_in_net(const Net* net) const
    {
        if (net == nullptr)
        {
            log_warning("gate", "could not check whether net is a fan-in of gate '{}' with ID {}: 'nullptr' given as net", m_name, m_id);
            return false;
        }

        if (const auto it = std::find_if(m_in_endpoints.begin(), m_in_endpoints.end(), [net](const Endpoint* ep) { return ep->get_net() == net; }); it != m_in_endpoints.end())
        {
            return true;
        }

        return false;
    }

    std::vector<Endpoint*> Gate::get_fan_in_endpoints() const
    {
        return m_in_endpoints;
    }

    Endpoint* Gate::get_fan_in_endpoint(const std::string& pin_name) const
    {
        const GatePin* pin = m_type->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("gate",
                        "could not get fan-in endpoint of pin '{}' at gate '{}' with ID {}: no pin with that name exists for gate type '{}'",
                        pin_name,
                        m_name,
                        std::to_string(m_id),
                        m_type->get_name());
            return nullptr;
        }
        return get_fan_in_endpoint(pin);
    }

    Endpoint* Gate::get_fan_in_endpoint(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            log_warning("gate", "could not get fan-in endpoint of gate '{}' with ID {}: 'nullptr' given as pin", m_name, m_id);
            return nullptr;
        }
        if (PinDirection direction = pin->get_direction(); direction != PinDirection::input && direction != PinDirection::inout)
        {
            log_warning("gate", "could not get fan-in endpoint of pin '{}' at gate '{}' with ID {}: pin is not an input pin", pin->get_name(), m_name, m_id);
            return nullptr;
        }
        auto it = std::find_if(m_in_endpoints.begin(), m_in_endpoints.end(), [&pin](auto& ep) { return *ep->get_pin() == *pin; });
        if (it == m_in_endpoints.end())
        {
            log_debug("gate", "could not get fan-in endpoint of pin '{}' at gate '{}' with ID {}: no net is connected to pin", pin->get_name(), m_name, m_id);
            return nullptr;
        }

        return *it;
    }

    Endpoint* Gate::get_fan_in_endpoint(const Net* net) const
    {
        if (net == nullptr)
        {
            log_warning("gate", "could not get fan-in endpoint of gate '{}' with ID {}: 'nullptr' given as net", m_name, m_id);
            return nullptr;
        }

        const auto it = std::find_if(m_in_endpoints.begin(), m_in_endpoints.end(), [net](const Endpoint* ep) { return ep->get_net() == net; });
        if (it == m_in_endpoints.end())
        {
            log_warning("gate", "could not get fan-in endpoint of net '{}' with ID {} at gate '{}' with ID {}: net is not an input net", net->get_name(), net->get_id(), m_name, m_id);
            return nullptr;
        }

        return *it;
    }

    std::vector<Net*> Gate::get_fan_out_nets() const
    {
        return m_out_nets;
    }

    Net* Gate::get_fan_out_net(const std::string& pin_name) const
    {
        auto ep = get_fan_out_endpoint(pin_name);
        if (ep == nullptr)
        {
            return nullptr;
        }
        return ep->get_net();
    }

    Net* Gate::get_fan_out_net(const GatePin* pin) const
    {
        auto ep = get_fan_out_endpoint(pin);
        if (ep == nullptr)
        {
            return nullptr;
        }
        return ep->get_net();
    }

    bool Gate::is_fan_out_net(const Net* net) const
    {
        if (net == nullptr)
        {
            log_warning("gate", "could not check whether net is a fan-out of gate '{}' with ID {}: 'nullptr' given as net", m_name, m_id);
            return false;
        }

        if (const auto it = std::find_if(m_out_endpoints.begin(), m_out_endpoints.end(), [net](const Endpoint* ep) { return ep->get_net() == net; }); it != m_out_endpoints.end())
        {
            return true;
        }

        return false;
    }

    std::vector<Endpoint*> Gate::get_fan_out_endpoints() const
    {
        return m_out_endpoints;
    }

    Endpoint* Gate::get_fan_out_endpoint(const std::string& pin_name) const
    {
        const GatePin* pin = m_type->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("gate", "could not get fan-out endpoint of pin '{}' at gate '{}' with ID {}: no pin with that name exists for gate type '{}'", pin_name, m_name, m_id, m_type->get_name());
            return nullptr;
        }
        return get_fan_out_endpoint(pin);
    }

    Endpoint* Gate::get_fan_out_endpoint(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            log_warning("gate", "could not get fan-out endpoint of gate '{}' with ID {}: 'nullptr' given as pin", m_name, m_id);
            return nullptr;
        }
        if (PinDirection direction = pin->get_direction(); direction != PinDirection::output && direction != PinDirection::inout)
        {
            log_warning("gate", "could not get fan-out endpoint of pin '{}' at gate '{}' with ID {}: pin is not an output pin", pin->get_name(), m_name, m_id);
            return nullptr;
        }
        auto it = std::find_if(m_out_endpoints.begin(), m_out_endpoints.end(), [&pin](auto& ep) { return *ep->get_pin() == *pin; });
        if (it == m_out_endpoints.end())
        {
            log_debug("gate", "could not get fan-out endpoint of pin '{}' at gate '{}' with ID {}: no net is connected to pin", pin->get_name(), m_name, m_id);
            return nullptr;
        }

        return *it;
    }

    Endpoint* Gate::get_fan_out_endpoint(const Net* net) const
    {
        if (net == nullptr)
        {
            log_warning("gate", "could not get fan-out endpoint of gate '{}' with ID {}: 'nullptr' given as net", m_name, m_id);
            return nullptr;
        }

        const auto it = std::find_if(m_out_endpoints.begin(), m_out_endpoints.end(), [net](const Endpoint* ep) { return ep->get_net() == net; });
        if (it == m_out_endpoints.end())
        {
            log_warning("gate", "could not get fan-out endpoint of net '{}' with ID {} at gate '{}' with ID {}: net is not an output net", net->get_name(), net->get_id(), m_name, m_id);
            return nullptr;
        }

        return *it;
    }

    std::vector<Gate*> Gate::get_unique_predecessors(const std::function<bool(const GatePin* pin, Endpoint*)>& filter) const
    {
        std::unordered_set<Gate*> res;
        auto endpoints = get_predecessors(filter);
        res.reserve(endpoints.size());
        for (auto ep : endpoints)
        {
            res.insert(ep->get_gate());
        }
        return std::vector<Gate*>(res.begin(), res.end());
    }

    std::vector<Endpoint*> Gate::get_predecessors(const std::function<bool(const GatePin* pin, Endpoint*)>& filter) const
    {
        std::vector<Endpoint*> result;
        for (auto ep : m_in_endpoints)
        {
            auto pred_pin     = ep->get_pin();
            auto predecessors = ep->get_net()->get_sources();
            if (!filter)
            {
                result.insert(result.end(), predecessors.begin(), predecessors.end());
            }
            else
            {
                for (auto pre : predecessors)
                {
                    if (!filter(pred_pin, pre))
                    {
                        continue;
                    }
                    result.push_back(pre);
                }
            }
        }
        return result;
    }

    Endpoint* Gate::get_predecessor(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            log_warning("gate", "could not get predecessor endpoint of gate '{}' with ID {}: 'nullptr' given as pin", m_name, std::to_string(m_id));
            return nullptr;
        }
        if (auto direction = pin->get_direction(); direction != PinDirection::input && direction != PinDirection::inout)
        {
            log_warning("gate", "could not get predecessor endpoint of pin '{}' at gate '{}' with ID {}: pin is not an input pin", pin->get_name(), m_name, std::to_string(m_id));
            return nullptr;
        }
        auto predecessors = get_predecessors([pin](const auto p, auto) -> bool { return *p == *pin; });
        if (predecessors.size() == 0)
        {
            return nullptr;
        }
        if (predecessors.size() > 1)
        {
            log_warning("gate", "gate '{}' with ID {} has multiple predecessors at input pin '{}' in netlist with ID {}.", m_name, m_id, pin->get_name(), m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return predecessors[0];
    }

    Endpoint* Gate::get_predecessor(const std::string& pin_name) const
    {
        const GatePin* pin = m_type->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("gate",
                        "could not get predecessor endpoint of pin '{}' at gate '{}' with ID {}: no pin with that name exists for gate type '{}'",
                        pin_name,
                        m_name,
                        std::to_string(m_id),
                        m_type->get_name());
            return nullptr;
        }
        return get_predecessor(pin);
    }

    std::vector<Gate*> Gate::get_unique_successors(const std::function<bool(const GatePin* pin, Endpoint*)>& filter) const
    {
        std::unordered_set<Gate*> res;
        auto endpoints = get_successors(filter);
        res.reserve(endpoints.size());
        for (auto ep : endpoints)
        {
            res.insert(ep->get_gate());
        }
        return std::vector<Gate*>(res.begin(), res.end());
    }

    std::vector<Endpoint*> Gate::get_successors(const std::function<bool(const GatePin* pin, Endpoint*)>& filter) const
    {
        std::vector<Endpoint*> result;
        for (auto ep : m_out_endpoints)
        {
            auto suc_pin    = ep->get_pin();
            auto successors = ep->get_net()->get_destinations();
            if (!filter)
            {
                result.insert(result.end(), successors.begin(), successors.end());
            }
            else
            {
                for (auto suc : successors)
                {
                    if (!filter(suc_pin, suc))
                    {
                        continue;
                    }
                    result.push_back(suc);
                }
            }
        }
        return result;
    }

    Endpoint* Gate::get_successor(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            log_warning("gate", "could not get successor endpoint of gate '{}' with ID {}: 'nullptr' given as pin", m_name, std::to_string(m_id));
            return nullptr;
        }
        if (auto direction = pin->get_direction(); direction != PinDirection::output && direction != PinDirection::inout)
        {
            log_warning("gate", "could not get successor endpoint of pin '{}' at gate '{}' with ID {}: pin is not an output pin", pin->get_name(), m_name, std::to_string(m_id));
            return nullptr;
        }
        auto successors = get_successors([pin](const auto p, auto) -> bool { return *p == *pin; });
        if (successors.size() == 0)
        {
            return nullptr;
        }
        if (successors.size() > 1)
        {
            log_warning("gate", "gate '{}' with ID {} has multiple successor at output pin '{}' in netlist with ID {}.", m_name, m_id, pin->get_name(), m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return successors[0];
    }

    Endpoint* Gate::get_successor(const std::string& pin_name) const
    {
        const GatePin* pin = m_type->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("gate",
                        "could not get successor endpoint of pin '{}' at gate '{}' with ID {}: no pin with that name exists for gate type '{}'",
                        pin_name,
                        m_name,
                        std::to_string(m_id),
                        m_type->get_name());
            return nullptr;
        }
        return get_successor(pin);
    }

    Result<std::vector<std::string>> Gate::get_init_data() const
    {
        InitComponent* init_component = m_type->get_component_as<InitComponent>([](const GateTypeComponent* c) { return c->get_type() == GateTypeComponent::ComponentType::init; });
        if (init_component == nullptr)
        {
            return ERR("could not get INIT data for gate '" + m_name + "' with ID '" + std::to_string(m_id) + "': type '" + m_type->get_name() + "' with ID " + std::to_string(m_type->get_id())
                       + "' cannot hold INIT data");
        }

        const std::string& category                 = init_component->get_init_category();
        const std::vector<std::string>& identifiers = init_component->get_init_identifiers();

        std::vector<std::string> init_data;
        for (const std::string& id : identifiers)
        {
            init_data.push_back(std::get<1>(get_data(category, id)));
        }

        return OK(init_data);
    }

    Result<std::monostate> Gate::set_init_data(const std::vector<std::string>& init_data)
    {
        InitComponent* init_component = m_type->get_component_as<InitComponent>([](const GateTypeComponent* c) { return c->get_type() == GateTypeComponent::ComponentType::init; });
        if (init_component == nullptr)
        {
            return ERR("could not set INIT data for gate '" + m_name + "' with ID '" + std::to_string(m_id) + "': type '" + m_type->get_name() + "' with ID " + std::to_string(m_type->get_id())
                       + "' cannot hold INIT data");
        }

        const std::string& category                 = init_component->get_init_category();
        const std::vector<std::string>& identifiers = init_component->get_init_identifiers();

        if (identifiers.size() != init_data.size())
        {
            return ERR("could not set INIT data for gate '" + m_name + "' with ID '" + std::to_string(m_id) + "': provided INIT data has size " + std::to_string(init_data.size())
                       + " and must be of size " + std::to_string(identifiers.size()));
        }

        u32 i = 0;
        for (const std::string& id : identifiers)
        {
            set_data(category, id, "bit_vector", init_data.at(i));
            i++;
        }

        return OK({});
    }
}    // namespace hal
