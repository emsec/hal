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
#include "hal_core/utilities/token_stream.h"

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
    class NETLIST_API EdifParser : public NetlistParser
    {
    public:
        EdifParser()  = default;
        ~EdifParser() = default;

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
        struct EdifProperty
        {
            std::string m_name;
            std::string m_type;
            std::string m_value;
        };

        struct EdifInstance
        {
            std::string m_name;
            std::string m_type;
            std::string m_library;
            std::vector<EdifProperty> m_properties;
        };

        struct EdifEndpoint
        {
            std::string m_port_name;
            u32 m_index                 = 1;
            std::string m_instance_name = "";
        };

        struct EdifNet
        {
            std::string m_name;
            std::vector<EdifEndpoint> m_endpoints;
        };

        struct EdifPort
        {
            std::string m_name;
            PinDirection m_direction;
            u32 m_width = 1;
        };

        struct EdifCell
        {
            std::string m_name;
            std::vector<EdifPort> m_ports;
            std::vector<EdifInstance> m_instances;
            std::vector<EdifNet> m_nets;
        };

        struct EdifLibrary
        {
            std::string m_name;
            std::vector<std::unique_ptr<EdifCell>> m_cells;
            std::unordered_map<std::string, EdifCell*> m_cells_by_name;
        };

        std::stringstream m_fs;
        std::filesystem::path m_path;

        // temporary netlist
        Netlist* m_netlist = nullptr;

        std::vector<std::unique_ptr<EdifLibrary>> m_libraries;
        std::unordered_map<std::string, EdifLibrary*> m_libraries_by_name;

        // token stream of entire input file
        TokenStream<std::string> m_token_stream;

        // parse HDL into intermediate format
        void tokenize();
        Result<std::monostate> parse_tokens();
        Result<std::monostate> parse_library();
        Result<std::monostate> parse_cell(EdifLibrary* library);
        Result<std::monostate> parse_view(EdifCell* cell);
        Result<std::monostate> parse_interface(EdifCell* cell);
        Result<std::monostate> parse_contents(EdifCell* cell);
        Result<std::monostate> parse_instance(EdifCell* cell);
        Result<std::monostate> parse_net(EdifCell* cell);
        Result<std::monostate> parse_endpoints(EdifNet& net);

        Result<std::string> parse_rename(TokenStream<std::string>& stream, bool enforce_match = true);
        Result<std::pair<std::string, u32>> parse_array(TokenStream<std::string>& stream);
    };
}    // namespace hal
