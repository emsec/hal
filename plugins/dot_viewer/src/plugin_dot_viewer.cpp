#include "dot_viewer/plugin_dot_viewer.h"

#include "dot_viewer/dot_viewer_factory.h"
#include "dot_viewer/gui_extension_dot_viewer.h"
#include "dot_viewer/dot_viewer_factory.h"
#include "gui/gui_globals.h"

#include <memory>
#include <string>

namespace hal
{
    class AbstractExtensionInterface;
}

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<DotViewerPlugin>();
    }

    DotViewerPlugin::DotViewerPlugin()
    {
        m_extensions.push_back(new GuiExtensionDotViewer);
    }

    std::string DotViewerPlugin::get_name() const
    {
        return std::string( "dot_viewer" );
    }

    std::string DotViewerPlugin::get_version() const
    {
        return std::string( "0.1" );
    }

    std::string DotViewerPlugin::get_description() const
    {
        return "DotViewer plugin for interactive display of graphviz .dot files";
    }

    void DotViewerPlugin::on_load()
    {
        DotViewerFactory* dvFactory = new DotViewerFactory(QString::fromStdString(get_name()));
        ExternalContent::instance()->append(dvFactory);
        if (gNetlist) gContentManager->addExternalWidget(dvFactory);
    }

    void DotViewerPlugin::on_unload()
    {
        QString pluginName = QString::fromStdString(get_name());
        ExternalContent::instance()->removePlugin(pluginName);
    }

    void DotViewerPlugin::initialize()
    {
    }

    std::set<std::string> DotViewerPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert( "hal_gui" );
        return retval;
    }
}  // namespace hal
