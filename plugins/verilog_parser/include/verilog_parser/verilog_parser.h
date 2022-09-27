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
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"
#include "hal_core/utilities/special_strings.h"
#include "hal_core/utilities/token_stream.h"
#include "verilog_parser/verilog_module.h"

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
    class NETLIST_API VerilogParser : public NetlistParser
    {
    public:
        VerilogParser()  = default;
        ~VerilogParser() = default;

        /**
         * Parse a Verilog netlist into an internal intermediate format.
         *
         * @param[in] file_path - Path to the Verilog netlist file.
         * @returns True on success, false otherwise.
         */
        Result<std::monostate> parse(const std::filesystem::path& file_path) override;

        /**
         * Instantiate the parsed Verilog netlist using the specified gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        Result<std::unique_ptr<Netlist>> instantiate(const GateLibrary* gate_library) override;

    private:
        std::stringstream m_fs;
        std::filesystem::path m_path;

        // temporary netlist
        Netlist* m_netlist = nullptr;

        // all modules of the netlist
        std::vector<std::unique_ptr<VerilogModule>> m_modules;
        std::unordered_map<std::string, VerilogModule*> m_modules_by_name;
        std::string m_last_module;

        // token stream of entire input file
        TokenStream<std::string> m_token_stream;

        // some caching
        std::unordered_map<std::string, GateType*> m_gate_types;
        std::unordered_map<std::string, GateType*> m_vcc_gate_types;
        std::unordered_map<std::string, GateType*> m_gnd_gate_types;
        std::unordered_map<Net*, std::vector<std::tuple<PinDirection, std::string, Module*>>> m_module_ports;

        // unique aliases
        std::unordered_map<std::string, u32> m_signal_name_occurrences;
        std::unordered_map<std::string, u32> m_instance_name_occurrences;

        // nets
        Net* m_zero_net;
        Net* m_one_net;
        std::unordered_map<std::string, Net*> m_net_by_name;
        std::unordered_map<std::string, std::vector<std::string>> m_nets_to_merge;

        // parse HDL into intermediate format
        void tokenize();
        Result<std::monostate> parse_tokens();
        Result<std::monostate> parse_module(std::vector<VerilogDataEntry>& attributes);
        void parse_port_list(VerilogModule* module);
        Result<std::monostate> parse_port_declaration_list(VerilogModule* module);
        Result<std::monostate> parse_port_definition(VerilogModule* module, std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_signal_definition(VerilogModule* module, std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_assignment(VerilogModule* module);
        void parse_attribute(std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_instance(VerilogModule* module, std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_port_assign(VerilogInstance* instance);
        Result<std::vector<VerilogDataEntry>> parse_generic_assign();

        // construct netlist from intermediate format
        Result<std::monostate> construct_netlist(VerilogModule* top_module);
        Result<Module*>
            instantiate_module(const std::string& instance_name, VerilogModule* verilog_module, Module* parent, const std::unordered_map<std::string, std::string>& parent_module_assignments);

        // helper functions
        std::string get_unique_alias(std::unordered_map<std::string, u32>& name_occurrences, const std::string& name) const;
    };
}    // namespace hal
