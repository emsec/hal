#pragma once

#include "core/plugin_interface_base.h"
#include "liberty_parser.h"

namespace hal
{
    class PLUGIN_API LibertyParserPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void on_load() override;
        void on_unload() override;

    private:
        LibertyParser m_parser;
    };
}    // namespace hal
