#include "factory_quine_mccluskey.h"
#include "plugin_quine_mccluskey.h"

std::shared_ptr<i_base> factory_quine_mccluskey::get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_quine_mccluskey>());
}

extern i_factory* get_factory()
{
    static factory_quine_mccluskey* factory = new factory_quine_mccluskey();
    return (i_factory*)factory;
}

std::set<std::string> factory_quine_mccluskey::get_dependencies()
{
    return {};
}