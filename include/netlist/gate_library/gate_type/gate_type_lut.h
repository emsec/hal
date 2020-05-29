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

#pragma once

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_type/gate_type.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

/**
 * LUT gate type class containing information about the internals of a specific LUT gate type.
 *
 * @ingroup netlist
 */
class gate_type_lut : public gate_type
{
public:
    /**
     * Constructor for a LUT gate type.
     *
     * @param[in] name - The name of the LUT gate type.
     */
    gate_type_lut(const std::string& name);
    ~gate_type_lut() override = default;

    /**
     * Adds an output pin to the collection of output pins that generate their output not from a boolean function but an initialization string.
     * The pin has to be declared as an output pin beforehand.
     *
     * @param[in] output_pin - The name of the output string.
     */
    void add_output_from_init_string_pin(const std::string& pin_name);

    /**
     * Get the set of output pins that generate their output not from a boolean function but an initialization string.
     *
     * @returns Set of oputput pin names.
     */
    std::unordered_set<std::string> get_output_from_init_string_pins() const;

    /**
     * Set the category in which to find the INIT string.
     *
     * @param[in] category - The category as a string.
     */
    void set_config_data_category(const std::string& category);

    /**
     * Get the category in which to find the INIT string.
     *
     * @returns The string describing the category.
     */
    std::string get_config_data_category() const;

    /**
     * Set the identifier used to specify the INIT string.
     *
     * @param[in] identifier - The identifier as a string.
     */
    void set_config_data_identifier(const std::string& identifier);

    /**
     * Get the identifier used to specify the INIT string.
     *
     * @returns The identifier as a string.
     */
    std::string get_config_data_identifier() const;

    /**
     * Set the bit-order of the INIT string.
     *
     * @param[in] ascending - True if ascending bit-order, false otherwise.
     */
    void set_config_data_ascending_order(bool ascending);

    /**
     * Get the bit-order of the INIT string.
     *
     * @returns True if ascending bit-order, false otherwise.
     */
    bool is_config_data_ascending_order() const;

private:
    std::unordered_set<std::string> m_output_from_init_string_pins;
    std::string m_config_data_category;
    std::string m_config_data_identifier;
    bool m_ascending;
};
