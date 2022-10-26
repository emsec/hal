// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_type.h"

#include <unordered_map>
#include <unordered_set>

namespace hal
{
    struct VHDLEntity
    {
    public:
        using ci_string = core_strings::CaseInsensitiveString;

        VHDLEntity()  = default;
        ~VHDLEntity() = default;

        /**
         * Check whether an module is considered smaller than another module.
         *
         * @param[in] other - The module to compare against.
         * @returns True if the module is smaller than 'other', false otherwise.
         */
        bool operator<(const VHDLEntity& other) const
        {
            return m_name < other.m_name;
        }

        // module information
        ci_string m_name;
        u32 m_line_number;
        std::vector<std::tuple<std::string, std::string, std::string>> m_attributes;    // module attributes

        // ports
        std::vector<ci_string> m_port_names;                                                                                // port names (in order)
        std::unordered_set<ci_string> m_port_names_set;                                                                     // port names set
        std::unordered_map<ci_string, PinDirection> m_port_directions;                                                      // port names to direction
        std::unordered_map<ci_string, std::vector<std::vector<u32>>> m_port_ranges;                                         // port names to ranges
        std::unordered_map<ci_string, std::vector<std::tuple<std::string, std::string, std::string>>> m_port_attributes;    // port attributes
        std::unordered_map<ci_string, std::vector<ci_string>> m_expanded_port_names;

        // signals
        std::vector<ci_string> m_signals;                                                                                     // signal names
        std::unordered_set<ci_string> m_signals_set;                                                                          // signal names set
        std::unordered_map<ci_string, std::vector<std::vector<u32>>> m_signal_ranges;                                         // signal ranges
        std::unordered_map<ci_string, std::vector<std::tuple<std::string, std::string, std::string>>> m_signal_attributes;    // signal attributes
        std::unordered_map<ci_string, std::vector<ci_string>> m_expanded_signals;

        // assignments
        std::vector<std::pair<TokenStream<ci_string>, TokenStream<ci_string>>> m_assignments;    // signal assignments
        std::vector<std::pair<ci_string, ci_string>> m_expanded_assignments;

        // instances
        std::vector<ci_string> m_instances;                                                                                              // instance names
        std::unordered_set<ci_string> m_instances_set;                                                                                   // instance names set
        std::unordered_map<ci_string, ci_string> m_instance_types;                                                                       // instance types
        std::unordered_map<ci_string, std::vector<std::pair<TokenStream<ci_string>, TokenStream<ci_string>>>> m_instance_assignments;    // port assignments
        std::unordered_map<ci_string, std::vector<std::tuple<std::string, std::string, std::string>>> m_instance_generic_assignments;    // generic assignments
        std::unordered_map<ci_string, std::vector<std::tuple<std::string, std::string, std::string>>> m_instance_attributes;             // instance attributes
        std::unordered_map<ci_string, std::vector<std::pair<ci_string, ci_string>>> m_expanded_gate_assignments;
        std::unordered_map<ci_string, std::vector<std::pair<ci_string, ci_string>>> m_expanded_entity_assignments;
    };
}    // namespace hal