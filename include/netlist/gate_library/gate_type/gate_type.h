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

#ifndef __HAL_GATE_TYPE_H
#define __HAL_GATE_TYPE_H

#include "netlist/boolean_function.h"

#include <string>
#include <unordered_map>

/**
 * Gate type class containing information about the internals of a specific gate type.
 *
 * @ingroup netlist
 */
class NETLIST_API gate_type
{
public:
    enum class base_type
    {
        combinatorial,
        lut,
        ff,
        latch
    };

    enum class special_behavior
    {
        U,    // not set
        L,
        H,
        N,
        T,
        X
    };

    /**
     * Constructor for a gate type.
     * 
     * @param[in] name - The name of the gate type.
     */
    gate_type(const std::string& name);
    virtual ~gate_type() = default;

    /**
     * Returns a string describing the gate type.
     * 
     * @returns A string describing the gate type.
     */
    static std::string to_string(const gate_type& v);

    // TODO description
    friend std::ostream& operator<<(std::ostream& os, const gate_type& gt);

    /**
     * Tests whether two gate type objects are equal.
     * 
     * @param[in] other - Gate type object to compare to.
     * @returns True when both gate type objects are equal, false otherwise.
     */
    bool operator==(const gate_type& other) const;

    /**
     * Tests whether two gate type objects are unequal.
     * 
     * @param[in] other - Gate type object to compare to.
     * @returns True when both gate type objects are unequal, false otherwise.
     */
    bool operator!=(const gate_type& other) const;

    /**
     * Adds an input pin to the gate type.
     *
     * @param[in] input_pin - The name of an input pin.
     */
    void add_input_pin(std::string input_pin);

    /**
     * Add a vector of input pins to the gate type.
     *
     * @param[in] input_pins - A vector of names of input pins.
     */
    void add_input_pins(const std::vector<std::string>& input_pins);

    /**
     * Adds an output pin to the gate type.
     *
     * @param[in] output_pin - The name of an output pin.
     */
    void add_output_pin(std::string output_pin);

    /**
     * Add a vector of output pins to the gate type.
     *
     * @param[in] output_pins - A vector of names of output pins.
     */
    void add_output_pins(const std::vector<std::string>& output_pins);

    /**
     * Adds a boolean function describing the behaviour of the specified pin to the gate type.
     *
     * @param[in] pin_name - The name of the pin that the boolean function corresponds to.
     * @param[in] bf - A boolean function object.
     */
    void add_boolean_function(std::string pin_name, boolean_function bf);

    /**
     * Returns the name of the gate type.
     *
     * @returns The name of the gate type.
     */
    std::string get_name() const;

    /**
     * Returns the base type of the gate type.
     * The base type can be either combinatorial, lut, ff, or latch.
     *
     * @returns The base type of the gate type.
     */
    base_type get_base_type() const;

    /**
     * Returns a vector of input pins of the gate type.
     *
     * @returns A vector of input pins of the gate type.
     */
    std::vector<std::string> get_input_pins() const;

    /**
     * Returns a vector of output pins of the gate type.
     *
     * @returns A vector of output pins of the gate type..
     */
    std::vector<std::string> get_output_pins() const;

    /**
     * Returns a map containing the boolean functions of the gate type.
     * The map can be accessed using the names of the respective pins.
     *
     * @returns A map of pin names to boolean functions.
     */
    std::unordered_map<std::string, boolean_function> get_boolean_functions() const;

protected:
    base_type m_base_type;

private:
    std::string m_name;

    std::vector<std::string> m_input_pins;
    std::vector<std::string> m_output_pins;

    std::unordered_map<std::string, boolean_function> m_functions;

    gate_type(const gate_type&) = delete;               // disable copy-constructor
    gate_type& operator=(const gate_type&) = delete;    // disable copy-assignment

    bool doCompare(const gate_type& other) const;
};
#endif    //__HAL_GATE_TYPE_H
