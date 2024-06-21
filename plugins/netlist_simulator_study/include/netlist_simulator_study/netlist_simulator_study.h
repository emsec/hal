#pragma once

#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

namespace hal
{
    class NetlistSimulatorController;
    class GuiExtensionNetlistSimulatorStudy;

    class PLUGIN_API NetlistSimulatorStudyPlugin : public BasePluginInterface
    {
        GuiExtensionNetlistSimulatorStudy* m_gui_extension;
        std::unique_ptr<NetlistSimulatorController> m_simul_controller;

    public:
        std::unique_ptr<Netlist> m_original_netlist;
        NetlistSimulatorStudyPlugin();

        std::string get_name() const override;
        std::string get_version() const override;

        void on_load() override;
        void on_unload() override;

        /**
         * Returns plugin dependencies (GUI, simulation controller, verilator, waveform viewer)
         */
        std::set<std::string> get_dependencies() const override;

        bool simulate(std::filesystem::path sim_input, std::vector<const Net*> probes);
    };

    class GuiExtensionNetlistSimulatorStudy : public GuiExtensionInterface
    {
        std::vector<PluginParameter> m_parameter;

    public:
        NetlistSimulatorStudyPlugin* m_parent;

        GuiExtensionNetlistSimulatorStudy();

        std::vector<PluginParameter> get_parameter() const override;

        void set_parameter(const std::vector<PluginParameter>& params) override;
    };
}    // namespace hal