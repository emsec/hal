#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/pins/base_pin.h"
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
    class ModulePin : public BasePin<ModulePin>
    {
    public:
        /**
         * Construct a new module pin from its name, net, direction and type.
         * 
         * @param[in] id - The pin ID.
         * @param[in] name - The pin name.
         * @param[in] net - The net passing through the pin.
         * @param[in] direction - The direction of the pin.
         * @param[in] type - The type of the pin.
         */
        ModulePin(const u32 id, const std::string& name, Net* net, PinDirection direction, PinType type = PinType::none);

        /**
         * TODO pybind, test
         * Check whether two module pins are equal.
         *
         * @param[in] other - The module pin to compare against.
         * @returns True if both module pins are equal, false otherwise.
         */
        bool operator==(const ModulePin& other) const;

        /**
         * TODO pybind, test
         * Check whether two module pins are unequal.
         *
         * @param[in] other - The module pin to compare against.
         * @returns True if both module pins are unequal, false otherwise.
         */
        bool operator!=(const ModulePin& other) const;

        /**
         * TODO test
         * Get the net passing through the pin.
         * 
         * @returns The net of the pin.
         */
        Net* get_net() const;

    private:
        Net* m_net;

        ModulePin(const ModulePin&) = delete;
        ModulePin(ModulePin&&)      = delete;
        ModulePin& operator=(const ModulePin&) = delete;
        ModulePin& operator=(ModulePin&&) = delete;
    };
}    // namespace hal