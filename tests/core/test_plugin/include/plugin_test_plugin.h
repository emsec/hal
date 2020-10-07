#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/plugin_system/plugin_interface_cli.h"

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;
    class Net;

    class PLUGIN_API PluginTestPlugin : public CLIPluginInterface    //CLIPluginInterface
    {
    public:
        /** constructor (= default) */
        PluginTestPlugin() = default;

        /** destructor (= default) */
        ~PluginTestPlugin() = default;

        /*
     *      interface implementations
     */

        /** interface implementation: BasePluginInterface */
        std::string get_name() const override;

        /** interface implementation: BasePluginInterface */
        std::string get_version() const override;

        /** interface implementation: CLIPluginInterface */
        ProgramOptions get_cli_options() const override;

        /** interface implementation: CLIPluginInterface */
        bool handle_cli_call(Netlist* nl, ProgramArguments& args) override;
    };

}    // namespace hal
