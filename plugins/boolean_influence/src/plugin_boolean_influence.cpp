#include "boolean_influence/plugin_boolean_influence.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<BooleanInfluencePlugin>();
    }

    std::string BooleanInfluencePlugin::get_name() const
    {
        return std::string("module_identification");
    }

    std::string BooleanInfluencePlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string BooleanInfluencePlugin::get_description() const
    {
        return std::string("Set of functions to determine the influence of variables of a Boolean function on its output.");
    }

    std::set<std::string> BooleanInfluencePlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("z3_utils");
        return retval;
    }

}    // namespace hal
