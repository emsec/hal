#include "gui/grouping/grouping_color_serializer.h"
#include "hal_core/netlist/project_manager.h"

#include "gui/gui_globals.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/module_model/module_model.h"
#include "gui/module_model/module_item.h"
#include "gui/module_model/module_color_manager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QMap>

namespace hal {

    GroupingColorSerializer::GroupingColorSerializer()
        : ProjectSerializer("groupingcolor")
    {;}

    void GroupingColorSerializer::restore(GroupingTableModel* gtm)
    {
        ProjectManager* pm = ProjectManager::instance();
        std::string relname = pm->get_filename(m_name);
        if (!relname.empty())
            restoreGroupingColor(pm->get_project_directory(), relname, gtm);
    }

    std::string GroupingColorSerializer::serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave)
    {
        Q_UNUSED(netlist);
        Q_UNUSED(isAutosave);
        QString gcFilename("groupingcolor.json");
        QFile gcFile(QDir(QString::fromStdString(savedir.string())).absoluteFilePath(gcFilename));
        if (!gcFile.open(QIODevice::WriteOnly)) return std::string();

        QJsonObject gcObj;
        QJsonArray  gcArr;

        const GroupingManagerWidget* gmw = gContentManager->getGroupingManagerWidget();
        if (!gmw) std::string();
        const GroupingTableModel* gtm = gmw->getModel();
        if (!gtm) std::string();

        for (int irow=0; irow<gtm->rowCount(); irow++)
        {
            GroupingTableEntry gtme = gtm->groupingAt(irow);
            QJsonObject gcEntry;
            gcEntry["id"] = (int)gtme.id();
            gcEntry["color"] = gtme.color().name(QColor::HexArgb);
            gcArr.append(gcEntry);
        }

        gcObj["grpcolors"] = gcArr;

        gcFile.write(QJsonDocument(gcObj).toJson(QJsonDocument::Compact));

        return gcFilename.toStdString();
    }

    void GroupingColorSerializer::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        Q_UNUSED(netlist);
        std::string relname = ProjectManager::instance()->get_filename(m_name);
        if (!relname.empty())
            restoreGroupingColor(loaddir, relname);
    }

    void GroupingColorSerializer::restoreGroupingColor(const std::filesystem::path& loaddir, const std::string& jsonfile, GroupingTableModel* gtm)
    {
        if (!gtm)
        {
            const GroupingManagerWidget* gmw = gContentManager->getGroupingManagerWidget();
            if (!gmw) return;
            gtm = gmw->getModel();
            if (!gtm) return;
        }

        QFile gcFile(QDir(QString::fromStdString(loaddir.string())).absoluteFilePath(QString::fromStdString(jsonfile)));
        if (!gcFile.open(QIODevice::ReadOnly))
            return;
        QJsonDocument jsonDoc   = QJsonDocument::fromJson(gcFile.readAll());
        const QJsonObject& json = jsonDoc.object();

        QMap<int,QColor> colorMap;
        if (json.contains("grpcolors") && json["grpcolors"].isArray())
        {
            QJsonArray gcArr = json["grpcolors"].toArray();
            int ngc          = gcArr.size();
            for (int igc = 0; igc < ngc; igc++)
            {
                QJsonObject gcEntry = gcArr.at(igc).toObject();
                colorMap[gcEntry["id"].toInt()] = QColor(gcEntry["color"].toString());
            }
        }

        for (int irow=0; irow < gtm->rowCount(); irow++)
        {
            u32 id = gtm->data(gtm->index(irow,1),Qt::DisplayRole).toInt();
            QColor color = colorMap.value(id);
            if (color.isValid())
                gtm->setData(gtm->index(irow,2), color, Qt::EditRole);
        }
    }
}
