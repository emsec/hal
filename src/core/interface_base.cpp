#include "core/interface_base.h"
#include "core/interface_cli.h"
#include "core/interface_gui.h"
#include "core/interface_interactive_ui.h"
#include "core/log.h"

void i_base::initialize()
{
}

void i_base::on_load() const
{
}

void i_base::on_unload() const
{
}

std::set<std::string> i_base::get_dependencies() const
{
    return {};
}

void i_base::initialize_logging() const
{
    log_manager& l = log_manager::get_instance();
    l.add_channel(get_name(), {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");
}

bool i_base::has_type(interface_type t) const
{
    return (t == interface_type::base && dynamic_cast<const i_base*>(this)) || (t == interface_type::cli && dynamic_cast<const i_cli*>(this))
           || (t == interface_type::interactive_ui && dynamic_cast<const i_interactive_ui*>(this)) || (t == interface_type::gui && dynamic_cast<const i_gui*>(this));
    return false;
}
