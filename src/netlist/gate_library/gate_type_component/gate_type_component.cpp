#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"

namespace hal
{
    template<>
    std::vector<std::string> EnumStrings<GateTypeComponent::ComponentType>::data = {"lut", "ff", "latch", "ram", "mac", "init", "ram_port"};

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
        if (component == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<FFComponent>(std::move(component), next_state_bf, clock_bf);
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_latch_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& data_in_bf, const BooleanFunction& enable_bf)
    {
        if (component == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<LatchComponent>(std::move(component), data_in_bf, enable_bf);
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_ram_component(std::unique_ptr<GateTypeComponent> component)
    {
        // TODO do fancy RAM stuff
        if (component == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<RAMComponent>(std::move(component));
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_mac_component()
    {
        // TODO do fancy MAC stuff
        return std::make_unique<MACComponent>();
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_init_component(const std::string& init_category, const std::string& init_identifier)
    {
        return std::make_unique<InitComponent>(init_category, init_identifier);
    }

    std::unique_ptr<GateTypeComponent> GateTypeComponent::create_ram_port_component()
    {
        // TODO do fancy RAM stuff
        return std::make_unique<RAMPortComponent>();
    }

    GateTypeComponent* GateTypeComponent::get_component(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        std::set<GateTypeComponent*> components = this->get_components(filter);

        if (components.size() == 1)
        {
            return *components.begin();
        }

        return nullptr;
    }
}    // namespace hal