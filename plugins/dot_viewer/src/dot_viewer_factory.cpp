#include "dot_viewer/dot_viewer_factory.h"
#include "dot_viewer/dot_viewer.h"
#include "hal_core/netlist/project_manager.h"

#include "gui/gui_globals.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QMap>

namespace hal {

    DotViewerFactory::DotViewerFactory(const QString& name)
        : ContentFactory(name), ProjectSerializer(name.toStdString())
    {;}

    ExternalContentWidget* DotViewerFactory::contentFactory() const
    {
        DotViewer* dv = new DotViewer(name());
        if (!mFilename.isEmpty()) dv->loadDotFile(mFilename, mCreator);
        return dv;
    }

    void DotViewerFactory::restore()
    {
        ProjectManager* pm = ProjectManager::instance();
        std::string relname = pm->get_filename(m_name);
        if (!relname.empty())
            restoreDotViewer(pm->get_project_directory(), relname);
    }

    std::string DotViewerFactory::serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave)
    {
        Q_UNUSED(netlist);
        Q_UNUSED(isAutosave);
        QString dvFilename("dot_viewer.json");
        QFile dvFile(QDir(QString::fromStdString(savedir.string())).absoluteFilePath(dvFilename));
        if (!dvFile.open(QIODevice::WriteOnly)) return std::string();


        DotViewer* dv = DotViewer::getDotviewerInstance();
        if (!dv) return std::string();

        QJsonObject dvObj;
        std::filesystem::path relDotPath = ProjectManager::instance()->get_project_directory().get_relative_file_path(dv->filename().toStdString());
        dvObj["filename"] = QString::fromStdString(relDotPath);
        dvObj["creator"] = dv->creatorPlugin();

        dvFile.write(QJsonDocument(dvObj).toJson(QJsonDocument::Compact));

        return dvFilename.toStdString();
    }

    void DotViewerFactory::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        Q_UNUSED(netlist);
        std::string relname = ProjectManager::instance()->get_filename(m_name);
        if (!relname.empty())
            restoreDotViewer(loaddir, relname);
    }

    void DotViewerFactory::restoreDotViewer(const std::filesystem::path& loaddir, const std::string& jsonfile)
    {
        QFile dvFile(QDir(QString::fromStdString(loaddir.string())).absoluteFilePath(QString::fromStdString(jsonfile)));
        if (!dvFile.open(QIODevice::ReadOnly))
            return;
        QJsonDocument jsonDoc   = QJsonDocument::fromJson(dvFile.readAll());
        const QJsonObject& json = jsonDoc.object();

        if (json.contains("filename") && json["filename"].isString())
        {
            std::filesystem::path dotfilePath(json["filename"].toString().toStdString());
            if (dotfilePath.is_relative())
                dotfilePath = ProjectManager::instance()->get_project_directory() / dotfilePath;
            mFilename = QString::fromStdString(dotfilePath.string());
        }

        if (json.contains("creator") && json["creator"].isString())
            mCreator = json["creator"].toString();

        DotViewer* dv = DotViewer::getDotviewerInstance();
        if (!dv) return;

        if (!mFilename.isEmpty())
            dv->loadDotFile(mFilename, mCreator);
    }
}
