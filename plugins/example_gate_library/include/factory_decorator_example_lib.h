#ifndef __HAL_FACTORY_DECORATOR_EXAMPLE_LIB_H__
#define __HAL_FACTORY_DECORATOR_EXAMPLE_LIB_H__

#include "core/interface_factory.h"

class factory_decorator_example_lib : public i_factory
{
public:
    std::shared_ptr<i_base> get_plugin_instance() override;

    std::set<std::string> get_dependencies() override;
};

EXPORT i_factory* get_factory();

#endif
