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
#include "netlist/gate_library/gate_type/gate_type.h"

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
    /**
     * Constructor for a lut gate type.
     * 
     * @param[in] name - The name of the lut gate type.
     */
    gate_type_lut(const std::string& name);
    ~gate_type_lut() override = default;

    /**
     * Describes in what part of the gate definition to find the INIT string, e.g., "generic".
     * 
     * @param[in] category - The category as a string.
     */
    void set_data_category(const std::string& category);

    /**
     * Describes the identifier used to specify the INIT string.
     * 
     * @param[in] identifier - The identifier as a string.
     */
    void set_data_identifier(const std::string& identifier);

    /**
     * Describes the bit-order of the INIT string.
     * 
     * @param[in] ascending - True if ascending bit-order, false otherwise.
     */
    void set_data_ascending_order(bool ascending);

    /**
     * Returns the string describing in what part of the gate definition to find the INIT string, e.g., "generic".
     * 
     * @returns The string describing the category.
     */
    std::string get_data_category() const;

    /**
     * Returns the string describing the identifier used to specify the INIT string.
     * 
     * @returns The identifier as a string.
     */
    std::string get_data_identifier() const;

    /**
     * Returns the bit-order of the INIT string.
     * 
     * @returns True if ascending bit-order, false otherwise.
     */
    bool is_ascending_order() const;

private:
    std::string m_data_category;
    std::string m_data_identifier;
    bool m_ascending;

    bool doCompare(const gate_type& other) const;
};
#endif    //__HAL_GATE_TYPE_LUT_H
