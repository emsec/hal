#include "python-binding/factory_python_shell.h"
#include "python-binding/plugin_python_shell.h"

std::shared_ptr<i_base> factory_python_shell::get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_python_shell>());
}

extern i_factory* get_factory()
{
    static factory_python_shell* factory = new factory_python_shell();
    return (i_factory*)factory;
}
