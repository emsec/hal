#pragma once

namespace hal
{
    namespace netlist_preprocessing
    {
        /**
         * Identifies the originating toolchain of a netlist (vendor, synthesizer, backend).
         * Different flavors encode multi-bit signals and instance names in different ways, so
         * downstream heuristics need to know which conventions to expect.
         */
        enum class NetlistFlavor
        {
            Default,      /**< Unknown or generic netlist with no vendor-specific assumptions. */
            Yosys,        /**< Open-source Yosys synthesizer output. */
            Vivado,       /**< Xilinx Vivado synthesizer output. */
            SynopsysDC,   /**< Synopsys Design Compiler output. */
        };
    }    // namespace netlist_preprocessing
}    // namespace hal
