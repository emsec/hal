#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

namespace hal
{
    template<>
    std::map<GateTypeComponent::ComponentType, std::string> EnumStrings<GateTypeComponent::ComponentType>::data = {{GateTypeComponent::ComponentType::lut, "lut"},
                                                                                                                   {GateTypeComponent::ComponentType::ff, "ff"},
                                                                                                                   {GateTypeComponent::ComponentType::latch, "latch"},
                                                                                                                   {GateTypeComponent::ComponentType::ram, "ram"},
                                                                                                                   {GateTypeComponent::ComponentType::mac, "mac"},
                                                                                                                   {GateTypeComponent::ComponentType::init, "init"},
                                                                                                                   {GateTypeComponent::ComponentType::state, "state"},
                                                                                                                   {GateTypeComponent::ComponentType::ram_port, "ram_port"},
                                                                                                                   {GateTypeComponent::ComponentType::state_table, "state_table"}};

    GateTypeComponent* GateTypeComponent::get_component(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        std::vector<GateTypeComponent*> components = this->get_components(filter);

        if (components.size() == 1)
        {
            return *components.begin();
        }

        return nullptr;
    }
}    // namespace hal