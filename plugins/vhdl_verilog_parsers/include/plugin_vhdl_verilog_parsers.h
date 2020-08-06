#pragma once

#include "core/plugin_interface_base.h"

namespace hal
{
    class HDLParserVHDL;
    class HDLParserVerilog;

    class PLUGIN_API VhdlVerilogParsersPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void on_load() const override;
        void on_unload() const override;

    private:
        mutable std::unique_ptr<HDLParserVHDL> m_vhdl_parser;
        mutable std::unique_ptr<HDLParserVerilog> m_verilog_parser;
    };
}    // namespace hal
