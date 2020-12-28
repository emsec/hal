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
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_sequential.h"
#include "rapidjson/document.h"

#include <filesystem>
#include <map>
#include <optional>
#include <unordered_map>

namespace hal
{
    class GateTypeLut;

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
        std::unique_ptr<GateLibrary> parse(const std::filesystem::path& file_path) override;

    private:
        struct PinCtx
        {
            std::vector<std::string> input_pins;
            std::vector<std::string> output_pins;
            std::unordered_map<std::string, std::string> boolean_functions;
            std::vector<std::string> power_pins;
            std::vector<std::string> ground_pins;
            std::vector<std::string> lut_pins;
            std::vector<std::string> state_pins;
            std::vector<std::string> neg_state_pins;
            std::vector<std::string> clock_pins;
            std::vector<std::string> enable_pins;
            std::vector<std::string> reset_pins;
            std::vector<std::string> set_pins;
            std::vector<std::string> data_pins;
            const std::unordered_map<std::string, std::vector<std::string>*> type_to_data = {{"power", &power_pins},
                                                                                             {"ground", &ground_pins},
                                                                                             {"lut", &lut_pins},
                                                                                             {"state", &state_pins},
                                                                                             {"neg_state", &neg_state_pins},
                                                                                             {"clock", &clock_pins},
                                                                                             {"enable", &enable_pins},
                                                                                             {"reset", &reset_pins},
                                                                                             {"set", &set_pins},
                                                                                             {"data", &data_pins}};
        };

        struct GroupCtx
        {
            std::unordered_map<std::string, std::map<u32, std::string>> input_groups;
            std::unordered_map<std::string, std::map<u32, std::string>> output_groups;
        };

        std::unique_ptr<GateLibrary> m_gate_lib;
        std::stringstream* m_fs;
        std::filesystem::path m_path;

        std::set<std::string> m_cell_names;

        static std::unordered_map<std::string, GateTypeSequential::ClearPresetBehavior> m_string_to_behavior;

        bool deserialize_gate_library(const rapidjson::Document& document);
        bool deserialize_gate_type(const rapidjson::Value& gate_type);
        bool deserialize_pin(PinCtx& pin_ctx, const rapidjson::Value& pin, const std::string& gt_name);
        bool deserialize_group(GroupCtx& group_ctx, const rapidjson::Value& group, const std::string& gt_name);
        bool deserialize_lut_config(GateTypeLut* gt_lut, const rapidjson::Value& lut_config);
        bool deserialize_ff_config(GateTypeSequential* gt_ff, const rapidjson::Value& ff_config);
        bool deserialize_latch_config(GateTypeSequential* gt_latch, const rapidjson::Value& latch_config);
    };
}    // namespace hal
