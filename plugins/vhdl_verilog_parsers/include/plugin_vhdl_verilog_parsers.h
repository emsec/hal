#pragma once

#include "core/plugin_interface_base.h"
#include "hdl_parser_verilog.h"
#include "hdl_parser_vhdl.h"

namespace hal
{
    class PLUGIN_API VhdlVerilogParsersPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void on_load() override;
        void on_unload() override;

    private:
        HDLParserVHDL m_vhdl_parser;
        HDLParserVerilog m_verilog_parser;
    };
}    // namespace hal
