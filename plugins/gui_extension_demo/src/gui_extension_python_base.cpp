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

    void GuiExtensionPythonBase::add_parameter(const PluginParameter& param)
    {
        m_param.push_back(param);
    }

    void GuiExtensionPythonBase::register_extension()
    {
        if (GuiExtensionDemoPlugin::sGuiExtension)
        {
           GuiExtensionDemoPlugin::sGuiExtension->register_external_extension(this);
           m_mutex.lock();
        }
    }

    std::vector<PluginParameter> GuiExtensionPythonBase::get_parameters() const
    {
        return m_param;
    }


    void GuiExtensionPythonBase::set_parameters(const std::vector<PluginParameter>& params)
    {
        m_param = params;
    }

    void GuiExtensionPythonBase::release_lock()
    {
        m_mutex.unlock();
    }

    void GuiExtensionPythonBase::wait_for_gui()
    {
        std::cerr << "wait for gui ..." << std::endl;
        m_mutex.lock();
        m_mutex.unlock();
        std::cerr << "wait for gui done" << std::endl;
    }

}
