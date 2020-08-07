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

#include "def.h"
#include "netlist/hdl_writer/hdl_writer.h"

#include <functional>
#include <map>

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Net;
    class Gate;

    /**
     * @ingroup hdl_writers
     */
    class HDL_FILE_WRITER_API HDLWriterVerilog : public HDLWriter
    {
    public:
        HDLWriterVerilog() = default;
        ~HDLWriterVerilog() = default;

        std::string get_name() const override;

        /**
         * Serializes the netlist into hdl code.
         *
         * @param[in] netlist - The netlist.
         * @param[out] stream - The string stream which will be filled with the hdl code.
         * @returns True on success.
         */
        bool write(Netlist* netlist, std::stringstream& stream) override;

    private:
        Netlist* m_netlist;
        std::stringstream* m_stream;

        void print_module_interface_verilog();

        void print_signal_definition_verilog();

        void print_gate_definitions_verilog();

        void print_generic_map_verilog(Gate* n);

        bool print_gate_signal_list_verilog(Gate* n, std::vector<std::string> port_types, bool is_first, std::function<Net*(std::string)> get_net_fkt);

        void prepare_signal_names();

        std::string get_net_name(Net* n);

        std::string get_gate_name(Gate* g);

        std::string get_port_name(std::string pin);

        std::map<std::string, std::vector<std::string>> get_gate_signal_buses_verilog(std::vector<std::string> port_types);

        /**
         * Following maps saves prepared net names used internally.
         */
        std::map<Net*, std::string> m_printable_signal_names;

        std::map<std::string, Net*> m_printable_signal_names_str_to_net;

        std::map<Net*, std::string> m_only_wire_names;

        std::map<std::string, Net*> m_only_wire_names_str_to_net;

        std::map<Net*, std::string> m_in_names;

        std::map<std::string, Net*> m_in_names_str_to_net;

        std::map<Net*, std::string> m_out_names;

        std::map<std::string, Net*> m_out_names_str_to_net;

        std::map<Net*, std::string> m_gnd_names;

        std::map<std::string, Net*> m_gnd_names_str_to_net;

        std::map<Net*, std::string> m_vcc_names;

        std::map<std::string, Net*> m_vcc_names_str_to_net;
    };
}    // namespace hal
