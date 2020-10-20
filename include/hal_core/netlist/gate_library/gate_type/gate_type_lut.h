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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    /**
     * A LUT gate type contains information about its internals such as input and output pins as well as its Boolean functions.<br>
     * In addition to the standard gate type functionality, it provides the foundation to automatically read the initialization string of each LUT gate of the netlist and convert it to a Boolean function.
     *
     * @ingroup gate_lib
     */
    class GateTypeLut : public GateType
    {
    public:
        /**
         * Construct a new LUT gate type by specifying its name.
         *
         * @param[in] name - The name of the LUT gate type.
         */
        GateTypeLut(const std::string& name);
        ~GateTypeLut() override = default;

        /**
         * Add an existing output pin to the collection of output pins that generate their output not from a Boolean function but an initialization string.<br>
         * The pin has to be declared as an output pin beforehand.
         *
         * @param[in] pin_name - The name of the output pin to add.
         */
        void add_output_from_init_string_pin(const std::string& pin_name);

        /**
         * Get the set of output pins that generate their output not from a Boolean function but an initialization string.
         *
         * @returns The set of output pin names.
         */
        std::unordered_set<std::string> get_output_from_init_string_pins() const;

        /**
         * Set the data category in which to find the initialization string.
         *
         * @param[in] category - The data category.
         */
        void set_config_data_category(const std::string& category);

        /**
         * Get the data category in which to find the initialization string.
         *
         * @returns The data category.
         */
        std::string get_config_data_category() const;

        /**
         * Set the data identifier used to specify the initialization string.
         *
         * @param[in] identifier - The data identifier.
         */
        void set_config_data_identifier(const std::string& identifier);

        /**
         * Get the data identifier used to specify the initialization string.
         *
         * @returns The data identifier.
         */
        std::string get_config_data_identifier() const;

        /**
         * Set the bit-order of the initialization string.
         *
         * @param[in] ascending - True if ascending bit-order, false otherwise.
         */
        void set_config_data_ascending_order(bool ascending);

        /**
         * Get the bit-order of the initialization string.
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
}    // namespace hal
