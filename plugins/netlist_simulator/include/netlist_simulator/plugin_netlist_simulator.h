#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "netlist_simulator/netlist_simulator.h"

namespace hal
{
    class PLUGIN_API NetlistSimulatorPlugin : public BasePluginInterface
    {
    public:

        std::string get_name() const override;
        std::string get_version() const override;

        std::unique_ptr<NetlistSimulator> create_simulator() const;
    };
}
