#pragma once

#include "core/plugin_interface_base.h"

namespace hal
{
    class PLUGIN_API VhdlVerilogWritersPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void on_load() override;
        void on_unload() override;
    };
}
