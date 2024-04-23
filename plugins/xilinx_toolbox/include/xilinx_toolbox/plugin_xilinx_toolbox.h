#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class Netlist;

    class PLUGIN_API XilinxToolboxPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        // preprocessing

        /**
         * Removes all LUTs with multiple outputs and replaces them with equivalent smaller LUTs.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of removed gates on success, an error otherwise.
         */
        static Result<u32> split_luts(Netlist* nl);

        /**
         * Removes all shift register primitives and replaces them with equivalent chains of singular flip flops.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of removed gates on success, an error otherwise.
         */
        static Result<u32> split_shift_registers(Netlist* nl);

        /**
         * Parses an .xdc file and extracts the position LOC and BEL data.
         * Afterwards translates the found LOC and BEL data into integer coordinates.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of removed gates on success, an error otherwise.
         */
        static Result<std::monostate> parse_xdc_file(Netlist* nl, const std::filesystem::path& xdc_file);
    };
}    // namespace hal
