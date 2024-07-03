#include "logic_evaluator/plugin_logic_evaluator.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "logic_evaluator/logic_evaluator.h"
#include "gui/content_manager/content_manager.h"
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<LogicEvaluatorPlugin>();
    }

    std::string LogicEvaluatorPlugin::get_name() const
    {
        return std::string("waveform_viewer");
    }

    std::string LogicEvaluatorPlugin::get_version() const
    {
        return std::string("0.7");
    }

    std::string LogicEvaluatorPlugin::get_description() const
    {
        return std::string("GUI to control simulation and view resulting waveforms");
    }

    std::set<std::string> LogicEvaluatorPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        retval.insert("hal_gui");
        return retval;
    }

    void LogicEvaluatorPlugin::on_load()
    {
        qRegisterMetaType<std::string>();
    }

    void LogicEvaluatorPlugin::on_unload()
    {
    }
}    // namespace hal
