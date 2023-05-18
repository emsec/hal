#include "xilinx_toolbox/plugin_xilinx_toolbox.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<XilinxToolboxPlugin>();
    }

    std::string XilinxToolboxPlugin::get_name() const
    {
        return std::string("xilinx_toolbox");
    }

    std::string XilinxToolboxPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void XilinxToolboxPlugin::initialize()
    {

    }
}
