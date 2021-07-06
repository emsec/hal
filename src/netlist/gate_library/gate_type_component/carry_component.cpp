#include "hal_core/netlist/gate_library/gate_type_component/carry_component.h"

namespace hal
{
    CarryComponent::ComponentType CarryComponent::get_type() const
    {
        return m_type;
    }

    bool CarryComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> CarryComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        return {};
    }
}    // namespace hal