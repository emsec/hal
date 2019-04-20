#include "gui/factory_gui.h"

#include "gui/plugin_gui.h"

std::shared_ptr<i_base> factory_gui::get_plugin_instance()
{
    return std::dynamic_pointer_cast<i_base>(std::make_shared<plugin_gui>());
}

extern i_factory* get_factory()
{
    static factory_gui* factory = new factory_gui();
    return (i_factory*)factory;
}
