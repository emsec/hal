#pragma once
#include "hal_core/defines.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class GuiExtensionNetlistModifier;

    class PLUGIN_API NetlistModifierPlugin : public BasePluginInterface
    {
        GuiExtensionNetlistModifier* m_gui_extension;

        static bool replace_gate_in_netlist(Netlist* netlist, Gate* gate);
        static std::string obfuscated_gate_name(int num_in, int num_out, int num_io=0);
        bool modify_gatelibrary();
    public:
        std::string get_name() const override;
        std::string get_version() const override;
        void initialize() override;

        void on_load() override;
        void on_unload() override;

        std::set<std::string> get_dependencies() const override;

        NetlistModifierPlugin();

        bool modify_in_place();
        bool save();
    };

    class GuiExtensionNetlistModifier : public GuiExtensionInterface
    {
        std::vector<PluginParameter> m_parameter;

    public:
        NetlistModifierPlugin* m_parent;

        GuiExtensionNetlistModifier();

        std::vector<PluginParameter> get_parameter() const override;

        void set_parameter(const std::vector<PluginParameter>& params) override;
    };
}    // namespace hal
