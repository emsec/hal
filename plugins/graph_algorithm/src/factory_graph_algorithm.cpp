#include "factory_graph_algorithm.h"
#include "plugin_graph_algorithm.h"

std::set<std::string> factory_graph_algorithm::get_dependencies()
{
    return {};
}

std::shared_ptr<i_base> factory_graph_algorithm::get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_graph_algorithm>());
}

extern i_factory* get_factory()
{
    static factory_graph_algorithm* factory = new factory_graph_algorithm();
    return (i_factory*)factory;
}
