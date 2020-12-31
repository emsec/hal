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

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_type.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace hal
{
    /**
     * A gate library is a collection of gate types including their pins and Boolean functions.
     *
     * @ingroup gate_lib
     */
    class NETLIST_API GateLibrary
    {
    public:
        /**
         * Construct a gate library by specifying its name and the path to the file that describes the library.
         *
         * @param[in] path - The path to the gate library file.
         * @param[in] name - The name of the gate library.
         */
        GateLibrary(const std::filesystem::path& path, const std::string& name);
        ~GateLibrary() = default;

        /**
         * Get the name of the gate library.
         *
         * @returns The name of the gate library.
         */
        std::string get_name() const;

        /**
         * Get the path to the file describing the gate library.
         *
         * @returns The path to the gate library file.
         */
        std::filesystem::path get_path() const;        

        /**
         * Create a new gate type, add it to the gate library, and return it.
         * 
         * @param[in] name - The name of the gate type.
         * @param[in] base_type - The base type of the gate type.
         * @returns The new gate type instance on success, a nullptr otherwise.
         */
        GateType* create_gate_type(const std::string& name, GateType::BaseType base_type = GateType::BaseType::combinational);

        /**
         * Check whether the given gate type is contained in this library.
         *
         * @param[in] gate_type - The gate type.
         * @returns True if the gate type is part of this library, false otherwise.
         */
        bool contains_gate_type(GateType* gate_type) const;

        /**
         * Check by name whether the given gate type is contained in this library.
         *
         * @param[in] name - The name of the gate type.
         * @returns True if the gate type is part of this library, false otherwise.
         */
        bool contains_gate_type_by_name(const std::string& name) const;

        /**
         * Get the gate type corresponding to the given name if contained within the library. In case there is no gate type with that name, a nullptr is returned.
         *
         * @param[in] name - The name of the gate type.
         * @returns The gate type on success, a nullptr otherwise.
         */
        GateType* get_gate_type_by_name(const std::string& name) const;

        /**
         * Get all gate types of the library.
         *
         * @returns A map from gate type names to gate types.
         */
        std::unordered_map<std::string, GateType*> get_gate_types() const;

        /**
         * Mark a gate type as a VCC gate type.
         *
         * @param[in] gate_type - The gate type.
         * @returns True on success, false otherwise.
         */
        bool mark_vcc_gate_type(GateType* gate_type);

        /**
         * Get all VCC gate types of the library.
         *
         * @returns A map from VCC gate type names to gate type objects.
         */
        std::unordered_map<std::string, GateType*> get_vcc_gate_types() const;

        /**
         * Mark a gate type as a GND gate type.
         *
         * @param[in] gate_type - The gate type.
         * @returns True on success, false otherwise.
         */
        bool mark_gnd_gate_type(GateType* gate_type);

        /**
         * Get all GND gate types of the library.
         *
         * @returns A map from GND gate type names to gate type objects.
         */
        std::unordered_map<std::string, GateType*> get_gnd_gate_types() const;

        /**
         * Add an include required for parsing a corresponding netlist, e.g., VHDL libraries.
         *
         * @param[in] inc - The include to add.
         */
        void add_include(const std::string& inc);

        /**
         * Get a vector of includes required for parsing a corresponding netlist, e.g., VHDL libraries.
         *
         * @returns A vector of includes.
         */
        std::vector<std::string> get_includes() const;

    private:
        std::string m_name;
        std::filesystem::path m_path;

        u32 m_next_gate_type_id;

        std::vector<std::unique_ptr<GateType>> m_gate_types;
        std::unordered_map<std::string, GateType*> m_gate_type_map;
        std::unordered_map<std::string, GateType*> m_vcc_gate_types;
        std::unordered_map<std::string, GateType*> m_gnd_gate_types;

        std::vector<std::string> m_includes;

        GateLibrary(const GateLibrary&) = delete;
        GateLibrary& operator=(const GateLibrary&) = delete;

        u32 get_unique_gate_type_id();
    };
}    // namespace hal
