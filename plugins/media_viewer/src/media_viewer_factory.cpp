#include "media_viewer/media_viewer_factory.h"
#include "media_viewer/media_viewer.h"
#include "hal_core/netlist/project_manager.h"

#include "gui/gui_globals.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QMap>

namespace hal {

    MediaViewerFactory::MediaViewerFactory(const QString& name)
        : ContentFactory(name), ProjectSerializer(name.toStdString())
    {;}

    ExternalContentWidget* MediaViewerFactory::contentFactory() const
    {
        MediaViewer* mv = new MediaViewer(name());
        if (!mFilename.isEmpty()) mv->loadMediaFile(mFilename);
        return mv;
    }

    void MediaViewerFactory::restore()
    {
        ProjectManager* pm = ProjectManager::instance();
        std::string relname = pm->get_filename(m_name);

        // TODO restore
        // if (!relname.empty())
           // restoreMediaViewer(pm->get_project_directory(), relname);
    }

    std::string MediaViewerFactory::serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave)
    {
        Q_UNUSED(netlist);
        Q_UNUSED(isAutosave);
        QString mvFilename("media_viewer.json");
        QFile mvFile(QDir(QString::fromStdString(savedir.string())).absoluteFilePath(mvFilename));
        if (!mvFile.open(QIODevice::WriteOnly)) return std::string();


        MediaViewer* mv = MediaViewer::getMediaviewerInstance();
        if (!mv) return std::string();

        QJsonObject mvObj;
        std::filesystem::path relMediaPath = ProjectManager::instance()->get_project_directory().get_relative_file_path(mv->filename().toStdString());
        mvObj["filename"] = QString::fromStdString(relMediaPath);


        mvFile.write(QJsonDocument(mvObj).toJson(QJsonDocument::Compact));

        return mvFilename.toStdString();
    }

    void MediaViewerFactory::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        Q_UNUSED(netlist);
        std::string relname = ProjectManager::instance()->get_filename(m_name);
        if (!relname.empty())
            restoreMediaViewer(loaddir, relname);
    }

    void MediaViewerFactory::restoreMediaViewer(const std::filesystem::path& loaddir, const std::string& jsonfile)
    {
        QFile mvFile(QDir(QString::fromStdString(loaddir.string())).absoluteFilePath(QString::fromStdString(jsonfile)));
        if (!mvFile.open(QIODevice::ReadOnly))
            return;
        QJsonDocument jsonDoc   = QJsonDocument::fromJson(mvFile.readAll());
        const QJsonObject& json = jsonDoc.object();

        if (json.contains("filename") && json["filename"].isString())
        {
            std::filesystem::path mediafilePath(json["filename"].toString().toStdString());
            if (mediafilePath.is_relative())
                mediafilePath = ProjectManager::instance()->get_project_directory() / mediafilePath;
            mFilename = QString::fromStdString(mediafilePath.string());
        }

        MediaViewer* mv = MediaViewer::getMediaviewerInstance();
        if (!mv) return;

        if (!mFilename.isEmpty())
            mv->loadMediaFile(mFilename);
    }
}
