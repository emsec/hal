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
#include "hal_core/netlist/netlist_writer/netlist_writer.h"

#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace hal
{
    class GateType;
    class DataContainer;
    class Net;
    class Gate;
    class Module;

    /**
     * @ingroup netlist_writer
     */
    class NETLIST_API VerilogWriter : public NetlistWriter
    {
    public:
        VerilogWriter()  = default;
        ~VerilogWriter() = default;

        /**
         * Write the netlist to a Verilog file at the provided location.
         *
         * @param[in] netlist - The netlist.
         * @param[in] file_path - The output path.
         * @returns True on success, false otherwise.
         */
        bool write(Netlist* netlist, const std::filesystem::path& file_path) override;

    private:
        static const std::set<std::string> valid_types;

        bool write_module_declaration(std::stringstream& res_stream,
                                      const Module* module,
                                      std::unordered_map<const Module*, std::string>& module_type_aliases,
                                      std::unordered_map<std::string, u32>& module_type_occurrences) const;
        bool write_gate_instance(std::stringstream& res_stream,
                                 const Gate* gate,
                                 std::unordered_map<const DataContainer*, std::string>& aliases,
                                 std::unordered_map<std::string, u32>& identifier_occurrences) const;
        bool write_module_instance(std::stringstream& res_stream,
                                   const Module* module,
                                   std::unordered_map<const DataContainer*, std::string>& aliases,
                                   std::unordered_map<std::string, u32>& identifier_occurrences,
                                   std::unordered_map<const Module*, std::string>& module_type_aliases) const;
        bool write_parameter_assignments(std::stringstream& res_stream, const DataContainer* container) const;
        bool write_pin_assignments(std::stringstream& res_stream,
                                   const std::vector<std::pair<std::string, std::vector<const Net*>>>& pin_assignments,
                                   std::unordered_map<const DataContainer*, std::string>& aliases) const;
        bool write_parameter_value(std::stringstream& res_stream, const std::string& type, const std::string& value) const;
        std::string get_unique_alias(std::unordered_map<std::string, u32>& name_occurrences, const std::string& name) const;
        std::string escape(const std::string& s) const;
    };
}    // namespace hal
