
#ifndef HAL_FACTORY_TEST_PLUGIN_H
#define HAL_FACTORY_TEST_PLUGIN_H

#ifdef REMOVE_ME

#include "core/interface_factory.h"

class PLUGIN_API factory_test_plugin : public i_factory
{
public:
    // interface implementation: i_factory
    //std::set<std::string> get_dependencies() override;

    // interface implementation: i_factory
    std::shared_ptr<BasePluginInterface> get_plugin_instance() override;

};

extern "C" PLUGIN_API i_factory* get_factory();

#endif // REMOVE_ME

#endif //HAL_FACTORY_TEST_PLUGIN_H
