//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include "pragma_once.h"

#ifndef __HAL_GATE_TYPE_LUT_H
#define __HAL_GATE_TYPE_LUT_H

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_type.h"

#include <map>
#include <string>

/**
 *  gate type class
 *
 * @ingroup netlist
 */
class gate_type_lut : public gate_type
{
public:
    enum direction_t
    {
        ascending,
        descending
    };

    using gate_type::gate_type;

    static std::string to_string(const gate_type_lut& v);
    friend std::ostream& operator<<(std::ostream& os, const gate_type_lut& gt);

    void set_data_category(std::string data_category);
    void set_data_key(std::string data_key);
    void set_direction(direction_t direction);

    std::string get_data_category() const;
    std::string get_data_key() const;
    direction_t get_direction() const;

private:
    std::string m_data_category;
    std::string m_data_key;
    direction_t m_direction;

    virtual bool doCompare(const gate_type& other) const;
};
#endif    //__HAL_GATE_TYPE_LUT_H
