#include "factory_decorator_example_lib.h"
#include "plugin_decorator_example_lib.h"

std::shared_ptr<i_base> factory_decorator_example_lib::get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_decorator_example_lib>());
}

extern i_factory* get_factory()
{
    static factory_decorator_example_lib* factory = new factory_decorator_example_lib();
    return (i_factory*)factory;
}

std::set<std::string> factory_decorator_example_lib::get_dependencies()
{
    return {};
}