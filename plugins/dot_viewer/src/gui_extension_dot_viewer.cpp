#include "dot_viewer/gui_extension_dot_viewer.h"
#include "dot_viewer/dot_viewer.h"

namespace hal {

    std::vector<PluginParameter> GuiExtensionDotViewer::get_parameter() const
    {
        std::vector<PluginParameter> retval;
        retval.push_back(PluginParameter(PluginParameter::Label,
                                         "help",
                                         "",
                                         "Enter name of .dot file to be loaded into viewer.\n\n"
                                         "The name of the plugin which created the .dot file\n"
                                         "is optional. If no plugin was specified the viewer\n"
                                         "will try to parse the name from file content.\n"
                                         "As fallback the interactive features get disabled.\n\n"));
        retval.push_back(PluginParameter(PluginParameter::ExistingFile, "filename", "Graphviz .dot file", ""));
        retval.push_back(PluginParameter(PluginParameter::String, "plugin", "File creator plugin", ""));
        retval.push_back(PluginParameter(PluginParameter::PushButton, "exec", "Load file"));
        return retval;
    }

    void GuiExtensionDotViewer::set_parameter(const std::vector<PluginParameter>& params)
    {
        QString filename;
        QString plugin;
        bool loadClicked = false;

        for (const PluginParameter& pp : params)
        {
            if (pp.get_tagname() == "filename")
                filename = QString::fromStdString(pp.get_value());
            else if (pp.get_tagname() == "plugin")
                plugin = QString::fromStdString(pp.get_value());
            else if (pp.get_tagname() == "exec")
                loadClicked = (pp.get_value() == "clicked");
        }
        if (loadClicked)
        {
            DotViewer* dv = DotViewer::getDotviewerInstance();
            if (dv) dv->loadDotFile(filename,plugin);
        }
    }

    void GuiExtensionDotViewer::netlist_about_to_close(Netlist*)
    {
        DotViewer* dv = DotViewer::getDotviewerInstance();
        if (dv) dv->disableInteractions();
    }
}
