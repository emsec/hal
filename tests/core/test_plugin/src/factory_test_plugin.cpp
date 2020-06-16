#include "factory_test_plugin.h"
#include "plugin_test_plugin.h"

#ifdef REMOVE_ME

std::shared_ptr<BasePluginInterface> factory_test_plugin::get_plugin_instance()
{
    return std::dynamic_pointer_cast<BasePluginInterface>(std::make_shared<plugin_test_plugin>());
}

extern i_factory* get_factory()
{
    static factory_test_plugin* factory = new factory_test_plugin();
    return (i_factory*)factory;
}

#endif // REMOVE_ME
