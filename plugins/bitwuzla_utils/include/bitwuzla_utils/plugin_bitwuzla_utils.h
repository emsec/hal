
#pragma once

#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

namespace hal
{
    class PLUGIN_API BitwuzlaUtilsPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        BooleanFunction get_subgraph_function_py(const Net* n, const std::vector<Gate*>& sub_graph_gates) const;
    };
}    // namespace hal
