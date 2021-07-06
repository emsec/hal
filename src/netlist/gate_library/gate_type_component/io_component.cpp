#include "hal_core/netlist/gate_library/gate_type_component/io_component.h"

namespace hal
{
    IOComponent::ComponentType IOComponent::get_type() const
    {
        return m_type;
    }

    bool IOComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> IOComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        return {};
    }
}    // namespace hal