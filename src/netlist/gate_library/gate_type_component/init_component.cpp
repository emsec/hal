#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"

namespace hal
{
    InitComponent::InitComponent(const std::string& init_category, const std::string& init_identifier) : m_init_category(init_category), m_init_identifier(init_identifier)
    {
    }

    InitComponent::ComponentType InitComponent::get_type() const
    {
        return m_type;
    }

    bool InitComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> InitComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        return {};
    }

    const std::string& InitComponent::get_init_category() const
    {
        return m_init_category;
    }

    void InitComponent::set_init_category(const std::string& init_category)
    {
        m_init_category = init_category;
    }

    const std::string InitComponent::get_init_identifier() const
    {
        return m_init_identifier;
    }

    void InitComponent::set_init_identifier(const std::string& init_identifier)
    {
        m_init_identifier = init_identifier;
    }
}    // namespace hal