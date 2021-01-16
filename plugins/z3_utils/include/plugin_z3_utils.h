#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "SubgraphFunctionGenerator.h"

namespace hal
{
    class PLUGIN_API Z3UtilsPlugin : public BasePluginInterface
    {
    public:

        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;
    };
}
