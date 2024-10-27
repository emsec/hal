#pragma once
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

#define SECRET_PASSWORD "test12345"

/**
 * @brief Header file for the netlist modifier plugin. This plugin is used by the researcher to replace some gates with gates of an unknown type
 * 
 */

namespace hal
{
    extern Netlist* gNetlist;

    class GuiExtensionNetlistModifier;

    class PLUGIN_API NetlistModifierPlugin : public BasePluginInterface
    {
        GuiExtensionNetlistModifier* m_gui_extension;

        static bool replace_gate_in_netlist(Netlist* netlist, Gate* gate);
        static std::string obfuscated_gate_name(int num_in, int num_out, int num_io = 0);
        bool modify_gatelibrary();
        bool create_encrypted_zip(std::string password, int probe_type, int probe_limit, std::string salt = "", bool existing_hal_file = false);
        bool update_encrypted_zip(std::string password, int probe_type, int probe_limit);
        bool modify_in_place(int probe_type, int probe_limit);

    public:
        std::string get_name() const override;
        std::string get_version() const override;
        void initialize() override;

        void on_load() override;
        void on_unload() override;

        std::set<std::string> get_dependencies() const override;

        NetlistModifierPlugin();

        void open_popup();
    };

    class GuiExtensionNetlistModifier : public GuiExtensionInterface
    {
    public:
        NetlistModifierPlugin* m_parent;

        GuiExtensionNetlistModifier();

        std::vector<ContextMenuContribution> get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets) override;

        void execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets) override;
    };
}    // namespace hal
