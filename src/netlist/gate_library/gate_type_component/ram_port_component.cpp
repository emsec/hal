#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"

namespace hal
{
    RAMPortComponent::ComponentType RAMPortComponent::get_type() const
    {
        return m_type;
    }

    bool RAMPortComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> RAMPortComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        return {};
    }
}    // namespace hal