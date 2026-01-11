#include "media_viewer/plugin_media_viewer.h"

#include "media_viewer/media_viewer_factory.h"
#include "media_viewer/media_viewer_factory.h"
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
        return std::make_unique<MediaViewerPlugin>();
    }

    MediaViewerPlugin::MediaViewerPlugin()
    {;}

    std::string MediaViewerPlugin::get_name() const
    {
        return std::string( "media_viewer" );
    }

    std::string MediaViewerPlugin::get_version() const
    {
        return std::string( "0.1" );
    }

    std::string MediaViewerPlugin::get_description() const
    {
        return "MediaViewer plugin for video display within HAL GUI";
    }

    void MediaViewerPlugin::on_load()
    {
        MediaViewerFactory* mvFactory = new MediaViewerFactory(QString::fromStdString(get_name()));
        ExternalContent::instance()->append(mvFactory);
        if (gNetlist) gContentManager->addExternalWidget(mvFactory);
    }

    void MediaViewerPlugin::on_unload()
    {
        QString pluginName = QString::fromStdString(get_name());
        ExternalContent::instance()->removePlugin(pluginName);
    }

    void MediaViewerPlugin::initialize()
    {
    }

    std::set<std::string> MediaViewerPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert( "hal_gui" );
        return retval;
    }
}  // namespace hal
