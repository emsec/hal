#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/utils.h"
#include "hal_core/netlist/project_manager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QResource>
#include <QFile>
#include <QSettings>
#include <QDir>

namespace hal
{
    u32 NetlistSimulatorControllerPlugin::sMaxControllerId = 0;
    SimulationSettings* NetlistSimulatorControllerPlugin::sSimulationSettings = nullptr;
    SimulatorSerializer* NetlistSimulatorControllerPlugin::sSimulatorSerializer = nullptr;

    SimulatorSerializer::SimulatorSerializer()
        : ProjectSerializer("simulator"), mNetlist(nullptr)
    {;}

    std::string SimulatorSerializer::serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave)
    {
        Q_UNUSED(netlist);
        Q_UNUSED(isAutosave);
        QString simFilename("simulator.json");
        QFile simFile(QDir(QString::fromStdString(savedir.string())).absoluteFilePath(simFilename));
        if (!simFile.open(QIODevice::WriteOnly)) return std::string();

        QJsonObject simObj;
        QJsonArray  simArr;

        for (NetlistSimulatorController* ctrl : NetlistSimulatorControllerMap::instance()->toList())
        {
            QJsonObject simEntry;
            simEntry["id"] = (int) ctrl->get_id();
            simEntry["name"] = ctrl->name();
            simEntry["workdir"] = QString::fromStdString(ctrl->get_working_directory());
            simArr.append(simEntry);
        }
        simObj["simulator"] = simArr;

        simFile.write(QJsonDocument(simObj).toJson(QJsonDocument::Compact));

        return simFilename.toStdString();

    }

    void SimulatorSerializer::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        mNetlist = netlist;
        if (!loaddir.empty())
            mProjDir = QDir(QString::fromStdString(loaddir.string()));
    }

    std::vector<std::unique_ptr<NetlistSimulatorController>> SimulatorSerializer::restore()
    {
        std::vector<std::unique_ptr<NetlistSimulatorController>> retval;
        ProjectManager* pm = ProjectManager::instance();
        std::string relname = pm->get_filename(m_name);
        if (relname.empty()) return retval;

        NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
        if (!ctrlPlug) return retval;
        if (mProjDir.isEmpty())
            mProjDir = QDir(QString::fromStdString(pm->get_project_directory()));


        QFile simFile(mProjDir.absoluteFilePath(QString::fromStdString(relname)));
        if (!simFile.open(QIODevice::ReadOnly))
            return retval;
        QJsonDocument simDoc   = QJsonDocument::fromJson(simFile.readAll());
        const QJsonObject& simObj = simDoc.object();

        if (simObj.contains("simulator") && simObj["simulator"].isArray())
        {
            QJsonArray simArr = simObj["simulator"].toArray();
            int n          = simArr.size();
            for (int i = 0; i < n; i++)
            {
                QJsonObject simEntry = simArr.at(i).toObject();
                QString workdir = simEntry["workdir"].toString();
                if (workdir.isEmpty()) continue;
                if (QFileInfo(workdir).isRelative())
                    workdir = mProjDir.absoluteFilePath(workdir);
                if (!QFileInfo(workdir).isDir())
                {
                    log_warning("simualtion_plugin", "Cannot restore simulation from workdir '{}'.", workdir.toStdString());
                    continue;
                }
                QString contrFile = QDir(workdir).absoluteFilePath("netlist_simulator_controller.json");
                retval.push_back(ctrlPlug->restore_simulator_controller(mNetlist,contrFile.toStdString()));
            }
        }

        return retval;
    }

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistSimulatorControllerPlugin>();
    }

    std::string NetlistSimulatorControllerPlugin::get_name() const
    {
        return std::string("netlist_simulator_controller");
    }

    std::string NetlistSimulatorControllerPlugin::get_version() const
    {
        return std::string("0.7");
    }

    std::unique_ptr<NetlistSimulatorController> NetlistSimulatorControllerPlugin::create_simulator_controller(const std::string &nam, const std::string &workdir) const
    {
        return std::unique_ptr<NetlistSimulatorController>(new NetlistSimulatorController(++sMaxControllerId, nam, workdir));
    }

    std::unique_ptr<NetlistSimulatorController> NetlistSimulatorControllerPlugin::restore_simulator_controller(Netlist* nl, const std::string &filename) const
    {
        NetlistSimulatorController* nsc = new NetlistSimulatorController(++sMaxControllerId,nl,filename);
        if (nsc->get_working_directory().empty())
        {
            delete nsc;
            return nullptr;
        }
        return std::unique_ptr<NetlistSimulatorController>(nsc);
    }

    std::shared_ptr<NetlistSimulatorController> NetlistSimulatorControllerPlugin::simulator_controller_by_id(u32 id) const
    {
        NetlistSimulatorController* ctrl = NetlistSimulatorControllerMap::instance()->controller(id);
        if (!ctrl) return nullptr;
        return std::shared_ptr<NetlistSimulatorController>(ctrl,[](void*){;});
    }

    void NetlistSimulatorControllerPlugin::on_unload()
    {
        NetlistSimulatorControllerMap::instance()->shutdown();
        QResource::unregisterResource("simulator_resources.rcc");
        if (sSimulationSettings) sSimulationSettings->sync();
        if (sSimulatorSerializer) delete sSimulatorSerializer;
    }

    void NetlistSimulatorControllerPlugin::on_load()
    {
        // report simulation warnings and error messages not related to specific controller to common channel
        LogManager::get_instance().add_channel("simulation_plugin", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        QResource::registerResource("simulator_resources.rcc");
        QDir userConfigDir(QString::fromStdString(utils::get_user_config_directory()));
        sSimulationSettings = new SimulationSettings(userConfigDir.absoluteFilePath("simulationsettings.ini"));
        sSimulatorSerializer = new SimulatorSerializer;
    }
}    // namespace hal
