#include "hal_core/netlist/gate_library/gate_type_component/buffer_component.h"

namespace hal
{
    BufferComponent::ComponentType BufferComponent::get_type() const
    {
        return m_type;
    }

    bool BufferComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> BufferComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        return {};
    }
}    // namespace hal