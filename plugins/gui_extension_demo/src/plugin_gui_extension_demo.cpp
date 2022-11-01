#include "gui_extension_demo/plugin_gui_extension_demo.h"
#include "gui_extension_demo/gui_extension_python_base.h"
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
    GuiExtensionContext* GuiExtensionDemoPlugin::sGuiExtension = nullptr;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<GuiExtensionDemoPlugin>();
    }

    std::string GuiExtensionDemoPlugin::get_name() const
    {
        return std::string("gui_extension_demo");
    }

    std::string GuiExtensionDemoPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void GuiExtensionDemoPlugin::initialize()
    {
        std::cerr << "GuiExtensionDemoPlugin::initialize()" << std::endl;
    }

    void GuiExtensionDemoPlugin::on_load()
    {
        std::cerr << "GuiExtensionDemoPlugin::on_load()" << std::endl;
        sGuiExtension = new GuiExtensionContext;
    }

    void GuiExtensionDemoPlugin::on_unload()
    {
        delete sGuiExtension;
    }

    GuiExtensionInterface* GuiExtensionDemoPlugin::get_gui_extension() const
    {
        return sGuiExtension;
    }

//----------------------

    GuiExtensionContext::GuiExtensionContext()
    {
        std::cerr << "GuiExtensionContext::GuiExtensionContext" << std::endl;
    }

    GuiExtensionContext::~GuiExtensionContext()
    {
        std::cerr << "GuiExtensionContext::~GuiExtensionContext" << std::endl;
    }

    std::vector<ContextMenuContribution> GuiExtensionContext::get_context_contribution(const Netlist *nl,
                                                                          const std::vector<u32>& mods,
                                                                          const std::vector<u32>& gats,
                                                                          const std::vector<u32>& nets)
    {
        std::vector<ContextMenuContribution> retval;
        if (nl && mods.empty() && nets.empty() && gats.size() == 1 )
        {
            Gate *g = nl->get_gate_by_id(*gats.begin());
            retval.push_back({this,1,"Highlight gate '" + g->get_name() +"'by extension"});
        }
        return retval;
    }

    void GuiExtensionContext::execute_context_action(u32 fid,
                         Netlist *nl,
                         const std::vector<u32>&,
                         const std::vector<u32>& gats,
                         const std::vector<u32>&)
    {

        if (fid != 1) return; // not my call

        static int num = 0;
        ++num;
        Grouping* grp = nl->create_grouping("extension grp " + std::to_string(num));
        grp->set_color({0,255,255});
        Gate *g = nl->get_gate_by_id(*gats.begin());
        grp->assign_gate(g);
    }

    std::vector<PluginParameter> GuiExtensionContext::get_parameter() const
    {
        std::vector<PluginParameter> retval;
        for (auto it = m_python_plugins.begin(); it != m_python_plugins.end(); ++it)
        {
            std::string tag = it->second->get_tagname();
            retval.push_back(PluginParameter(PluginParameter::TabName, tag, it->second->get_label()));
            for (PluginParameter par : it->second->get_parameters())
            {
                par.set_tagname(tag + "/" + par.get_tagname());
                retval.push_back(par);
            }
            retval.push_back(PluginParameter(PluginParameter::PushButton, tag + "/exec", "Execute " + it->second->get_label()));
        }
        return retval;
    }

    void GuiExtensionContext::set_parameter(Netlist* nl, const std::vector<PluginParameter>& params)
    {
        std::string tag;
        for (PluginParameter par : params)
        {
            if (par.get_type() == PluginParameter::PushButton && par.get_value() == "clicked" && par.get_tagname().find("/exec") != std::string::npos)
            {
                tag = par.get_tagname().substr(0,par.get_tagname().find('/'));
                std::cerr << "set parameter <" << tag << ">" << std::endl;
                break;
            }
        }
        if (tag.empty()) return;

        auto it = m_python_plugins.find(tag);
        if (it == m_python_plugins.end()) return;

        std::string prefix = tag + '/';
        int n = prefix.size();
        std::vector<PluginParameter> plugPars;
        for (PluginParameter par : params)
        {
            if (par.get_tagname().substr(0,n) != prefix) continue;
            par.set_tagname(par.get_tagname().substr(n+1));
            plugPars.push_back(par);
        }
        it->second->set_parameters(plugPars);
        it->second->release_lock();
    }

    void GuiExtensionContext::register_external_extension(GuiExtensionPythonBase* plug)
    {
        std::string tag = plug->get_tagname();
        m_python_plugins.insert(std::make_pair(tag,plug));
    }


}    // namespace hal
