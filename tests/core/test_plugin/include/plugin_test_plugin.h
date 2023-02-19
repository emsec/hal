#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/plugin_system/cli_extension_interface.h"

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;
    class Net;

    class PluginTestPlugin;

    class CliExtensionTestPlugin : public CliExtensionInterface
    {
        PluginTestPlugin* mParent;
    public:
        CliExtensionTestPlugin(PluginTestPlugin* p) { mParent = p; }

        /** interface implementation: CliExtensionInterface */
        ProgramOptions get_cli_options() const override;

        /** interface implementation: CliExtensionInterface */
        bool handle_cli_call(Netlist* nl, ProgramArguments& args) override;
    };

    class PLUGIN_API PluginTestPlugin : public BasePluginInterface    //BasePluginInterface
    {
        CliExtensionTestPlugin* mCliExtensions;
    public:
        /** constructor (= default) */
        PluginTestPlugin() { mCliExtensions = new CliExtensionTestPlugin(this); }

        /** destructor (= default) */
        ~PluginTestPlugin() { delete mCliExtensions; }

        /*
         *      interface implementations
         */

        /** interface implementation: BasePluginInterface */
        std::string get_name() const override;

        /** interface implementation: BasePluginInterface */
        std::string get_version() const override;

        std::vector<AbstractExtensionInterface *> get_extensions() const override {
            return std::vector<AbstractExtensionInterface*>({mCliExtensions}); }
    };

}    // namespace hal
