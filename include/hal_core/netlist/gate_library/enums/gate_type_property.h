//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/utilities/enums.h"

namespace hal
{
    /**
     * A set of available properties for a gate type.
     */
    enum class GateTypeProperty
    {
        combinational,  /**< Combinational gate type. **/
        sequential,     /**< Sequential gate type. **/
        tristate,       /**< Tristate gate type. **/
        power,          /**< Power gate type. **/
        ground,         /**< Ground gate type. **/
        ff,             /**< Flip-flop gate type. **/
        latch,          /**< Latch gate type. **/
        ram,            /**< RAM gate type. **/
        fifo,           /**< FIFO gate type. **/
        shift_register, /**< Shift register gate type. **/
        io,             /**< IO gate type. **/
        dsp,            /**< DSP gate type. **/
        pll,            /**< PLL gate type. **/
        oscillator,     /**< Oscillator gate type. **/
        scan,           /**< Scan gate type. **/
        c_buffer,       /**< Buffer gate type. **/
        c_inverter,     /**< Inverter gate type. **/
        c_and,          /**< AND gate type. **/
        c_nand,         /**< NAND gate type. **/
        c_or,           /**< OR gate type. **/
        c_nor,          /**< NOR gate type. **/
        c_xor,          /**< XOR gate type. **/
        c_xnor,         /**< XNOR gate type. **/
        c_aoi,          /**< AOI gate type. **/
        c_oai,          /**< OAI gate type. **/
        c_mux,          /**< MUX gate type. **/
        c_carry,        /**< Carry gate type. **/
        c_half_adder,   /**< Half adder gate type. **/
        c_full_adder,   /**< Full adder gate type. **/
        c_lut           /**< LUT gate type. **/
    };

    template<>
    std::map<GateTypeProperty, std::string> EnumStrings<GateTypeProperty>::data;
}    // namespace hal