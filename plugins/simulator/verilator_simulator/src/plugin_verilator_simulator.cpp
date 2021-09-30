#include "verilator_simulator/plugin_verilator_simulator.h"

namespace hal {

extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
{
    return std::make_unique<VerilatorSimulatorPlugin>();
}

std::string VerilatorSimulatorPlugin::get_name() const
{
    return std::string("verilator_simulator");
}

std::string VerilatorSimulatorPlugin::get_version() const
{
    return std::string("0.1");
}


}
