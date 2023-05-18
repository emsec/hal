#include "xilinx_toolbox/types.h"

#include "hal_core/utilities/enums.h"

namespace hal
{
    template<>
    std::map<xilinx_toolbox::BELType, std::string> EnumStrings<xilinx_toolbox::BELType>::data =  {
        {xilinx_toolbox::BELType::A6LUT, "A6LUT"},
        {xilinx_toolbox::BELType::B6LUT, "B6LUT"},
        {xilinx_toolbox::BELType::C6LUT, "C6LUT"},
        {xilinx_toolbox::BELType::D6LUT, "D6LUT"},
        {xilinx_toolbox::BELType::A5LUT, "A5LUT"},
        {xilinx_toolbox::BELType::B5LUT, "B5LUT"},
        {xilinx_toolbox::BELType::C5LUT, "C5LUT"},
        {xilinx_toolbox::BELType::D5LUT, "D5LUT"},
        {xilinx_toolbox::BELType::F7AMUX, "F7AMUX"},
        {xilinx_toolbox::BELType::F7BMUX, "F7BMUX"},
        {xilinx_toolbox::BELType::F8MUX, "F8MUX"},
        {xilinx_toolbox::BELType::CARRY4, "CARRY4"},
        {xilinx_toolbox::BELType::AFF, "AFF"},
        {xilinx_toolbox::BELType::BFF, "BFF"},
        {xilinx_toolbox::BELType::CFF, "CFF"},
        {xilinx_toolbox::BELType::DFF, "DFF"},
        {xilinx_toolbox::BELType::A5FF, "A5FF"},
        {xilinx_toolbox::BELType::B5FF, "B5FF"},
        {xilinx_toolbox::BELType::C5FF, "C5FF"},
        {xilinx_toolbox::BELType::D5FF, "D5FF"},
        {xilinx_toolbox::BELType::BUFG, "BUFG"},
        {xilinx_toolbox::BELType::INBUF_EN, "INBUF_EN"},
        {xilinx_toolbox::BELType::OUTBUF, "OUTBUF"},
    };

    template<>
    std::map<xilinx_toolbox::LOCType, std::string> EnumStrings<xilinx_toolbox::LOCType>::data = {
        {xilinx_toolbox::LOCType::SLICE, "SLICE"},
        {xilinx_toolbox::LOCType::RAMB36, "RAMB36"},

        {xilinx_toolbox::LOCType::BUFGCTRL, "BUFGCTRL"},

        {xilinx_toolbox::LOCType::PIN, "PIN"},
    };

}    // namespace hal