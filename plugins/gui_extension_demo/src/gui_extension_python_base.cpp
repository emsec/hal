#include "hal_core/python_bindings/python_bindings.h"
#include "gui_extension_demo/gui_extension_python_base.h"
#include "gui_extension_demo/plugin_gui_extension_demo.h"
#include <iostream>

namespace hal {

    GuiExtensionPythonBase::GuiExtensionPythonBase(const std::string& tag, const std::string &lab)
        : m_tagname(tag), m_label(lab)
    {
        std::cerr << "GuiExtensionPythonBase::GuiExtensionPythonBase <" << tag << "> <" << lab << ">" << std::endl;
    }

    GuiExtensionPythonBase::~GuiExtensionPythonBase()
    {
        std::cerr << "GuiExtensionPythonBase::~GuiExtensionPythonBase <" << m_tagname << "> <" << m_label << ">" << std::endl;
    }

    void GuiExtensionPythonBase::add_main_menu(const std::vector<PluginParameter> &params)
    {
        if (GuiExtensionDemoPlugin::sGuiExtension)
        {
            GuiExtensionDemoPlugin::sGuiExtension->add_main_menu(this,params);
        }
    }

    void GuiExtensionPythonBase::add_module_context(const std::string tagname, const std::string label)
    {
        if (GuiExtensionDemoPlugin::sGuiExtension)
        {
            GuiExtensionDemoPlugin::sGuiExtension->add_module_context(this,tagname,label);
        }
    }

    void GuiExtensionPythonBase::add_gate_context(const std::string tagname, const std::string label)
    {
        if (GuiExtensionDemoPlugin::sGuiExtension)
        {
            GuiExtensionDemoPlugin::sGuiExtension->add_gate_context(this,tagname,label);
        }
    }

    void GuiExtensionPythonBase::add_net_context(const std::string tagname, const std::string label)
    {
        if (GuiExtensionDemoPlugin::sGuiExtension)
        {
            GuiExtensionDemoPlugin::sGuiExtension->add_net_context(this,tagname,label);
        }
    }


    void GuiExtensionPythonBase::set_selection(const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets)
    {
        m_mods_selected = mods;
        m_gats_selected = gats;
        m_nets_selected = nets;
    }

    std::vector<u32> GuiExtensionPythonBase::get_selected_modules() const
    {
        return m_mods_selected;
    }

    std::vector<u32> GuiExtensionPythonBase::get_selected_gates() const
    {
        return m_gats_selected;
    }

    std::vector<u32> GuiExtensionPythonBase::get_selected_nets() const
    {
        return m_nets_selected;
    }

    void GuiExtensionPythonBase::set_function_call(const std::string& fc)
    {
        m_function_call = fc;
    }

    std::string GuiExtensionPythonBase::get_function_call() const
    {
        return m_function_call;
    }

    std::vector<PluginParameter> GuiExtensionPythonBase::get_parameter() const
    {
        if (GuiExtensionDemoPlugin::sGuiExtension)
        {
            return GuiExtensionDemoPlugin::sGuiExtension->get_parameter();
        }
        return std::vector<PluginParameter>();
    }

    void GuiExtensionPythonBase::clear()
    {
        m_tagname.clear();
        m_label.clear();

        m_mods_selected.clear();
        m_gats_selected.clear();
        m_nets_selected.clear();

        m_function_call.clear();

        if (GuiExtensionDemoPlugin::sGuiExtension)
        {
           GuiExtensionDemoPlugin::sGuiExtension->clear();
        }
    }
}
