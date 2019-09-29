#include "plugin_test_plugin.h"
#include "core/log.h"

std::string plugin_test_plugin::get_name()
{
    return std::string("test_plugin");
}

std::string plugin_test_plugin::get_version()
{
    return std::string("1.2.3");
}
