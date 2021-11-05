#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <string>

namespace hal
{
    /**
     * The pin of a gate type. Each pin has a name, a direction, and a type. 
     * 
     * @ingroup pins
     */
    class GatePin
    {
    public:
        virtual ~GatePin() = default;

        /**
         * Get the name of the pin.
         * 
         * @returns The name of the pin.
         */
        const std::string& get_name() const;

        /**
         * Get the direction of the pin.
         * 
         * @returns The direction of the pin.
         */
        PinDirection get_direction() const;

        /**
         * Get the type of the pin.
         * 
         * @returns The type of the pin.
         */
        PinType get_type() const;

        /**
         * Get the group of the pin as well as the index of the pin within the group.
         * 
         * @returns The group and the index of the pin.
         */
        const std::pair<PinGroup<GatePin>*, u32>& get_group() const;

    private:
        friend PinGroup<GatePin>;

        std::pair<PinGroup<GatePin>*, u32> m_group = {nullptr, 0};

        GatePin(const GatePin&) = delete;
        GatePin(GatePin&&)      = delete;
        GatePin& operator=(const GatePin&) = delete;
        GatePin& operator=(GatePin&&) = delete;

    protected:
        std::string m_name;
        PinDirection m_direction;
        PinType m_type;

        /**
         * Construct a new gate pin from its name, direction and type.
         * 
         * @param[in] name - The pin name.
         * @param[in] direction - The direction of the pin.
         * @param[in] type - The type of the pin.
         */
        GatePin(const std::string& name, PinDirection direction, PinType type = PinType::none);
    };
}    // namespace hal