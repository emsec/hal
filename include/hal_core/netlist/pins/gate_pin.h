#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <string>

namespace hal
{
    class GatePin
    {
    public:
        GatePin(const std::string& name, PinDirection direction, PinType type = PinType::none);
        virtual ~GatePin() = default;

        const std::string& get_name() const;
        PinDirection get_direction() const;
        PinType get_type() const;
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
    };
}    // namespace hal