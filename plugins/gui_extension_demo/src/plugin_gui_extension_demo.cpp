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

    void GuiExtensionContext::clear()
    {
        m_python_plugins.clear();
        m_module_context_contribution.clear();
        m_gate_context_contribution.clear();
        m_net_context_contribution.clear();
        m_parameter.clear();
    }

    std::vector<ContextMenuContribution> GuiExtensionContext::get_context_contribution(const Netlist *nl,
                                                                          const std::vector<u32>& mods,
                                                                          const std::vector<u32>& gats,
                                                                          const std::vector<u32>& nets)
    {
        if (!nl)
            return std::vector<ContextMenuContribution>();

        if (mods.size() == 1 && gats.empty() && nets.empty() )
            return m_module_context_contribution;

        if (mods.empty() && gats.size() == 1 && nets.empty() )
            return m_gate_context_contribution;

        if (mods.empty() && gats.empty() && nets.size() == 1 )
            return m_net_context_contribution;

        return std::vector<ContextMenuContribution>();
    }

    void GuiExtensionContext::execute_function(std::string tag,
                         Netlist *nl,
                         const std::vector<u32>& mods,
                         const std::vector<u32>& gats,
                         const std::vector<u32>& nets)
    {
        Q_UNUSED(nl);

        if (tag.empty()) return; // not my call

        auto it = m_python_plugins.find(tag);
        if (it == m_python_plugins.end()) return;

        it->second->set_function_call(tag);
        it->second->set_selection(mods, gats, nets);
    }

    std::vector<PluginParameter> GuiExtensionContext::get_parameter() const
    {
        return m_parameter;
    }

    void GuiExtensionContext::set_parameter(const std::vector<PluginParameter>& params)
    {
        m_parameter = params;
    }

    void GuiExtensionContext::add_module_context(GuiExtensionPythonBase* plug, const std::string tagname, const std::string label)
    {
        m_module_context_contribution.push_back({this,tagname,label});
        m_python_plugins.insert(std::make_pair(tagname,plug));
    }

    void GuiExtensionContext::add_gate_context(GuiExtensionPythonBase* plug, const std::string tagname, const std::string label)
    {
        m_gate_context_contribution.push_back({this,tagname,label});
        m_python_plugins.insert(std::make_pair(tagname,plug));
    }

    void GuiExtensionContext::add_net_context(GuiExtensionPythonBase* plug, const std::string tagname, const std::string label)
    {
        m_net_context_contribution.push_back({this,tagname,label});
        m_python_plugins.insert(std::make_pair(tagname,plug));
    }

    void GuiExtensionContext::add_main_menu(GuiExtensionPythonBase* plug, const std::vector<PluginParameter>& params)
    {
        for (const PluginParameter& par : params)
        {
            m_parameter.push_back(par);
            if (par.get_type() == PluginParameter::PushButton)
                m_python_plugins.insert(std::make_pair(par.get_tagname(),plug));
        }
    }

}    // namespace hal
