#pragma once

#include "core/plugin_interface_base.h"
#include "core/plugin_interface_cli.h"

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;
    class Net;

    class PLUGIN_API plugin_test_plugin : virtual public CLIPluginInterface    //CLIPluginInterface
    {
    public:
        /** constructor (= default) */
        plugin_test_plugin() = default;

        /** destructor (= default) */
        ~plugin_test_plugin() = default;

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
        bool handle_cli_call(std::shared_ptr<Netlist> nl, ProgramArguments& args) override;
    };

}    // namespace hal
