#pragma once

#include "hal_core/plugin_system/plugin_interface_cli.h"

namespace hal
{
    /* forward declaration */
    class NetlistSimulatorController;
    class Gate;

    class PLUGIN_API PerfTestPlugin : public CLIPluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        /** interface implementation: i_cli */
        ProgramOptions get_cli_options() const override;

        /** interface implementation: i_cli */
        bool handle_cli_call(Netlist* nl, ProgramArguments& args) override;

        bool cmp_sim_data(NetlistSimulatorController* reference_simulation_ctrl, NetlistSimulatorController* simulation_ctrl, int tolerance = 200);
    };
}    // namespace hal
