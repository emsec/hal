#ifndef HAL_PLUGIN_DECORATOR_EXAMPLE_LIB_H
#define HAL_PLUGIN_DECORATOR_EXAMPLE_LIB_H

#include "core/interface_base.h"

class PLUGIN_API plugin_decorator_example_lib : virtual public i_base
{
public:
    std::string get_name() override;

    std::string get_version() override;

    void on_load() override;

    void on_unload() override;
};

#endif
