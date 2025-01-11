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

#include "verilator/verilator.h"

#include <sstream>

namespace hal
{
    namespace verilator
    {
        const std::string get_testbench_cpp_template()
        {
            const char* testbench_cpp_template = "#include \"V<design_name>.h\"\n"
                                                 "#include \"saleae_file.h\"\n"
                                                 "#include \"saleae_parser.h\"\n"
                                                 "#include \"saleae_directory.h\"\n"
                                                 "#include \"verilated_vcd_c.h\"\n"
                                                 "#include <iostream>\n"
                                                 "#include <stdlib.h>\n"
                                                 "#include <verilated.h>\n"
                                                 "\n"
                                                 "#include <fstream>\n"
                                                 "\n"
                                                 "#define MAX_SIM_TIME 20\n"
                                                 "vluint64_t sim_time = 0;\n"
                                                 "vluint64_t to_simulate = 0;\n"
                                                 "\n"
                                                 "V<design_name> *dut = new V<design_name>;\n"
                                                 "VerilatedVcdC *m_trace = new VerilatedVcdC;\n"
                                                 "\n"
                                                 "double sc_time_stamp() { return sim_time; }\n"
                                                 "\n"
                                                 "void propagate_events() {\n"
                                                 "  dut->eval();\n"
                                                 "  //printf(\"dut->eval();\\n\");\n"
                                                 "  m_trace->dump(sim_time);\n"
                                                 "  //printf(\"m_trace->dump(%d);\\n\", sim_time);\n"
                                                 "  sim_time += to_simulate;\n"
                                                 "  //printf(\"sim_time += %d;\\n\\n\", to_simulate);\n"
                                                 "}\n"
                                                 "\n"
                                                 "void set_simulation_value(void *obj, uint64_t t, int val) {\n"
                                                 "  vluint8_t* vluintPtr = static_cast<vluint8_t*>(obj);\n"
                                                 "  if (t != sim_time) {\n"
                                                 "    to_simulate = t - sim_time;\n"
                                                 "    propagate_events();\n"
                                                 "  }\n"
                                                 "  if (val != 0 && val != 1)\n"
                                                 "  {\n"
                                                 "    printf(\"error: %d\\n\", val);\n"
                                                 "  }\n"
                                                 "  else\n"
                                                 "  {\n"
                                                 "    *vluintPtr = (vluint8_t)val;\n"
                                                 "    //printf(\"%x = 0x%d;\\n\", obj, val);\n"
                                                 "  }\n"
                                                 "}\n"
                                                 "\n"
                                                 "int main(int argc, char **argv, char **env) {\n"
                                                 "  hal::SaleaeParser sp(\"saleae/saleae.json\");\n"
                                                 "\n"
                                                 "  std::unordered_map<std::string,hal::Net*> netMap;\n"
                                                 "  for (const hal::SaleaeDirectory::ListEntry& sdle : sp.get_directory().get_net_list())\n"
                                                 "  {\n"
                                                 "    netMap.insert(std::make_pair(sdle.name,new hal::Net(sdle.name,sdle.id)));\n"
                                                 "  }\n"
                                                 "\n"
                                                 "  Verilated::traceEverOn(true);\n"
                                                 "\n"
                                                 "  dut->trace(m_trace, 1);\n"
                                                 "  m_trace->open(\"waveform.vcd\");\n"
                                                 "\n"
                                                 "//  <set_vcc>\n"
                                                 "//  <set_gnd>\n"
                                                 "\n"
                                                 "  <set_callbacks>\n"
                                                 "\n"
                                                 "\n"
                                                 "  int counter = 0;\n"
                                                 "  while (sp.next_event()) {\n"
                                                 "    counter++;\n"
                                                 "    if (counter % 1000000 == 0) {\n"
                                                 "      printf(\"%d\\n\", counter);\n"
                                                 "    }\n"
                                                 "  }\n"
                                                 "  \n"
                                                 "  dut->eval();\n"
                                                 "  //printf(\"dut->eval()\\n\");\n"
                                                 "  m_trace->dump(sim_time); // to_simulate already added in event loop\n"
                                                 "  //printf(\"m_trace->dump(%d);\\n\", sim_time);\n"
                                                 "  // m_trace->final();\n"
                                                 "\n"
                                                 "  m_trace->close();\n"
                                                 "  delete dut;\n"
                                                 "  exit(EXIT_SUCCESS);\n"
                                                 "}\n";
            return std::string(testbench_cpp_template);
        }
    }    // namespace verilator
}    // namespace hal
