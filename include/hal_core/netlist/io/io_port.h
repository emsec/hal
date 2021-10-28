#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/io/io_pin.h"
#include "hal_core/netlist/net.h"

#include <string>

namespace hal
{
    class IOPort : IOPin
    {
    public:
        IOPort(DataContainer* parent, const std::string& name, Net* net, PinDirection direction, PinType type = PinType::none);

        Net* get_net() const;

    private:
        friend Module;

        IOPort(const IOPort&) = delete;
        IOPort(IOPort&&)      = delete;
        IOPort& operator=(const IOPort&) = delete;
        IOPort& operator=(IOPort&&) = delete;

        Net* m_net;
    };
}    // namespace hal