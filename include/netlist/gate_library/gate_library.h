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

#include "def.h"
#include "netlist/gate_library/gate_type/gate_type.h"

#include <map>
#include <set>
#include <string>
#include <vector>

/**
 * @ingroup netlist
 */
class NETLIST_API gate_library
{
public:
    /**
     * Constructor.
     *
	 * @param[in] name - Name of the gate library.
	 */
    gate_library(const std::string& name);

    ~gate_library() = default;

    /**
     * Returns the library name.
     *
     * @returns The library's name.
     */
    std::string get_name() const;

    /**
     * Add a gate type to the gate library.
     *
     * @param[in] gt - The gate type object.
     */
    void add_gate_type(std::shared_ptr<const gate_type> gt);

    /**
     * Get all gate types of the library.
     *
     * @returns A map from gate type names to gate type objects.
     */
    const std::map<std::string, std::shared_ptr<const gate_type>>& get_gate_types();

    /**
     * Get all global VCC gate types of the library.
     *
     * @returns A map from global VCC gate type names to gate type objects.
     */
    const std::map<std::string, std::shared_ptr<const gate_type>>& get_vcc_gate_types();

    /**
     * Get all global GND gate types of the library.
     *
     * @returns A map from global GND gate type names to gate type objects.
     */
    const std::map<std::string, std::shared_ptr<const gate_type>>& get_gnd_gate_types();

    /**
     * Get necessary includes of the library, e.g., VHDL libraries.
     *
     * @returns The includes of the library
     */
    std::vector<std::string> get_includes() const;

    /**
     * Add a necessary includes of the library, e.g., VHDL libraries.
     *
     * @param[in] inc - the include to add
     */
    void add_include(const std::string& inc);

private:
    std::string m_name;

    std::map<std::string, std::shared_ptr<const gate_type>> m_gate_type_map;
    std::map<std::string, std::shared_ptr<const gate_type>> m_vcc_gate_types;
    std::map<std::string, std::shared_ptr<const gate_type>> m_gnd_gate_types;

    std::vector<std::string> m_includes;
};
