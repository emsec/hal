#pragma once

#include "hal_core/plugin_system/plugin_interface_ui.h"

namespace hal
{
    class PluginPythonShell : virtual public UIPluginInterface
    {
    public:
        PluginPythonShell() = default;

        ~PluginPythonShell() = default;

        /*
         *      interface implementations
         */

        /**
         * Get the name of the plugin.
         *
         * @returns Plugin name.
         */
        std::string get_name() const override;

        /**
         * Get the version of the plugin.
         *
         * @returns Plugin version.
         */
        std::string get_version() const override;

        /**
         * Returns command line interface options
         *
         * @returns The program options description.
         */
        ProgramOptions get_cli_options() const override;

        /**
         * Excutes the plugin with given command line parameters.
         *
         * @param[in] args - The command line parameters.
         * @returns True on success.
         */
        bool exec(ProgramArguments& args) override;
    };
}    // namespace hal
