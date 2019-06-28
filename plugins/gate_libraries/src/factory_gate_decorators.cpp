#include "factory_gate_decorators.h"
#include "plugin_gate_decorators.h"

std::shared_ptr<i_base> factory_gate_decorators::get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_gate_decorators>());
}

extern i_factory* get_factory()
{
    static factory_gate_decorators* factory = new factory_gate_decorators();
    return (i_factory*)factory;
}

std::set<std::string> factory_gate_decorators::get_dependencies()
{
    return {};
}