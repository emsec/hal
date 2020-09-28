#include "netlist_simulator/plugin_netlist_simulator.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistSimulatorPlugin>();
    }

    std::string NetlistSimulatorPlugin::get_name() const
    {
        return std::string("netlist_simulator");
    }

    std::string NetlistSimulatorPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::unique_ptr<NetlistSimulator> NetlistSimulatorPlugin::create_simulator() const
    {
        return std::unique_ptr<NetlistSimulator>(new NetlistSimulator());
    }
}
