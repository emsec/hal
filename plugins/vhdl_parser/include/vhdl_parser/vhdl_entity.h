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
    namespace
    {
        using ci_string           = core_strings::CaseInsensitiveString;
        using identifier_t        = ci_string;
        using ranged_identifier_t = std::pair<ci_string, std::vector<std::vector<u32>>>;
        using numeral_t           = std::vector<BooleanFunction::Value>;
        using empty_t             = std::monostate;
        using assignment_t        = std::variant<identifier_t, ranged_identifier_t, numeral_t, empty_t>;

        struct VhdlDataEntry
        {
            u32 m_line_number;
            std::string m_name;
            std::string m_type  = "unknown";
            std::string m_value = "";
        };

        struct VhdlSignal
        {
            ci_string m_name;
            std::vector<std::vector<u32>> m_ranges;
            std::vector<VhdlDataEntry> m_attributes;
            std::vector<ci_string> m_expanded_names;
        };

        struct VhdlPort
        {
            ci_string m_identifier;
            PinDirection m_direction;
            std::vector<std::vector<u32>> m_ranges;
            std::vector<VhdlDataEntry> m_attributes;
            std::vector<ci_string> m_expanded_identifiers;
        };

        struct VhdlPortAssignment
        {
            std::optional<assignment_t> m_port;
            std::vector<assignment_t> m_assignment;
        };

        struct VhdlAssignment
        {
            std::vector<assignment_t> m_variable;
            std::vector<assignment_t> m_assignment;
        };

        struct VhdlInstance
        {
            ci_string m_name;
            ci_string m_type;
            bool m_is_entity = false;
            std::vector<VhdlPortAssignment> m_port_assignments;
            std::vector<VhdlDataEntry> m_generics;
            std::vector<VhdlDataEntry> m_attributes;
            std::vector<std::pair<ci_string, ci_string>> m_expanded_port_assignments;
        };
    }    // namespace
    struct VhdlEntity
    {
    public:
        using ci_string = core_strings::CaseInsensitiveString;

        VhdlEntity()  = default;
        ~VhdlEntity() = default;

        /**
         * Check whether an module is considered smaller than another module.
         *
         * @param[in] other - The module to compare against.
         * @returns True if the module is smaller than 'other', false otherwise.
         */
        bool operator<(const VhdlEntity& other) const
        {
            return m_name < other.m_name;
        }

        // module information
        ci_string m_name;
        u32 m_line_number;
        std::vector<VhdlDataEntry> m_attributes;    // module attributes

        // ports
        std::vector<std::unique_ptr<VhdlPort>> m_ports;
        std::map<ci_string, VhdlPort*> m_ports_by_identifier;
        std::set<ci_string> m_expanded_port_identifiers;

        // signals
        std::vector<std::unique_ptr<VhdlSignal>> m_signals;
        std::map<ci_string, VhdlSignal*> m_signals_by_name;

        // assignments
        std::vector<VhdlAssignment> m_assignments;
        std::vector<std::pair<ci_string, ci_string>> m_expanded_assignments;

        // instances
        std::vector<std::unique_ptr<VhdlInstance>> m_instances;
        std::map<ci_string, VhdlInstance*> m_instances_by_name;
    };
}    // namespace hal