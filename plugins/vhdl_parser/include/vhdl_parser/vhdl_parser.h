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
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"
#include "hal_core/utilities/result.h"
#include "hal_core/utilities/special_strings.h"
#include "hal_core/utilities/token_stream.h"
#include "vhdl_parser/vhdl_entity.h"

#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace hal
{
    /**
     * @ingroup netlist_parser
     */
    class NETLIST_API VHDLParser : public NetlistParser
    {
    public:
        VHDLParser()  = default;
        ~VHDLParser() = default;

        /**
         * Parse a VHDL netlist into an internal intermediate format.
         *
         * @param[in] file_path - Path to the VHDL netlist file.
         * @returns True on success, false otherwise.
         */
        Result<std::monostate> parse(const std::filesystem::path& file_path) override;

        /**
         * Instantiate the parsed VHDL netlist using the specified gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        Result<std::unique_ptr<Netlist>> instantiate(const GateLibrary* gate_library) override;

    private:
        enum class AttributeTarget
        {
            ENTITY,
            INSTANCE,
            SIGNAL
        };

        using ci_string          = core_strings::CaseInsensitiveString;
        using attribute_buffer_t = std::map<AttributeTarget, std::map<ci_string, VhdlDataEntry>>;

        std::stringstream m_fs;
        std::filesystem::path m_path;

        // temporary netlist
        Netlist* m_netlist = nullptr;

        // all entities of the netlist
        std::vector<std::unique_ptr<VhdlEntity>> m_entities;
        std::unordered_map<ci_string, VhdlEntity*> m_entities_by_name;
        ci_string m_last_entity;

        // std::unordered_map<ci_string, VhdlEntity> m_entities;
        // ci_string m_last_entity;

        // token stream of entire input file
        TokenStream<ci_string> m_token_stream;

        // some caching
        std::unordered_map<ci_string, GateType*> m_gate_types;
        std::unordered_map<ci_string, GateType*> m_vcc_gate_types;
        std::unordered_map<ci_string, GateType*> m_gnd_gate_types;
        std::unordered_map<Net*, std::vector<std::pair<Module*, u32>>> m_module_port_by_net;
        std::unordered_map<Module*, std::vector<std::pair<std::string, Net*>>> m_module_ports;
        attribute_buffer_t m_attribute_buffer;
        std::unordered_map<ci_string, ci_string> m_attribute_types;

        // library prefixes
        std::set<ci_string> m_libraries;

        // unique aliases
        std::unordered_map<ci_string, u32> m_signal_name_occurrences;
        std::unordered_map<ci_string, u32> m_instance_name_occurrences;

        // nets
        Net* m_zero_net;
        Net* m_one_net;
        std::unordered_map<ci_string, Net*> m_net_by_name;
        std::unordered_map<ci_string, std::vector<ci_string>> m_nets_to_merge;

        // parse HDL into intermediate format
        void tokenize();
        Result<std::monostate> parse_tokens();
        void parse_library();
        Result<std::monostate> parse_entity();
        Result<std::monostate> parse_port_definitons(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_attribute();
        Result<std::monostate> parse_architecture();
        Result<std::monostate> parse_architecture_header(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_signal_definition(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_architecture_body(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_assignment(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_instance(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_port_assign(VhdlInstance* instance);
        Result<std::monostate> parse_generic_assign(VhdlInstance* instance);
        Result<std::monostate> assign_attributes(VhdlEntity* vhdl_entity);

        // construct netlist from intermediate format
        Result<std::monostate> construct_netlist(VhdlEntity* top_entity);
        Result<Module*> instantiate_entity(const ci_string& instance_name, VhdlEntity* vhdl_entity, Module* parent, const std::unordered_map<ci_string, ci_string>& parent_module_assignments);

        // helper functions
        ci_string get_unique_alias(std::unordered_map<ci_string, u32>& name_occurrences, const ci_string& name) const;
    };
}    // namespace hal
