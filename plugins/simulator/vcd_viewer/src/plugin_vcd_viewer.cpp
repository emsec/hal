#include "vcd_viewer/plugin_vcd_viewer.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "vcd_viewer/vcd_viewer.h"
#include "gui/content_manager/content_manager.h"
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VcdViewerPlugin>();
    }

    std::string VcdViewerPlugin::get_name() const
    {
        return std::string("vcd_viewer");
    }

    std::string VcdViewerPlugin::get_version() const
    {
        return std::string("0.7");
    }

    std::set<std::string> VcdViewerPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        retval.insert("netlist_simulator");
        retval.insert("hal_gui");
        return retval;
    }

    void VcdViewerPlugin::on_load()
    {
        ExternalContent::instance()->append(new VcdViewerFactory);
    }

    void VcdViewerPlugin::on_unload()
    {
        // TODO: ExternalContent unregister
    }
}    // namespace hal
