#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include <unordered_map>
#include <memory>

namespace hal
{
    class PLUGIN_API CustomSimPlugin : public BasePluginInterface
    {
        static std::string s_engine_name;

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
         * Short description for plugin.
         *
         * @return The short description.
         */
        std::string get_description() const override;

        /**
         * Get dependencies of plugin
         * @return Set of plugin names which must be loaded first, contains {netlist_simulator_controller}
         */
        std::set<std::string> get_dependencies() const override;

        /**
         * Creates instance and registers with NetlistSimulatorController
         */
        void on_load() override;

        /**
         * Unregisters and delete instance
         */
        void on_unload() override;
    };
}    // namespace hal
