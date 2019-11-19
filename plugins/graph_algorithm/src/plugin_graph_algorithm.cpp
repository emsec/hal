#include "plugin_graph_algorithm.h"
#include "core/log.h"

extern std::shared_ptr<i_base> get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_graph_algorithm>());
}

std::string plugin_graph_algorithm::get_name() const
{
    return std::string("graph_algorithm");
}

std::string plugin_graph_algorithm::get_version() const
{
    return std::string("0.1");
}
