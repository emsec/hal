#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/pins/base_pin.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <string>

namespace hal
{
    /**
     * The pin of a gate type. Each pin has a name, a direction, and a type. 
     * 
     * @ingroup pins
     */
    class GatePin : public BasePin<GatePin>
    {
    public:
        ~GatePin() = default;

    private:
        GatePin(const GatePin&) = delete;
        GatePin(GatePin&&)      = delete;
        GatePin& operator=(const GatePin&) = delete;
        GatePin& operator=(GatePin&&) = delete;

    protected:
        /**
         * Construct a new gate pin from its name, direction and type.
         * 
         * @param[in] name - The pin name.
         * @param[in] direction - The direction of the pin.
         * @param[in] type - The type of the pin.
         */
        GatePin(const u32 id, const std::string& name, PinDirection direction, PinType type = PinType::none);
    };
}    // namespace hal