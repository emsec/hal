#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "netlist_simulator/netlist_simulator.h"

namespace hal
{
    class PLUGIN_API NetlistSimulatorPlugin : public BasePluginInterface
    {
    public:
        /**
         * Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /**
         * Create a netlist simulator instance.
         * 
         * @returns The simulator instance.
         */
        std::unique_ptr<NetlistSimulator> create_simulator() const;
    };
}    // namespace hal
