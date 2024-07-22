#pragma once
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"

#define SECRET_PASSWORD "test12345"

namespace hal
{
    extern Netlist* gNetlist;

    class NetlistSimulatorController;
    class GuiExtensionNetlistSimulatorStudy;

    class PLUGIN_API NetlistSimulatorStudyPlugin : public BasePluginInterface
    {
        GuiExtensionNetlistSimulatorStudy* m_gui_extension;
        std::unique_ptr<NetlistSimulatorController> m_simul_controller;
        std::string gen_salted_password(std::string password, std::string salt);
        std::vector<std::tuple<std::string, std::string>> read_all_zip_files_decrypted(std::filesystem::path zip_path, std::string password, std::string filename = "");
        std::string read_named_zip_file_decrypted(std::filesystem::path zip_path, std::string password, std::string filename);

        void init_simulation(std::string input_file);
        bool simulate(std::filesystem::path sim_input, std::vector<const Net*> probes);

        std::unique_ptr<Netlist> m_original_netlist;

    public:
        std::string get_name() const override;
        std::string get_version() const override;
        void initialize() override;

        NetlistSimulatorStudyPlugin();

        void on_load() override;
        void on_unload() override;

        std::set<std::string> get_dependencies() const override;

        void open_popup();
    };

    class GuiExtensionNetlistSimulatorStudy : public GuiExtensionInterface
    {
    public:
        NetlistSimulatorStudyPlugin* m_parent;

        GuiExtensionNetlistSimulatorStudy();

        std::vector<ContextMenuContribution> get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets) override;

        void execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets) override;

        /**
         * Register function to indicate work progress when busy
         * @param pif Progress Indicator Function to register
         */
        static std::function<void(int, const std::string&)> s_progress_indicator_function;
        virtual void register_progress_indicator(std::function<void(int, const std::string&)> pif) override;
    };
}    // namespace hal
