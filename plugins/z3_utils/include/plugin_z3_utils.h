#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "SubgraphFunctionGenerator.h"
#include "z3Wrapper.h"
#include "converter/converter.h"
#include "converter/cpp_converter.h"

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
