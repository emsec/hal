#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"

namespace hal
{
    MACComponent::ComponentType MACComponent::get_type() const
    {
        return m_type;
    }

    bool MACComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> MACComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        return {};
    }
}    // namespace hal