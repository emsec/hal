#ifndef __HAL_FACTORY_GRAPH_ALGORITHM_H__
#define __HAL_FACTORY_GRAPH_ALGORITHM_H__

#include "core/interface_factory.h"

class PLUGIN_API factory_graph_algorithm : public i_factory
{
public:
    /** interface implementation: i_factory */
    std::set<std::string> get_dependencies() override;

    /** interface implementation: i_factory */
    std::shared_ptr<i_base> get_plugin_instance() override;
};

extern "C" PLUGIN_API i_factory* get_factory();

#endif /* __HAL_FACTORY_GRAPH_ALGORITHM_H__ */
