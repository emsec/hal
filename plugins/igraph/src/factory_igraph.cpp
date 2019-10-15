#include "factory_igraph.h"
#include "plugin_igraph.h"

std::shared_ptr<i_base> factory_igraph::get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_igraph>());
}

extern i_factory* get_factory()
{
    static factory_igraph* factory = new factory_igraph();
    return (i_factory*)factory;
}
