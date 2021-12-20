#include "hal_core/netlist/gate_library/gate_library.h"

#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    GateLibrary::GateLibrary(const std::filesystem::path& path, const std::string& name) : m_name(name), m_path(path)
    {
        m_next_gate_type_id = 1;
    }

    std::string GateLibrary::get_name() const
    {
        return m_name;
    }

    std::filesystem::path GateLibrary::get_path() const
    {
        return m_path;
    }

    void GateLibrary::set_gate_location_data_category(const std::string& category)
    {
        m_gate_location_data_category = category;
    }

    const std::string& GateLibrary::get_gate_location_data_category() const
    {
        return m_gate_location_data_category;
    }

    void GateLibrary::set_gate_location_data_identifiers(const std::string& x_coordinate, const std::string& y_coordinate)
    {
        m_gate_location_data_identifiers = std::make_pair(x_coordinate, y_coordinate);
    }

    const std::pair<std::string, std::string>& GateLibrary::get_gate_location_data_identifiers() const
    {
        return m_gate_location_data_identifiers;
    }

    GateType* GateLibrary::create_gate_type(const std::string& name, std::set<GateTypeProperty> properties, std::unique_ptr<GateTypeComponent> component)
    {
        if (m_gate_type_map.find(name) != m_gate_type_map.end())
        {
            log_error("gate_library", "could not create gate type with name '{}' as a gate type with the same name already exists within gate library '{}'.", name, m_name);
            return nullptr;
        }

        std::unique_ptr<GateType> gt = std::unique_ptr<GateType>(new GateType(this, get_unique_gate_type_id(), name, properties, std::move(component)));

        auto res = gt.get();
        m_gate_type_map.emplace(name, res);
        m_gate_types.push_back(std::move(gt));
        return res;
    }

    bool GateLibrary::contains_gate_type(GateType* gate_type) const
    {
        if (gate_type == nullptr)
        {
            return false;
        }

        auto it = m_gate_type_map.find(gate_type->get_name());
        if (it == m_gate_type_map.end())
        {
            return false;
        }
        return *(it->second) == *gate_type;
    }

    bool GateLibrary::contains_gate_type_by_name(const std::string& name) const
    {
        if (auto it = m_gate_type_map.find(name); it != m_gate_type_map.end())
        {
            return true;
        }

        return false;
    }

    GateType* GateLibrary::get_gate_type_by_name(const std::string& name) const
    {
        if (auto it = m_gate_type_map.find(name); it != m_gate_type_map.end())
        {
            return it->second;
        }

        log_error("gate_library", "could not find the specified gate type, as there exists no gate type called '{}' within gate library '{}'.", name, m_name);
        return nullptr;
    }

    std::unordered_map<std::string, GateType*> GateLibrary::get_gate_types(const std::function<bool(const GateType*)>& filter) const
    {
        if (filter)
        {
            std::unordered_map<std::string, GateType*> res;
            for (const auto& type : m_gate_types)
            {
                if (filter(type.get()))
                {
                    res[type->get_name()] = type.get();
                }
            }
            return res;
        }

        return m_gate_type_map;
    }

    bool GateLibrary::mark_vcc_gate_type(GateType* gate_type)
    {
        auto out_pins = gate_type->get_output_pins();

        if (gate_type->get_input_pins().empty() && (out_pins.size() == 1))
        {
            auto functions = gate_type->get_boolean_functions();
            auto it        = functions.find(out_pins[0]);
            if (it != functions.end())
            {
                auto bf = it->second;

                if (bf.has_constant_value(1))
                {
                    m_vcc_gate_types.emplace(gate_type->get_name(), gate_type);
                    return true;
                }
            }
        }

        return false;
    }

    std::unordered_map<std::string, GateType*> GateLibrary::get_vcc_gate_types() const
    {
        return m_vcc_gate_types;
    }

    bool GateLibrary::mark_gnd_gate_type(GateType* gate_type)
    {
        auto out_pins = gate_type->get_output_pins();

        if (gate_type->get_input_pins().empty() && (out_pins.size() == 1))
        {
            auto functions = gate_type->get_boolean_functions();
            auto it        = functions.find(out_pins[0]);
            if (it != functions.end())
            {
                auto bf = it->second;

                if (bf.has_constant_value(0))
                {
                    m_gnd_gate_types.emplace(gate_type->get_name(), gate_type);
                    return true;
                }
            }
        }

        return false;
    }

    std::unordered_map<std::string, GateType*> GateLibrary::get_gnd_gate_types() const
    {
        return m_gnd_gate_types;
    }

    std::vector<std::string> GateLibrary::get_includes() const
    {
        return m_includes;
    }

    void GateLibrary::add_include(const std::string& inc)
    {
        m_includes.push_back(inc);
    }

    u32 GateLibrary::get_unique_gate_type_id()
    {
        return m_next_gate_type_id++;
    }
}    // namespace hal
