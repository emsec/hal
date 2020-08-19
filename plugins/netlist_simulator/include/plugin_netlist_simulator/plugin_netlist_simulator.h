#pragma once

#include "core/plugin_interface_base.h"
#include "netlist_simulator.h"

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
