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

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_type.h"

namespace hal
{
    struct VerilogModule
    {
    public:
        VerilogModule()  = default;
        ~VerilogModule() = default;

        /**
         * Check whether an module is considered smaller than another module.
         *
         * @param[in] other - The module to compare against.
         * @returns True if the module is smaller than 'other', false otherwise.
         */
        bool operator<(const VerilogModule& other) const
        {
            return m_name < other.m_name;
        }

        // module information
        std::string m_name;
        u32 m_line_number;
        std::vector<std::tuple<std::string, std::string, std::string>> m_attributes;    // module attributes

        // ports
        std::vector<std::string> m_port_identifiers;                                                                          // external port names (in order)
        std::unordered_set<std::string> m_port_expressions;                                                                   // internal port names
        std::unordered_map<std::string, std::string> m_port_ident_to_expr;                                                    // external port names to internal port names
        std::unordered_map<std::string, PinDirection> m_port_directions;                                                      // internal port names to direction
        std::unordered_map<std::string, std::vector<std::vector<u32>>> m_port_ranges;                                         // internal port names to ranges
        std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> m_port_attributes;    // internal port attributes
        std::unordered_map<std::string, std::vector<std::string>> m_expanded_port_identifiers;
        std::unordered_map<std::string, std::string> m_expanded_port_ident_to_expr;

        // signals
        std::vector<std::string> m_signals;                                                                                     // signal names
        std::unordered_map<std::string, std::vector<std::vector<u32>>> m_signal_ranges;                                         // signal ranges
        std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> m_signal_attributes;    // signal attributes
        std::unordered_map<std::string, std::vector<std::string>> m_expanded_signals;

        // assignments
        std::vector<std::pair<TokenStream<std::string>, TokenStream<std::string>>> m_assignments;    // signal assignments
        std::vector<std::pair<std::string, std::string>> m_expanded_assignments;

        // instances
        std::vector<std::string> m_instances;                                                                                              // instance names
        std::unordered_map<std::string, std::string> m_instance_types;                                                                     // instance types
        std::unordered_map<std::string, std::vector<std::pair<Token<std::string>, TokenStream<std::string>>>> m_instance_assignments;      // port assignments
        std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> m_instance_generic_assignments;    // generic assignments
        std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> m_instance_attributes;             // instance attributes
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> m_expanded_gate_assignments;
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> m_expanded_module_assignments;
    };
}    // namespace hal