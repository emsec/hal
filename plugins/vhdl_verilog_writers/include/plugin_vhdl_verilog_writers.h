#pragma once

#include "core/plugin_interface_base.h"
#include "hdl_writer_verilog.h"
#include "hdl_writer_vhdl.h"

namespace hal
{
    class PLUGIN_API VhdlVerilogWritersPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void on_load() override;
        void on_unload() override;

    private:
        HDLWriterVHDL m_vhdl_writer;
        HDLWriterVerilog m_verilog_writer;
    };
}
