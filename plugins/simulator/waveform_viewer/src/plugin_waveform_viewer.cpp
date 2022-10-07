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

    std::set<std::string> WaveformViewerPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        retval.insert("netlist_simulator");
        retval.insert("hal_gui");
        return retval;
    }

    void WaveformViewerPlugin::on_load()
    {
        qRegisterMetaType<u32>();
        qRegisterMetaType<std::string>();
        qRegisterMetaType<hal::NetlistSimulatorController::SimulationState>();
        ExternalContent::instance()->append(new VcdViewerFactory);
    }

    void WaveformViewerPlugin::on_unload()
    {
        // TODO: ExternalContent unregister
    }
}    // namespace hal
