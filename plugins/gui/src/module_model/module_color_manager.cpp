#include "gui/module_model/module_color_manager.h"

#include "hal_core/netlist/project_manager.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>

namespace hal
{

    ModuleColorManager::ModuleColorManager(QObject *parent)
        : QObject(parent)
    {
        setModuleColor(1, QColor(96, 110, 112));
    }

    void ModuleColorManager::clear()
    {
        mModuleColors.clear();
    }

    QColor ModuleColorManager::moduleColor(u32 id) const
    {
        return mModuleColors.value(id);
    }

    QColor ModuleColorManager::setModuleColor(u32 id, const QColor& col)
    {
        QColor retval = mModuleColors.value(id);
        mModuleColors[id] = col;
        Q_EMIT moduleColorChanged(id);
        return retval;
    }

    QColor ModuleColorManager::getRandomColor()
    {
        static qreal h = 0.5;

        h += 0.6180339887498948;

        if (h > 1)
            --h;

        QColor c;
        c.setHsvF(h, 0.8, 0.95);    // (MAYBE) GET S AND V FROM STYLESHEET OR CYCLE 3 DIMENSIONAL
        return c;
    }

    QColor ModuleColorManager::setRandomColor(u32 id)
    {
        QColor retval = mModuleColors.value(id);
        mModuleColors.insert(id,getRandomColor());
        Q_EMIT moduleColorChanged(id);
        return retval;
    }

    QMap<u32, QColor> ModuleColorManager::getColorMap() const
    {
        return mModuleColors;
    }

    void ModuleColorManager::removeColor(u32 id)
    {
        mModuleColors.remove(id);
    }

    //---------------------------------------
    ModuleColorSerializer::ModuleColorSerializer()
        : ProjectSerializer("modulecolor")
    {;}

    void ModuleColorSerializer::restore(ModuleColorManager *mcm)
    {
        ProjectManager* pm = ProjectManager::instance();
        std::string relname = pm->get_filename(m_name);
        if (!relname.empty())
            restoreModuleColor(pm->get_project_directory(), relname, mcm);
    }

    void ModuleColorSerializer::serializeColorMap(QJsonArray& mcArr, const ModuleColorManager *mcm)
    {
        QMap<u32, QColor> cmap = mcm->getColorMap();

        for (auto it = cmap.constBegin(); it != cmap.constEnd(); ++it)
        {
            QJsonObject mcEntry;
            mcEntry["id"] = (int) it.key();
            mcEntry["color"] = it.value().name(QColor::HexArgb);
            mcArr.append(mcEntry);
        }
    }

    std::string ModuleColorSerializer::serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave)
    {
        Q_UNUSED(netlist);
        Q_UNUSED(isAutosave);
        QString mcFilename("modulecolor.json");
        QFile mcFile(QDir(QString::fromStdString(savedir.string())).absoluteFilePath(mcFilename));
        if (!mcFile.open(QIODevice::WriteOnly)) return std::string();

        QJsonObject mcObj;
        QJsonArray  mcArr;

        const ModuleColorManager* mcm = gNetlistRelay->getModuleColorManager();
        if (!mcm) return std::string();

        serializeColorMap(mcArr,mcm);

        mcObj["modcolors"] = mcArr;

        mcFile.write(QJsonDocument(mcObj).toJson(QJsonDocument::Compact));

        return mcFilename.toStdString();
    }


    void ModuleColorSerializer::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        Q_UNUSED(netlist);
        std::string relname = ProjectManager::instance()->get_filename(m_name);
        if (!relname.empty())
            restoreModuleColor(loaddir, relname);
    }

    void ModuleColorSerializer::restoreModuleColor(const std::filesystem::path& loaddir, const std::string& jsonfile, ModuleColorManager *mcm)
    {
        if (!mcm)
        {
            mcm = gNetlistRelay->getModuleColorManager();
            if (!mcm) return;
        }

        QFile mcFile(QDir(QString::fromStdString(loaddir.string())).absoluteFilePath(QString::fromStdString(jsonfile)));
        if (!mcFile.open(QIODevice::ReadOnly))
            return;
        QJsonDocument jsonDoc   = QJsonDocument::fromJson(mcFile.readAll());
        const QJsonObject& json = jsonDoc.object();

        if (json.contains("modcolors") && json["modcolors"].isArray())
        {
            QJsonArray mcArr = json["modcolors"].toArray();
            int nmc          = mcArr.size();
            for (int imc = 0; imc < nmc; imc++)
            {
                QJsonObject mcEntry = mcArr.at(imc).toObject();
                u32 moduleId = mcEntry["id"].toInt();
                gNetlistRelay->getModuleColorManager()->setModuleColor(moduleId,QColor(mcEntry["color"].toString()));
            }
        }
    }
}
