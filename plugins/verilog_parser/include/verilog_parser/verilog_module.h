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

#include <optional>
#include <variant>

namespace hal
{
    using identifier_t        = std::string;
    using ranged_identifier_t = std::pair<std::string, std::vector<std::vector<u32>>>;
    using numeral_t           = std::vector<BooleanFunction::Value>;
    using empty_t             = std::monostate;
    using assignment_t        = std::variant<identifier_t, ranged_identifier_t, numeral_t, empty_t>;

    struct VerilogDataEntry
    {
        std::string m_name;
        std::string m_type  = "unknown";
        std::string m_value = "";
    };

    struct VerilogSignal
    {
        std::string m_name;
        std::vector<std::vector<u32>> m_ranges;
        std::vector<VerilogDataEntry> m_attributes;
        std::vector<std::string> m_expanded_names;
    };

    struct VerilogPort
    {
        std::string m_identifier;
        std::string m_expression;
        PinDirection m_direction;
        std::vector<std::vector<u32>> m_ranges;
        std::vector<VerilogDataEntry> m_attributes;
        std::vector<std::string> m_expanded_identifiers;
    };

    struct VerilogPortAssignment
    {
        std::optional<std::string> m_port_name;
        std::vector<assignment_t> m_assignment;
    };

    struct VerilogAssignment
    {
        std::vector<assignment_t> m_variable;
        std::vector<assignment_t> m_assignment;
    };

    struct VerilogInstance
    {
        std::string m_name;
        std::string m_type;
        bool m_is_module = false;
        std::vector<VerilogPortAssignment> m_port_assignments;
        std::vector<VerilogDataEntry> m_parameters;
        std::vector<VerilogDataEntry> m_attributes;
        std::vector<std::pair<std::string, std::string>> m_expanded_port_assignments;
    };

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
        std::vector<VerilogDataEntry> m_attributes;    // module attributes

        // ports
        std::vector<std::unique_ptr<VerilogPort>> m_ports;
        std::map<std::string, VerilogPort*> m_ports_by_identifier;
        std::map<std::string, VerilogPort*> m_ports_by_expression;

        // signals
        std::vector<std::unique_ptr<VerilogSignal>> m_signals;
        std::map<std::string, VerilogSignal*> m_signals_by_name;

        // assignments
        std::vector<VerilogAssignment> m_assignments;
        std::vector<std::pair<std::string, std::string>> m_expanded_assignments;

        // instances
        std::vector<std::unique_ptr<VerilogInstance>> m_instances;
        std::map<std::string, VerilogInstance*> m_instances_by_name;
    };
}    // namespace hal