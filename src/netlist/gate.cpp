#include "hal_core/netlist/gate.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/netlist/parameter.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/utilities/log.h"

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace
{
    template<typename T, T m, int k>
    inline T swapbits(T p)
    {
        T q = ((p >> k) ^ p) & m;
        return p ^ q ^ (q << k);
    }

    std::string lut_init_read(const hal::DataContainer* dc, const std::string& key)
    {
        if (!dc->has_parameter(key))
        {
            return "";
        }
        const std::string pv = dc->get_parameter_value(key).get();
        return (pv.size() >= 2 && pv[0] == '0' && pv[1] == 'x') ? pv.substr(2) : pv;
    }

    void lut_init_write(hal::DataContainer* dc, const std::string& key, const std::string& plain_hex)
    {
        if (plain_hex.empty())
        {
            return;
        }
        const std::string value = "0x" + plain_hex;
        const size_t nbits      = plain_hex.size() * 4;
        if (nbits == 0 || nbits > 64)
        {
            return;
        }

        auto param_res = hal::Parameter::BitVector(key, static_cast<u16>(nbits), "");
        if (param_res.is_error())
        {
            return;
        }

        dc->set_parameter(param_res.get(), value);
    }

    u64 bitreverse(u64 n)
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
}    // namespace

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
            log_debug("gate", "the gates with IDs {} and {} are not equal due to an unequal ID, name, or type.", m_id, other.get_id());
            return false;
        }

        if (m_x != other.get_location_x() || m_y != other.get_location_y())
        {
            log_debug("gate", "the gates with IDs {} and {} are not equal due to unequal location data.", m_id, other.get_id());
            return false;
        }

        if (is_gnd_gate() != other.is_gnd_gate() || is_vcc_gate() != other.is_vcc_gate())
        {
            log_debug("gate", "the gates with IDs {} and {} are not equal as one is a GND or VCC gate and the other is not.", m_id, other.get_id());
            return false;
        }

        if (m_functions != other.get_boolean_functions(true))
        {
            log_debug("gate", "the gates with IDs {} and {} are not equal due to an unequal Boolean functions.", m_id, other.get_id());
            return false;
        }

        if (!DataContainer::operator==(other))
        {
            log_debug("gate", "the gates with IDs {} and {} are not equal due to unequal data.", m_id, other.get_id());
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
        return m_x >= 0 && m_y >= 0;
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

    std::vector<Module*> Gate::get_modules(const std::function<bool(Module*)>& filter, bool recursive) const
    {
        std::vector<Module*> res;

        if (!filter)
        {
            res.push_back(m_module);
        }
        else
        {
            if (filter(m_module))
            {
                res.push_back(m_module);
            }
        }

        if (recursive)
        {
            std::vector<Module*> more = m_module->get_parent_modules(filter, true);
            res.reserve(res.size() + more.size());
            res.insert(res.end(), more.begin(), more.end());
        }
        return res;
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
            const auto output_pins = m_type->get_output_pins();
            if (output_pins.empty())
            {
                log_warning("gate", "could not get Boolean function of gate '{}' with ID {}: gate type has no output pins", m_name, m_id);
                return BooleanFunction();
            }
            internal_name = output_pins.front()->get_name();
        }
        auto res = get_boolean_function(internal_name, false, false);
        if (res.is_error())
        {
            log_warning("gate", "{}", res.get_error().get());
            return BooleanFunction();
        }
        return res.get();
    }

    Result<BooleanFunction> Gate::get_boolean_function(const std::string& name, bool inlined, bool substitute_nets) const
    {
        if (name.empty())
        {
            return ERR("could not get Boolean function of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": name must not be empty");
        }
        const std::string& internal_name = name;

        if (substitute_nets && !inlined)
        {
            return ERR("could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": substitute_nets requires inlined=true");
        }

        // Raw lookup (no inlining): LUT → m_functions → gate type BFs
        if (!inlined)
        {
            if (m_type->has_component_of_type(GateTypeComponent::ComponentType::lut))
            {
                auto lut_pins = m_type->get_pins([&internal_name](const GatePin* p) { return p->get_type() == PinType::lut && p->get_name() == internal_name; });
                if (!lut_pins.empty())
                {
                    return OK(get_lut_function(lut_pins.front()));
                }
            }
            if (auto it = m_functions.find(internal_name); it != m_functions.end())
            {
                return OK(it->second);
            }
            const auto type_bfs = m_type->get_boolean_functions();
            if (auto it = type_bfs.find(internal_name); it != type_bfs.end())
            {
                return OK(it->second);
            }
            return ERR("could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": no function with that name exists");
        }

        // Inlined path: substitute internal/output pin variables and gate parameters.
        // Seed on_stack with the starting name so immediate self-references are caught
        // in the first variable scan rather than requiring an extra recursion level.
        BooleanFunction bf;
        const GatePin* pin = m_type->get_pin_by_name(internal_name);
        if (pin != nullptr)
        {
            std::unordered_set<std::string> on_stack = {internal_name};
            auto res = get_inlined_boolean_function(get_boolean_function(pin), on_stack);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id));
            }
            bf = res.get();
        }
        else
        {
            // Not a pin (custom-named function): raw lookup then inline
            if (auto it = m_functions.find(internal_name); it != m_functions.end())
            {
                bf = it->second;
            }
            else
            {
                const auto type_bfs = m_type->get_boolean_functions();
                if (auto it = type_bfs.find(internal_name); it != type_bfs.end())
                {
                    bf = it->second;
                }
                else
                {
                    return ERR("could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": no function with that name exists");
                }
            }
            std::unordered_set<std::string> on_stack;
            auto res = get_inlined_boolean_function(bf, on_stack);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id));
            }
            bf = res.get();
        }

        if (substitute_nets)
        {
            const auto& type_params = m_type->get_parameters();
            std::map<std::string, BooleanFunction> net_subs;
            for (const auto& var : bf.get_variable_names())
            {
                if (type_params.count(var))
                {
                    continue;    // parameter variable, not a net
                }
                const GatePin* var_pin = m_type->get_pin_by_name(var);
                if (var_pin == nullptr || var_pin->get_direction() != PinDirection::input)
                {
                    return ERR("could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": variable '" + var
                               + "' is not a recognized input pin");
                }
                const Net* const input_net = get_fan_in_net(var);
                if (input_net == nullptr)
                {
                    return ERR("could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": no fan-in net at pin '" + var + "'");
                }
                net_subs[var] = BooleanFunctionNetDecorator(*input_net).get_boolean_variable();
            }
            if (!net_subs.empty())
            {
                auto sub_res = bf.substitute(net_subs);
                if (sub_res.is_error())
                {
                    return ERR_APPEND(sub_res.get_error(),
                                      "could not get Boolean function '" + internal_name + "' of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": net substitution failed");
                }
                bf = sub_res.get();
            }
        }

        return OK(bf);
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

    Result<BooleanFunction> Gate::get_boolean_function(const GatePin* pin, bool inlined, bool substitute_nets) const
    {
        if (pin == nullptr)
        {
            return ERR("could not get Boolean function of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": given pin is a nullptr");
        }
        return get_boolean_function(pin->get_name(), inlined, substitute_nets);
    }

    std::unordered_map<std::string, BooleanFunction> Gate::get_boolean_functions(bool only_custom_functions) const
    {
        std::unordered_map<std::string, BooleanFunction> result;
        if (!only_custom_functions)
        {
            for (const auto& [fn, _] : m_type->get_boolean_functions())
            {
                result[fn] = get_boolean_function(fn);
            }
            if (m_type->has_component_of_type(GateTypeComponent::ComponentType::lut))
            {
                for (const auto* p : m_type->get_pins([](const GatePin* p) { return p->get_type() == PinType::lut; }))
                {
                    result[p->get_name()] = get_boolean_function(p->get_name());
                }
            }
        }
        for (const auto& [fn, _] : m_functions)
        {
            result[fn] = get_boolean_function(fn);
        }
        return result;
    }

    Result<std::unordered_map<std::string, BooleanFunction>> Gate::get_boolean_functions(bool only_custom_functions, bool inlined, bool substitute_nets) const
    {
        std::vector<std::string> names;
        if (!only_custom_functions)
        {
            for (const auto& [fn, _] : m_type->get_boolean_functions())
            {
                names.push_back(fn);
            }
            if (m_type->has_component_of_type(GateTypeComponent::ComponentType::lut))
            {
                for (const auto* p : m_type->get_pins([](const GatePin* p) { return p->get_type() == PinType::lut; }))
                {
                    names.push_back(p->get_name());
                }
            }
        }
        for (const auto& [fn, _] : m_functions)
        {
            names.push_back(fn);
        }

        std::unordered_map<std::string, BooleanFunction> result;
        for (const auto& fn : names)
        {
            auto res = get_boolean_function(fn, inlined, substitute_nets);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not get Boolean functions of gate '" + m_name + "' with ID " + std::to_string(m_id));
            }
            result[fn] = res.get();
        }
        return OK(result);
    }

    Result<BooleanFunction> Gate::get_inlined_boolean_function(BooleanFunction bf, std::unordered_set<std::string>& on_stack) const
    {
        const auto& type_params = m_type->get_parameters();
        std::map<std::string, BooleanFunction> subs;
        bool has_param_subs = false;

        for (const auto& var : bf.get_variable_names())
        {
            const GatePin* var_pin = m_type->get_pin_by_name(var);
            if (var_pin == nullptr)
            {
                // May be a parameter — substitute with its instance value if set
                if (type_params.count(var))
                {
                    if (const auto it = m_parameters.find(var); it != m_parameters.end())
                    {
                        const auto& [decl, value] = it->second;
                        if (auto enc = decl.encode_as_int(value); enc.is_ok())
                        {
                            subs[var]      = BooleanFunction::Const(enc.get(), decl.get_size());
                            has_param_subs = true;
                        }
                    }
                }
                // Unknown variable or un-valued parameter: leave as-is
                continue;
            }
            const PinDirection dir = var_pin->get_direction();
            if (dir == PinDirection::internal || dir == PinDirection::output)
            {
                if (on_stack.count(var))
                {
                    return ERR("could not inline Boolean function of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": circular dependency at variable '" + var + "'");
                }
                on_stack.insert(var);
                auto res = get_inlined_boolean_function(get_boolean_function(var_pin), on_stack);
                on_stack.erase(var);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not inline Boolean function of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": failed at variable '" + var + "'");
                }
                subs[var] = res.get();
            }
            // input pins: leave as-is
        }

        if (!subs.empty())
        {
            auto sub_res = bf.substitute(subs);
            if (sub_res.is_error())
            {
                return ERR_APPEND(sub_res.get_error(), "could not inline Boolean function of gate '" + m_name + "' with ID " + std::to_string(m_id) + ": substitution failed");
            }
            bf = has_param_subs ? sub_res.get().simplify() : sub_res.get();
        }

        return OK(bf);
    }

    BooleanFunction Gate::get_lut_function(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            log_error("gate", "could not get LUT function of gate '{}' with ID {}: pin is nullptr.", m_name, m_id);
            return BooleanFunction();
        }

        const LUTComponent* lut_component = m_type->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return c->get_type() == GateTypeComponent::ComponentType::lut; });
        if (lut_component == nullptr)
        {
            return BooleanFunction();
        }

        const LUTComponent::LUTOutputConfig* cfg = lut_component->get_output_pin_config(pin->get_name());
        if (cfg == nullptr)
        {
            return BooleanFunction();
        }

        const u32 bit_count  = cfg->bit_count;
        const u32 bit_offset = cfg->bit_offset;

        if (bit_count > 64)
        {
            log_error("gate", "LUT gate '{}' with ID {} in netlist with ID {} has an INIT string longer than 64 bits, which is not supported.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return BooleanFunction();
        }

        std::string config_str = lut_init_read(this, cfg->init_identifier);
        if (config_str.empty())
        {
            return BooleanFunction::Const(BooleanFunction::Value::ZERO);
        }

        if (cfg->input_pins.empty())
        {
            log_error("gate", "LUT gate '{}' with ID {}: output pin '{}' has no input_pins configured.", m_name, m_id, pin->get_name());
            return BooleanFunction();
        }
        std::vector<GatePin*> inputs;
        for (const auto& pname : cfg->input_pins)
        {
            if (GatePin* p = m_type->get_pin_by_name(pname); p != nullptr)
            {
                inputs.push_back(p);
            }
        }

        {
            auto res = LUTComponent::extract_init_slice(config_str, bit_offset, bit_count);
            if (res.is_error())
            {
                log_error("gate", "LUT gate '{}' with ID {} in netlist with ID {}: {}", m_name, m_id, m_internal_manager->m_netlist->get_id(), res.get_error().get());
                return BooleanFunction();
            }
            config_str = res.get();
        }

        u64 config = 0;
        try
        {
            config = std::stoull(config_str, nullptr, 16);
        }
        catch (const std::invalid_argument&)
        {
            log_error("gate",
                      "LUT gate '{}' with ID {} in netlist with ID {} has invalid configuration string of '{}', which is not a hex value.",
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id(),
                      config_str);
            return BooleanFunction();
        }
        catch (const std::out_of_range&)
        {
            log_error("gate",
                      "LUT gate '{}' with ID {} in netlist with ID {} has invalid configuration string of '{}', which has too many hex digits.",
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id(),
                      config_str);
            return BooleanFunction();
        }

        const u32 max_config_size = bit_count;

        if (cfg->is_ascending)
        {
            config = bitreverse(config) >> (64 - max_config_size);
        }

        auto& cache          = m_internal_manager->m_lut_function_cache;
        const auto cache_key = std::make_pair(inputs, config);
        if (const auto it = cache.find(cache_key); it != cache.end())
        {
            return it->second;
        }

        const u32 config_size = (config == 0) ? 0 : (64u - static_cast<u32>(__builtin_clzll(config)));
        if (config_size > max_config_size)
        {
            log_error("gate",
                      "LUT gate '{}' with ID {} in netlist with ID {} supports up to {} config bits, but '{}' encodes {} bits.",
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id(),
                      max_config_size,
                      config_str,
                      config_str.size() * 4);
            return BooleanFunction();
        }

        auto result = BooleanFunction::Const(BooleanFunction::Value::ZERO);
        for (u32 i = 0; config != 0 && i < max_config_size; i++)
        {
            if (config & 1)
            {
                auto conjunction  = BooleanFunction::Const(1, 1);
                auto input_values = i;
                for (auto input : inputs)
                {
                    conjunction &= ((input_values & 1) == 1) ? BooleanFunction::Var(input->get_name()) : ~BooleanFunction::Var(input->get_name());
                    input_values >>= 1;
                }
                result |= conjunction;
            }
            config >>= 1;
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
            auto lut_pins                                   = m_type->get_pins([&name](const GatePin* p) { return p->get_type() == PinType::lut && p->get_name() == name; });
            const LUTComponent::LUTOutputConfig* output_cfg = lut_component->get_output_pin_config(name);
            if (!lut_pins.empty() && output_cfg != nullptr)
            {
                const std::string& key = output_cfg->init_identifier;
                const u32 bit_offset   = output_cfg->bit_offset;
                const u32 bit_count    = output_cfg->bit_count;

                {
                    std::vector<std::string> input_pin_names;
                    if (!output_cfg->input_pins.empty())
                    {
                        input_pin_names = output_cfg->input_pins;
                    }
                    else
                    {
                        input_pin_names           = m_type->get_input_pin_names();
                        const u32 effective_count = __builtin_ctz(bit_count);
                        if (effective_count < input_pin_names.size())
                        {
                            input_pin_names.resize(effective_count);
                        }
                    }

                    auto tt = func.compute_truth_table(input_pin_names);
                    if (tt.is_error())
                    {
                        log_error("netlist", "Boolean function '{} = {}' cannot be added to LUT gate '{}' with ID {}.", name, func.to_string(), m_name, m_id);
                        return false;
                    }
                    auto truth_table = tt.get();
                    if (truth_table.size() > 1)
                    {
                        log_error("netlist", "Boolean function '{} = {}' cannot be added to LUT gate '{}' with ID {} (= function is > 1-bit in output size).", name, func.to_string(), m_name, m_id);
                        return false;
                    }

                    if (!output_cfg->is_ascending)
                    {
                        std::reverse(truth_table[0].begin(), truth_table[0].end());
                    }

                    u64 new_bits = 0;
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
                        new_bits <<= 1;
                        new_bits |= v;
                    }

                    // Ensure the base string has the full expected width before splicing.
                    const u32 total_inputs   = m_type->get_input_pins().size();
                    std::string existing_str = lut_init_read(this, key);
                    if (existing_str.empty())
                    {
                        existing_str = std::string(static_cast<size_t>(1u << (total_inputs - 2)), '0');
                    }

                    std::stringstream slice_ss;
                    slice_ss << std::hex << std::setfill('0') << std::setw(static_cast<int>((bit_count + 3) / 4)) << new_bits;

                    auto res = LUTComponent::splice_init_slice(existing_str, slice_ss.str(), bit_offset, bit_count);
                    if (res.is_error())
                    {
                        log_error("netlist", "cannot add Boolean function '{}' to LUT gate '{}' with ID {}: {}.", name, m_name, m_id, res.get_error().get());
                        return false;
                    }
                    lut_init_write(this, key, res.get());
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

    const std::vector<Net*>& Gate::get_fan_in_nets() const
    {
        return m_in_nets;
    }

    std::vector<Net*> Gate::get_fan_in_nets(const std::function<bool(Net*)>& filter) const
    {
        std::vector<Net*> res;
        if (!filter)
        {
            res = m_in_nets;
        }
        else
        {
            for (auto n : m_in_nets)
            {
                if (!filter(n))
                {
                    continue;
                }
                res.push_back(n);
            }
        }
        return res;
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

    const std::vector<Endpoint*>& Gate::get_fan_in_endpoints() const
    {
        return m_in_endpoints;
    }

    std::vector<Endpoint*> Gate::get_fan_in_endpoints(const std::function<bool(Endpoint*)>& filter) const
    {
        std::vector<Endpoint*> res;
        if (!filter)
        {
            res = m_in_endpoints;
        }
        else
        {
            for (auto ep : m_in_endpoints)
            {
                if (!filter(ep))
                {
                    continue;
                }
                res.push_back(ep);
            }
        }
        return res;
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

    const std::vector<Net*>& Gate::get_fan_out_nets() const
    {
        return m_out_nets;
    }

    std::vector<Net*> Gate::get_fan_out_nets(const std::function<bool(Net*)>& filter) const
    {
        std::vector<Net*> res;
        if (!filter)
        {
            res = m_out_nets;
        }
        else
        {
            for (auto n : m_out_nets)
            {
                if (!filter(n))
                {
                    continue;
                }
                res.push_back(n);
            }
        }
        return res;
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

    const std::vector<Endpoint*>& Gate::get_fan_out_endpoints() const
    {
        return m_out_endpoints;
    }

    std::vector<Endpoint*> Gate::get_fan_out_endpoints(const std::function<bool(Endpoint*)>& filter) const
    {
        std::vector<Endpoint*> res;
        if (!filter)
        {
            res = m_out_endpoints;
        }
        else
        {
            for (auto ep : m_out_endpoints)
            {
                if (!filter(ep))
                {
                    continue;
                }
                res.push_back(ep);
            }
        }
        return res;
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

}    // namespace hal
