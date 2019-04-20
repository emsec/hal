//  MIT License
//
//  Copyright (c) 2019 Marc Fyrbiak
//  Copyright (c) 2019 Sebastian Wallat
//  Copyright (c) 2019 Max Hoffmann
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

#ifndef __HAL_GATE_DECORATOR_LUT_H__
#define __HAL_GATE_DECORATOR_LUT_H__

#include "def.h"

#include "../gate_decorator.h"

#include <string>
#include <vector>

/**
 * @ingroup gate_decorator_system
 */
class gate_decorator_lut : public gate_decorator, public std::enable_shared_from_this<gate_decorator_lut>
{
public:
    /**
     * Decorator constructor.
     *
     * @param[in] gate - the gate to decorate.
     * @param[in] input_bit_width - the number of lut input signals.
     * @param[in] output_bit_width - the number of lut output signals.
     * @param[in] config - the lut configuration.
    */
    gate_decorator_lut(std::shared_ptr<gate> gate, u32 input_bit_width, u32 output_bit_width, const std::string& config);

    ~gate_decorator_lut() = default;

    /**
     * Get the type of the decorator.
     *
     * @returns The type.
     */
    gate_decorator_system::decorator_type get_type() override;

    /**
     * Get the number of input signals of the lut.
     *
     * @returns The number of input signals.
     */
    u32 get_input_bit_width();

    /**
     * Get the number of output signals of the lut.
     *
     * @returns The number of output signals.
     */
    u32 get_output_bit_width();

    /**
     * Get the lut configuration memory.
     *
     * @returns a vector of output bits.
     */
    std::vector<bool> get_lut_configuration();

    /**
     * Get the size of the lut configuration memory in bits.
     *
     * @returns the number of bits.
     */
    u32 get_lut_configuration_bit_size();

    /**
     * Get the size of the lut configuration memory in bytes.
     *
     * @returns the number of bytes.
     */
    u32 get_lut_configuration_byte_size();

    /**
     * Get a byte of the lut configuration memory.
     *
     * @param[in] index - the byte index
     * @returns a single byte.
     */
    u8 get_lut_configuration_byte_by_index(u32 index);

    /**
     * Get the lut configuration string.
     *
     * @returns the configuration.
     */
    std::string get_lut_configuration_string();

private:
    u32 m_input_bit_width;

    u32 m_output_bit_width;

    std::string m_lut_config;
};

#endif /* __HAL_GATE_DECORATOR_LUT_H__ */
