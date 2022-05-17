#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"

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
                                                                                                                   {GateTypeComponent::ComponentType::ram_port, "ram_port"}};

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_lut_component(std::unique_ptr<GateTypeComponent> component, bool init_ascending)
    {
        if (component == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<LUTComponent>(std::move(component), init_ascending);
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_ff_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& next_state_bf, const BooleanFunction& clock_bf)
    {
        return std::make_unique<FFComponent>(std::move(component), next_state_bf.clone(), clock_bf.clone());
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_latch_component(std::unique_ptr<GateTypeComponent> component)
    {
        return std::make_unique<LatchComponent>(std::move(component));
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_ram_component(std::unique_ptr<GateTypeComponent> component, const u32 bit_size)
    {
        if (component == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<RAMComponent>(std::move(component), bit_size);
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_mac_component()
    {
        // TODO do fancy MAC stuff
        return std::make_unique<MACComponent>();
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_init_component(const std::string& init_category, const std::vector<std::string>& init_identifiers)
    {
        return std::make_unique<InitComponent>(init_category, init_identifiers);
    }

    std::unique_ptr<GateTypeComponent>
        GateTypeComponent::create_state_component(std::unique_ptr<GateTypeComponent> component, const std::string& state_identifier, const std::string& neg_state_identifier)
    {
        return std::make_unique<StateComponent>(std::move(component), state_identifier, neg_state_identifier);
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_ram_port_component(std::unique_ptr<GateTypeComponent> component,
                                                                                    const std::string& data_group,
                                                                                    const std::string& addr_group,
                                                                                    const BooleanFunction& clock_bf,
                                                                                    const BooleanFunction& enable_bf,
                                                                                    bool is_write)
    {
        return std::make_unique<RAMPortComponent>(std::move(component), data_group, addr_group, clock_bf.clone(), enable_bf.clone(), is_write);
    }

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