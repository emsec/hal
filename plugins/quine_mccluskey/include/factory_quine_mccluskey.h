#ifndef __HAL_FACTORY_QUINE_MCCLUSKEY_H__
#define __HAL_FACTORY_QUINE_MCCLUSKEY_H__

#include "core/interface_factory.h"

class factory_quine_mccluskey : public i_factory
{
public:
    /** interface implementation: i_factory */
    std::shared_ptr<i_base> get_plugin_instance() override;
    /** interface implementation: i_factory */
    std::set<std::string> get_dependencies() override;
};

extern "C" PLUGIN_API i_factory* get_factory();

#endif /* __HAL_FACTORY_QUINE_MCCLUSKEY_H__ */
