#include "clock_tree_extractor/plugin_clock_tree_extractor.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<ClockTreeExtractorPlugin>();
    }

    std::string ClockTreeExtractorPlugin::get_name() const
    {
        return std::string( "clock_tree_extractor" );
    }

    std::string ClockTreeExtractorPlugin::get_version() const
    {
        return std::string( "0.1" );
    }

    std::string ClockTreeExtractorPlugin::get_description() const
    {
        return "Prototype plugin for extracting and visualizing the clock tree of a digital gate-level netlist.";
    }

    void ClockTreeExtractorPlugin::on_load()
    {
    }

    void ClockTreeExtractorPlugin::on_unload()
    {
    }

    void ClockTreeExtractorPlugin::initialize()
    {
    }

    std::set<std::string> ClockTreeExtractorPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        return retval;
    }
}  // namespace hal