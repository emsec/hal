#include "extend_context/plugin_extend_context.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_data.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"

#include <thread>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<ExtendContextPlugin>();
    }

    std::string ExtendContextPlugin::get_name() const
    {
        return std::string("extend_context");
    }

    std::string ExtendContextPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void ExtendContextPlugin::initialize()
    {
    }

    std::vector<ContextMenuContribution> ExtendContextPlugin::get_context_contribution(const Netlist *nl,
                                                                          const std::unordered_set<u32>& mods,
                                                                          const std::unordered_set<u32>& gats,
                                                                          const std::unordered_set<u32>& nets)
    {
        std::vector<ContextMenuContribution> retval;
        if (nl && mods.empty() && nets.empty() && gats.size() == 1 )
        {
            Gate *g = nl->get_gate_by_id(*gats.begin());
            retval.push_back({1,"Highlight gate '" + g->get_name() +"'by extension"});
        }
        return retval;
    }

    void ExtendContextPlugin::execute(u32 fid,
                         Netlist *nl,
                         const std::unordered_set<u32>&,
                         const std::unordered_set<u32>& gats,
                         const std::unordered_set<u32>&)
    {

        if (fid != 1) return; // not my call

        static int num = 0;
        ++num;
        Grouping* grp = nl->create_grouping("extension grp " + std::to_string(num));
        grp->set_color({0,255,255});
        Gate *g = nl->get_gate_by_id(*gats.begin());
        grp->assign_gate(g);
    }

}    // namespace hal
