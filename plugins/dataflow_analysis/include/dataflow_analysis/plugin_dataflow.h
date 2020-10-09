#pragma once

#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "dataflow_analysis/processing/configuration.h"
#include "dataflow_analysis/utils/utils.h"

#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;
    struct Context;

    class PLUGIN_API plugin_dataflow : virtual public CLIPluginInterface
    {
    public:
        /*
     *      interface implementations
     */

        plugin_dataflow()  = default;
        ~plugin_dataflow() = default;

        /** interface implementation: i_base */
        std::string get_name() const override;

        /** interface implementation: i_base */
        std::string get_version() const override;

        /** interface implementation: i_cli */
        ProgramOptions get_cli_options() const override;

        /** interface implementation: i_cli */
        bool handle_cli_call(Netlist* nl, ProgramArguments& args) override;

        bool execute(Netlist* nl, std::string path, const u32 layer, const std::vector<u32> sizes);

    };
}    // namespace hal
