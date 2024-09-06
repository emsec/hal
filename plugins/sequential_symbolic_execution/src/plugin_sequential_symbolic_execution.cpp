#include "sequential_symbolic_execution/plugin_sequential_symbolic_execution.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<SequentialSymbolicExecutionPlugin>();
    }

    std::string SequentialSymbolicExecutionPlugin::get_name() const
    {
        return std::string("sequential_symbolic_execution");
    }

    std::string SequentialSymbolicExecutionPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string SequentialSymbolicExecutionPlugin::get_description() const
    {
        return "Allows to perform symbolic execution spanning multiple cycles and including sequential gates.";
    }

    std::set<std::string> SequentialSymbolicExecutionPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("z3_utils");
        return retval;
    }
}    // namespace hal
