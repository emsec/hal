#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/enums.h"

#include <optional>

namespace hal
{
    /**
     * Applies Xilinx-specific placement information from an XDC file to a netlist.
     */
    namespace xilinx_toolbox
    {
        /**
         * @enum BELType
         * @brief BEL types relevant for XDC file parsing.
         */
        enum BELType
        {
            A6LUT, /**< The 6-input LUT at position A of a slice. */
            B6LUT, /**< The 6-input LUT at position B of a slice. */
            C6LUT, /**< The 6-input LUT at position C of a slice. */
            D6LUT, /**< The 6-input LUT at position D of a slice. */
            A5LUT, /**< The 5-input LUT at position A of a slice. */
            B5LUT, /**< The 5-input LUT at position B of a slice. */
            C5LUT, /**< The 5-input LUT at position C of a slice. */
            D5LUT, /**< The 5-input LUT at position D of a slice. */

            F7AMUX, /**< The F7 multiplexer that combines the LUTs at positions A and B. */
            F7BMUX, /**< The F7 multiplexer that combines the LUTs at positions C and D. */
            F8MUX,  /**< The F8 multiplexer that combines the outputs of both F7 multiplexers. */

            CARRY4, /**< The 4-bit carry chain of a slice. */

            AFF,  /**< The main flip-flop at position A of a slice. */
            BFF,  /**< The main flip-flop at position B of a slice. */
            CFF,  /**< The main flip-flop at position C of a slice. */
            DFF,  /**< The main flip-flop at position D of a slice. */
            A5FF, /**< The secondary flip-flop at position A of a slice. */
            B5FF, /**< The secondary flip-flop at position B of a slice. */
            C5FF, /**< The secondary flip-flop at position C of a slice. */
            D5FF, /**< The secondary flip-flop at position D of a slice. */

            BUFG,     /**< A global clock buffer. */
            INBUF_EN, /**< The input buffer of an I/O block. */
            OUTBUF,   /**< The output buffer of an I/O block. */
        };

        /**
         * @enum LOCType
         * @brief LOC types relevant for XDC file parsing.
         */
        enum LOCType
        {
            SLICE,    /**< A logic slice of the FPGA fabric. */
            RAMB36,   /**< A 36 Kb block RAM of the FPGA fabric. */
            BUFGCTRL, /**< A global clock buffer site of the FPGA fabric. */

            PIN, /**< A package pin of the device. */
        };

        /**
         * @struct LOC
         * @brief Information on a LOC.
         * 
         * This struct contains all relevant information on a LOC that is needed to parse and apply a XDC file to an existing netlist.
         */
        struct LOC
        {
            /** The type of the site that the cell is placed on. Defaults to `PIN`, which is what a LOC denoting a package pin keeps. */
            LOCType loc_type = PIN;

            /** The LOC exactly as it appears in the XDC file, e.g., `SLICE_X12Y34` for a site or `A5` for a pin. */
            std::string loc_name;

            /** The x-coordinate of the site on the FPGA fabric. Defaults to `0` and is only meaningful if `loc_type` is not `PIN`. */
            u64 loc_x = 0;

            /** The y-coordinate of the site on the FPGA fabric. Defaults to `0` and is only meaningful if `loc_type` is not `PIN`. */
            u64 loc_y = 0;
        };

        /**
         * @struct CellData
         * @brief Data of a cell on the FPGA fabric.
         * 
         * This struct contains all location information on a cell on the FPGA fabric, including its LOC and BEL data.
         */
        struct CellData
        {
            /** The site that the cell is placed on, empty if the XDC file does not constrain it. */
            std::optional<LOC> loc;

            /** The BEL within that site that the cell is placed on, empty if the XDC file does not constrain it. */
            std::optional<BELType> bel_type;
        };
    }    // namespace xilinx_toolbox

    template<>
    std::map<xilinx_toolbox::BELType, std::string> EnumStrings<xilinx_toolbox::BELType>::data;

    template<>
    std::map<xilinx_toolbox::LOCType, std::string> EnumStrings<xilinx_toolbox::LOCType>::data;
}    // namespace hal