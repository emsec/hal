#pragma once

#include "hal_core/defines.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

#include <set>
#include <string>

namespace hal
{
    class Netlist;
}
namespace hal
{
    class GuiExtensionHelix;
}
namespace hal
{
    namespace helix
    {
        class Helix;
    }
}  // namespace hal

namespace hal
{
    extern Netlist *gNetlist;

    class PLUGIN_API HelixPlugin : public BasePluginInterface
    {
      public:
        HelixPlugin();

        ~HelixPlugin() = default;

        std::string get_name() const override;

        std::string get_version() const override;

        std::string get_description() const override;

        std::set<std::string> get_dependencies() const override;

        void initialize() override;

        void on_load() override;

        void on_unload() override;

        helix::Helix *get_helix();

      private:
        GuiExtensionHelix *m_gui_extension;

        helix::Helix *m_helix;
    };
}  // namespace hal
