#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/enums.h"

#include <optional>

namespace hal
{
    namespace xilinx_toolbox
    {
        enum BELType
        {
            A6LUT,
            B6LUT,
            C6LUT,
            D6LUT,
            A5LUT,
            B5LUT,
            C5LUT,
            D5LUT,

            F7AMUX,
            F7BMUX,
            F8MUX,

            CARRY4,

            AFF,
            BFF,
            CFF,
            DFF,
            A5FF,
            B5FF,
            C5FF,
            D5FF,

            BUFG,
            INBUF_EN,
            OUTBUF,
        };

        enum LOCType
        {
            SLICE,
            RAMB36,
            BUFGCTRL,

            PIN,
        };

        struct LOC
        {
            LOCType loc_type;
            std::string loc_name;
            u64 loc_x;
            u64 loc_y;
        };

        struct CellData
        {
            std::optional<LOC> loc;
            std::optional<BELType> bel_type;
        };
    }    // namespace xilinx_toolbox

    template<>
    std::map<xilinx_toolbox::BELType, std::string> EnumStrings<xilinx_toolbox::BELType>::data;

    template<>
    std::map<xilinx_toolbox::LOCType, std::string> EnumStrings<xilinx_toolbox::LOCType>::data;
}    // namespace hal