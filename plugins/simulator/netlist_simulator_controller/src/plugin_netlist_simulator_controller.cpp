#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "hal_core/utilities/utils.h"
#include <QDebug>
#include <QResource>
#include <QFile>
#include <QSettings>
#include <QDir>

namespace hal
{
    u32 NetlistSimulatorControllerPlugin::sMaxControllerId = 0;
    SimulationSettings* NetlistSimulatorControllerPlugin::sSimulationSettings = nullptr;

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

    std::unique_ptr<NetlistSimulatorController> NetlistSimulatorControllerPlugin::create_simulator_controller(const std::string &nam) const
    {
        return std::unique_ptr<NetlistSimulatorController>(new NetlistSimulatorController(++sMaxControllerId, nam));
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
    }

    void NetlistSimulatorControllerPlugin::on_load()
    {
        QResource::registerResource("simulator_resources.rcc");
        QDir userConfigDir(QString::fromStdString(utils::get_user_config_directory()));
        sSimulationSettings = new SimulationSettings(userConfigDir.absoluteFilePath("simulationsettings.ini"));
    }
}    // namespace hal
