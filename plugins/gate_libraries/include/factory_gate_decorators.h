#ifndef __HAL_FACTORY_GATE_DECORATORS_H__
#define __HAL_FACTORY_GATE_DECORATORS_H__

#include "core/interface_factory.h"

class PLUGIN_API factory_gate_decorators : public i_factory
{
public:
    std::shared_ptr<i_base> get_plugin_instance() override;

    std::set<std::string> get_dependencies() override;
};

extern "C" PLUGIN_API i_factory* get_factory();

#endif