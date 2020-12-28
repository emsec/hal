#pragma once

#include "dataflow_analysis/processing/configuration.h"
#include "dataflow_analysis/utils/utils.h"
#include "hal_core/plugin_system/plugin_interface_cli.h"

#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;

    class PLUGIN_API plugin_dataflow : virtual public CLIPluginInterface
    {
    public:
        /*
         *      interface implementations
         */

        plugin_dataflow()  = default;
        ~plugin_dataflow() = default;

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

        /** interface implementation: i_cli */
        ProgramOptions get_cli_options() const override;

        /** interface implementation: i_cli */
        bool handle_cli_call(Netlist* nl, ProgramArguments& args) override;

        std::vector<std::vector<Gate*>>execute(Netlist* nl, std::string path, const u32 layer, const std::vector<u32> sizes);
    };
}    // namespace hal