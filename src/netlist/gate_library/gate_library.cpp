#include "hal_core/netlist/gate_library/gate_library.h"

#include "hal_core/netlist/gate_library/gate_type/gate_type_lut.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_sequential.h"

namespace hal
{
    GateLibrary::GateLibrary(const std::filesystem::path& path, const std::string& name) : m_name(name), m_path(path)
    {
    }

    std::string GateLibrary::get_name() const
    {
        return m_name;
    }

    std::filesystem::path GateLibrary::get_path() const
    {
        return m_path;
    }

    GateType* GateLibrary::create_gate_type(const std::string& name, GateType::BaseType type)
    {
        std::unique_ptr<GateType> gt;

        switch (type)
        {
            case GateType::BaseType::combinatorial:
                gt = std::make_unique<GateType>(name);
                break;
            case GateType::BaseType::lut:
                gt = std::make_unique<GateTypeLut>(name);
                break;
            case GateType::BaseType::ff:
                gt = std::make_unique<GateTypeSequential>(name, type);
                break;
            case GateType::BaseType::latch:
                gt = std::make_unique<GateTypeSequential>(name, type);
                break;
        }

        auto res = gt.get();
        add_gate_type(std::move(gt));
        return res;
    }

    void GateLibrary::add_gate_type(std::unique_ptr<GateType> gt)
    {
        m_gate_type_map.emplace(gt->get_name(), gt.get());

        auto out_pins = gt->get_output_pins();

        if (gt->get_input_pins().empty() && (out_pins.size() == 1))
        {
            auto functions = gt->get_boolean_functions();
            auto it        = functions.find(out_pins[0]);
            if (it != functions.end())
            {
                auto bf = it->second;

                if (bf.is_constant_one())
                {
                    m_vcc_gate_types.emplace(gt->get_name(), gt.get());
                }
                else if (bf.is_constant_zero())
                {
                    m_gnd_gate_types.emplace(gt->get_name(), gt.get());
                }
            }
        }

        m_gate_types.push_back(std::move(gt));
    }

    bool GateLibrary::contains_gate_type(const GateType* gt) const
    {
        auto it = m_gate_type_map.find(gt->get_name());
        if (it == m_gate_type_map.end())
        {
            return false;
        }
        return *(it->second) == *gt;
    }

    std::unordered_map<std::string, const GateType*> GateLibrary::get_gate_types() const
    {
        return m_gate_type_map;
    }

    std::unordered_map<std::string, const GateType*> GateLibrary::get_vcc_gate_types() const
    {
        return m_vcc_gate_types;
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
}    // namespace hal
