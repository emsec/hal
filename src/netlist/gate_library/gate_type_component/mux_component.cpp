#include "hal_core/netlist/gate_library/gate_type_component/mux_component.h"

namespace hal
{
    MUXComponent::ComponentType MUXComponent::get_type() const
    {
        return m_type;
    }

    bool MUXComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> MUXComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        return {};
    }
}    // namespace hal