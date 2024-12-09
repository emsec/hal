#include "machine_learning/plugin_machine_learning.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<MachineLearningPlugin>();
    }

    std::string MachineLearningPlugin::get_name() const
    {
        return std::string("machine_learning");
    }

    std::string MachineLearningPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string MachineLearningPlugin::get_description() const
    {
        return "TODO";
    }

    std::set<std::string> MachineLearningPlugin::get_dependencies() const
    {
        return {"graph_algorithm", "netlist_preprocessing"};
    }
}    // namespace hal
