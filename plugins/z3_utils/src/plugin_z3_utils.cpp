#include "plugin_z3_utils.h"
#include "SubgraphFunctionGenerator.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<Z3UtilsPlugin>();
    }

    std::string Z3UtilsPlugin::get_name() const
    {
        return std::string("z3_utils");
    }

    std::string Z3UtilsPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void Z3UtilsPlugin::initialize()
    {

    }
}
