#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"

namespace hal
{
    InitComponent::InitComponent(const std::string& init_category, const std::vector<std::string>& init_identifiers) : m_init_category(init_category), m_init_identifiers(init_identifiers)
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

    std::vector<GateTypeComponent*> InitComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        UNUSED(filter);
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

    const std::vector<std::string>& InitComponent::get_init_identifiers() const
    {
        return m_init_identifiers;
    }

    void InitComponent::set_init_identifiers(const std::vector<std::string>& init_identifiers)
    {
        m_init_identifiers = init_identifiers;
    }
}    // namespace hal