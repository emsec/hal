#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <string>

namespace hal
{
    class Module;

    /**
     * The pin of a module. Each pin has a name, a direction, and a type and is associated with a net. 
     * 
     * @ingroup pins
     */
    class ModulePin : public GatePin
    {
    public:
        /**
         * Get the net passing through the pin.
         * 
         * @returns The net of the pin.
         */
        Net* get_net() const;

        /**
         * Get the group of the pin as well as the index of the pin within the group.
         * 
         * @returns The group and the index of the pin.
         */
        const std::pair<PinGroup<ModulePin>*, u32>& get_group() const;

    private:
        friend Module;
        friend PinGroup<ModulePin>;

        Net* m_net;
        std::pair<PinGroup<ModulePin>*, u32> m_group = {nullptr, 0};

        ModulePin(const ModulePin&) = delete;
        ModulePin(ModulePin&&)      = delete;
        ModulePin& operator=(const ModulePin&) = delete;
        ModulePin& operator=(ModulePin&&) = delete;

        /**
         * Construct a new module pin from its name, net, direction and type.
         * 
         * @param[in] name - The pin name.
         * @param[in] net - The net passing through the pin.
         * @param[in] direction - The direction of the pin.
         * @param[in] type - The type of the pin.
         */
        ModulePin(const std::string& name, Net* net, PinDirection direction, PinType type = PinType::none);
    };
}    // namespace hal