#include "core/interface_base.h"

#include "core/log.h"

std::set<interface_type> i_base::get_type()
{
    return {interface_type::base};
}

void i_base::initialize_logging()
{
    log_manager& l = log_manager::get_instance();
    l.add_channel(this->get_name(), {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");
}

void i_base::on_load()
{
}

void i_base::on_unload()
{
}