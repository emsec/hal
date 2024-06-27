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
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "rapidjson/document.h"

#include <filesystem>
#include <map>
#include <optional>
#include <unordered_map>

namespace hal
{
    /**
     * @ingroup netlist
     */
    class NETLIST_API HGLParser : public GateLibraryParser
    {
    public:
        HGLParser()  = default;
        ~HGLParser() = default;

        /**
         * Deserializes a gate library in HGL format from the string stream into a gate library object.
         *
         * @param[in] file_path - Path to the file containing the gate library definition.
         * @returns The gate library or a nullptr on failure.
         */
        Result<std::unique_ptr<GateLibrary>> parse(const std::filesystem::path& file_path) override;

    private:
        const u32 HGL_FORMAT_VERSION = 4;
        u32 file_version             = 1;

        struct PinCtx
        {
            std::vector<std::string> pins;
            std::unordered_map<std::string, PinType> pin_to_type;
            std::unordered_map<std::string, PinDirection> pin_to_direction;
            std::unordered_map<std::string, std::string> boolean_functions;
        };

        struct GroupCtx
        {
            std::vector<std::string> pins;
            bool ascending;
            u32 start_index;
        };

        std::unique_ptr<GateLibrary> m_gate_lib;
        std::filesystem::path m_path;

        std::set<std::string> m_cell_names;

        Result<std::monostate> parse_gate_library(const rapidjson::Document& document);
        Result<std::monostate> parse_gate_type(const rapidjson::Value& gate_type);
        Result<std::monostate> parse_pin(PinCtx& pin_ctx, const rapidjson::Value& pin);
        Result<std::unique_ptr<GateTypeComponent>> parse_lut_config(const rapidjson::Value& lut_config);
        Result<std::unique_ptr<GateTypeComponent>> parse_ff_config(const rapidjson::Value& ff_config);
        Result<std::unique_ptr<GateTypeComponent>> parse_latch_config(const rapidjson::Value& latch_config);
        Result<std::unique_ptr<GateTypeComponent>> parse_ram_config(const rapidjson::Value& ram_config);
    };
}    // namespace hal
