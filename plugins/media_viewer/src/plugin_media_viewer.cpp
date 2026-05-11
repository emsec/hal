// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "media_viewer/plugin_media_viewer.h"

#include "media_viewer/gui_extension_media_viewer.h"
#include "media_viewer/media_viewer_factory.h"
#include "gui/gui_globals.h"

#include <memory>
#include <string>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<MediaViewerPlugin>();
    }

    MediaViewerPlugin::MediaViewerPlugin()
    {
        m_extensions.push_back(new GuiExtensionMediaViewer);
    }

    std::string MediaViewerPlugin::get_name() const
    {
        return std::string("media_viewer");
    }

    std::string MediaViewerPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string MediaViewerPlugin::get_description() const
    {
        return "Media Viewer plugin for playing local video files inside HAL";
    }

    void MediaViewerPlugin::initialize()
    {
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

    std::set<std::string> MediaViewerPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("hal_gui");
        return retval;
    }
}    // namespace hal
