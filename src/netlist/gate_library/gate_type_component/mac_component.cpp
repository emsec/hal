#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"

#include <memory>

namespace hal
{
    std::unique_ptr<MACComponent> MACComponent::create()
    {
        // TODO do fancy MAC stuff
        return std::unique_ptr<MACComponent>(new MACComponent());
    }

    MACComponent::ComponentType MACComponent::get_type() const
    {
        return m_type;
    }

    bool MACComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::vector<GateTypeComponent*> MACComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        UNUSED(filter);
        return {};
    }
}    // namespace hal