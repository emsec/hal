#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

namespace hal
{
    class PLUGIN_API MachineLearningPlugin : public BasePluginInterface
    {
    public:
        /** 
         * @brief Default constructor for `MachineLearningPlugin`.
         */
        MachineLearningPlugin() = default;

        /** 
         * @brief Default destructor for `MachineLearningPlugin`.
         */
        ~MachineLearningPlugin() = default;

        /**
         * @brief Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * @brief Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /**
         * @brief Get a short description of the plugin.
         *
         * @returns The short description of the plugin.
         */
        std::string get_description() const override;

        /**
         * @brief Get the plugin dependencies.
         * 
         * @returns A set of plugin names that this plugin depends on.
         */
        std::set<std::string> get_dependencies() const override;
    };
}    // namespace hal
