#include "waveform_viewer/plugin_waveform_viewer.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "waveform_viewer/waveform_viewer.h"
#include "gui/content_manager/content_manager.h"
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<WaveformViewerPlugin>();
    }

    std::string WaveformViewerPlugin::get_name() const
    {
        return std::string("waveform_viewer");
    }

    std::string WaveformViewerPlugin::get_version() const
    {
        return std::string("0.7");
    }

    std::string WaveformViewerPlugin::get_description() const
    {
        return std::string("GUI to control simulation and view resulting waveforms");
    }

    std::set<std::string> WaveformViewerPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        retval.insert("hal_gui");
        return retval;
    }

    void WaveformViewerPlugin::on_load()
    {
        qRegisterMetaType<std::string>();
        qRegisterMetaType<hal::NetlistSimulatorController::SimulationState>();
        WaveformViewerFactory* wvFactory = new WaveformViewerFactory(QString::fromStdString(get_name()));
        ExternalContent::instance()->append(wvFactory);
        if (gNetlist) gContentManager->addExternalWidget(wvFactory);
    }

    void WaveformViewerPlugin::on_unload()
    {
        QString pluginName = QString::fromStdString(get_name());
        ExternalContent::instance()->removePlugin(pluginName);
    }
}    // namespace hal
