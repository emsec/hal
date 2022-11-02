#include "plugin_smt_utils.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<SmtUtilsPlugin>();
    }

    std::string SmtUtilsPlugin::get_name() const
    {
        return std::string("smt_utils");
    }

    std::string SmtUtilsPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void SmtUtilsPlugin::initialize()
    {
    }
}    // namespace hal
