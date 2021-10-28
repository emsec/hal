#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"

#include <string>

namespace hal
{
    class IOGroup;

    class IOPin
    {
    public:
        IOPin(DataContainer* parent, const std::string& name, PinDirection direction, PinType type = PinType::none);

        DataContainer* get_parent() const;
        const std::string& get_name() const;
        PinDirection get_direction() const;
        PinType get_type() const;
        const std::pair<IOGroup*, u32>& get_group() const;

    private:
        IOPin(const IOPin&) = delete;
        IOPin(IOPin&&)      = delete;
        IOPin& operator=(const IOPin&) = delete;
        IOPin& operator=(IOPin&&) = delete;

    protected:
        DataContainer* m_parent;
        std::string m_name;
        PinDirection m_direction;
        PinType m_type;
        std::pair<IOGroup*, u32> m_group;
    };
}    // namespace hal