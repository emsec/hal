#include "boolean_influence/plugin_boolean_influence.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<BooleanInfluencePlugin>();
    }

    std::string BooleanInfluencePlugin::get_name() const
    {
        return std::string("boolean_influence");
    }

    std::string BooleanInfluencePlugin::get_version() const
    {
        return std::string("0.1");
    }

    void BooleanInfluencePlugin::initialize()
    {
    }

}    // namespace hal
