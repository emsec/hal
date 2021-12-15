#pragma once

#include "hal_core/plugin_system/plugin_interface_cli.h"

namespace hal
{
    class PLUGIN_API TestBooleanFunctionPlugin : virtual public CLIPluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        /** interface implementation: i_cli */
        ProgramOptions get_cli_options() const override;

        /** interface implementation: i_cli */
        bool handle_cli_call(Netlist* nl, ProgramArguments& args) override;

        void initialize() override;
    };
}    // namespace hal
