#include "core/interface_factory.h"

#include "core/log.h"

std::set<interface_type> i_factory::get_plugin_types()
{
    return this->get_plugin_instance()->get_type();
}

std::set<std::string> i_factory::get_dependencies()
{
    return {};
}

std::shared_ptr<i_base> i_factory::query_interface(const interface_type type)
{
    auto valid_type = this->get_plugin_types();
    if (valid_type.find(type) == valid_type.end())
    {
        log_error("core", "cannot construct interface type '{}'.", (u32)type);
        return nullptr;
    }

    auto interface = this->get_plugin_instance();
    log_debug("core", "created interface '{}' for plugin '{}'.", (u32)type, interface->get_name());
    return interface;
}
