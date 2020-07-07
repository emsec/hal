#include "netlist/gate_library/gate_library.h"

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

    void GateLibrary::add_gate_type(std::shared_ptr<const GateType> gt)
    {
        m_gate_type_map.emplace(gt->get_name(), gt);

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
                    m_vcc_gate_types.emplace(gt->get_name(), gt);
                }
                else if (bf.is_constant_zero())
                {
                    m_gnd_gate_types.emplace(gt->get_name(), gt);
                }
            }
        }
    }

    const std::map<std::string, std::shared_ptr<const GateType>>& GateLibrary::get_gate_types()
    {
        return m_gate_type_map;
    }

    const std::map<std::string, std::shared_ptr<const GateType>>& GateLibrary::get_vcc_gate_types()
    {
        return m_vcc_gate_types;
    }

    const std::map<std::string, std::shared_ptr<const GateType>>& GateLibrary::get_gnd_gate_types()
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
