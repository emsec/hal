#pragma once

#include "hal_core/defines.h"
#include "hal_core/plugin_system/gui_extension_interface.h"

#include <string>
#include <vector>

namespace hal
{
    class HelixPlugin;
}
namespace hal
{
    class Netlist;
}
namespace hal
{
    class PluginParameter;
}

namespace hal
{
    class GuiExtensionHelix : public GuiExtensionInterface
    {
      public:
        HelixPlugin *m_parent;

        GuiExtensionHelix();

        std::vector<PluginParameter> get_parameter() const override;

        void set_parameter( const std::vector<PluginParameter> &argv ) override;

        virtual std::vector<ContextMenuContribution> get_context_contribution( const Netlist *nl,
                                                                               const std::vector<u32> &mods,
                                                                               const std::vector<u32> &gats,
                                                                               const std::vector<u32> &nets ) override;

        virtual void execute_function( std::string tag,
                                       Netlist *nl,
                                       const std::vector<u32> &,
                                       const std::vector<u32> &,
                                       const std::vector<u32> & ) override;

      private:
        std::string m_host;
        u16 m_port;

        std::vector<std::string> m_channels;

        bool m_button_clicked;
    };
}  // namespace hal
