#include "Vbram_netlist.h"
#include "saleae_file.h"
#include "saleae_parser.h"
#include "saleae_directory.h"
#include "verilated_vcd_c.h"
#include <iostream>
#include <stdlib.h>
#include <verilated.h>

#include <fstream>

#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;
vluint64_t to_simulate = 0;

Vbram_netlist *dut = new Vbram_netlist;
VerilatedVcdC *m_trace = new VerilatedVcdC;

double sc_time_stamp() { return sim_time; }

void propagate_events() {
  dut->eval();
  //printf("dut->eval();\n");
  m_trace->dump(sim_time);
  //printf("m_trace->dump(%d);\n", sim_time);
  sim_time += to_simulate;
  //printf("sim_time += %d;\n\n", to_simulate);
}

void set_simulation_value(void *obj, uint64_t t, int val) {
  vluint8_t* vluintPtr = static_cast<vluint8_t*>(obj);
  if (t != sim_time) {
    to_simulate = t - sim_time;
    propagate_events();
  }
  if (val != 0 && val != 1)
  {
    printf("error: %d\n", val);
  }
  else
  {
    *vluintPtr = (vluint8_t)val;
    //printf("%x = 0x%d;\n", obj, val);
  }
}

int main(int argc, char **argv, char **env) {
  hal::SaleaeParser sp("saleae/saleae.json");

  std::unordered_map<std::string,hal::Net*> netMap;
  for (const hal::SaleaeDirectory::ListEntry& sdle : sp.get_directory().get_net_list())
  {
    netMap.insert(std::make_pair(sdle.name,new hal::Net(sdle.name,sdle.id)));
  }

  Verilated::traceEverOn(true);

  dut->trace(m_trace, 1);
  m_trace->open("waveform.vcd");

//  <set_vcc>
//  <set_gnd>

  
  auto mask_0_net = netMap.find(std::string("mask_0"));
  if (mask_0_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_0" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_0_net->second, set_simulation_value, &dut->mask_0)) {
         std::cerr << "cannot initialize callback for net mask_0" << std::endl;
     }
  }



  auto mask_1_net = netMap.find(std::string("mask_1"));
  if (mask_1_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_1" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_1_net->second, set_simulation_value, &dut->mask_1)) {
         std::cerr << "cannot initialize callback for net mask_1" << std::endl;
     }
  }



  auto mask_2_net = netMap.find(std::string("mask_2"));
  if (mask_2_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_2" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_2_net->second, set_simulation_value, &dut->mask_2)) {
         std::cerr << "cannot initialize callback for net mask_2" << std::endl;
     }
  }



  auto mask_3_net = netMap.find(std::string("mask_3"));
  if (mask_3_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_3" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_3_net->second, set_simulation_value, &dut->mask_3)) {
         std::cerr << "cannot initialize callback for net mask_3" << std::endl;
     }
  }



  auto mask_4_net = netMap.find(std::string("mask_4"));
  if (mask_4_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_4" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_4_net->second, set_simulation_value, &dut->mask_4)) {
         std::cerr << "cannot initialize callback for net mask_4" << std::endl;
     }
  }



  auto mask_5_net = netMap.find(std::string("mask_5"));
  if (mask_5_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_5" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_5_net->second, set_simulation_value, &dut->mask_5)) {
         std::cerr << "cannot initialize callback for net mask_5" << std::endl;
     }
  }



  auto mask_6_net = netMap.find(std::string("mask_6"));
  if (mask_6_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_6" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_6_net->second, set_simulation_value, &dut->mask_6)) {
         std::cerr << "cannot initialize callback for net mask_6" << std::endl;
     }
  }



  auto mask_7_net = netMap.find(std::string("mask_7"));
  if (mask_7_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_7" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_7_net->second, set_simulation_value, &dut->mask_7)) {
         std::cerr << "cannot initialize callback for net mask_7" << std::endl;
     }
  }



  auto mask_8_net = netMap.find(std::string("mask_8"));
  if (mask_8_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_8" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_8_net->second, set_simulation_value, &dut->mask_8)) {
         std::cerr << "cannot initialize callback for net mask_8" << std::endl;
     }
  }



  auto mask_9_net = netMap.find(std::string("mask_9"));
  if (mask_9_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_9" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_9_net->second, set_simulation_value, &dut->mask_9)) {
         std::cerr << "cannot initialize callback for net mask_9" << std::endl;
     }
  }



  auto mask_10_net = netMap.find(std::string("mask_10"));
  if (mask_10_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_10" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_10_net->second, set_simulation_value, &dut->mask_10)) {
         std::cerr << "cannot initialize callback for net mask_10" << std::endl;
     }
  }



  auto mask_11_net = netMap.find(std::string("mask_11"));
  if (mask_11_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_11" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_11_net->second, set_simulation_value, &dut->mask_11)) {
         std::cerr << "cannot initialize callback for net mask_11" << std::endl;
     }
  }



  auto mask_12_net = netMap.find(std::string("mask_12"));
  if (mask_12_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_12" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_12_net->second, set_simulation_value, &dut->mask_12)) {
         std::cerr << "cannot initialize callback for net mask_12" << std::endl;
     }
  }



  auto mask_13_net = netMap.find(std::string("mask_13"));
  if (mask_13_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_13" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_13_net->second, set_simulation_value, &dut->mask_13)) {
         std::cerr << "cannot initialize callback for net mask_13" << std::endl;
     }
  }



  auto waddr_0_net = netMap.find(std::string("waddr_0"));
  if (waddr_0_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_0" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_0_net->second, set_simulation_value, &dut->waddr_0)) {
         std::cerr << "cannot initialize callback for net waddr_0" << std::endl;
     }
  }



  auto read_en_net = netMap.find(std::string("read_en"));
  if (read_en_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net read_en" << std::endl;
  }
  else {
     if (!sp.register_callback(read_en_net->second, set_simulation_value, &dut->read_en)) {
         std::cerr << "cannot initialize callback for net read_en" << std::endl;
     }
  }



  auto raddr_7_net = netMap.find(std::string("raddr_7"));
  if (raddr_7_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_7" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_7_net->second, set_simulation_value, &dut->raddr_7)) {
         std::cerr << "cannot initialize callback for net raddr_7" << std::endl;
     }
  }



  auto din_2_net = netMap.find(std::string("din_2"));
  if (din_2_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_2" << std::endl;
  }
  else {
     if (!sp.register_callback(din_2_net->second, set_simulation_value, &dut->din_2)) {
         std::cerr << "cannot initialize callback for net din_2" << std::endl;
     }
  }



  auto din_1_net = netMap.find(std::string("din_1"));
  if (din_1_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_1" << std::endl;
  }
  else {
     if (!sp.register_callback(din_1_net->second, set_simulation_value, &dut->din_1)) {
         std::cerr << "cannot initialize callback for net din_1" << std::endl;
     }
  }



  auto mask_14_net = netMap.find(std::string("mask_14"));
  if (mask_14_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_14" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_14_net->second, set_simulation_value, &dut->mask_14)) {
         std::cerr << "cannot initialize callback for net mask_14" << std::endl;
     }
  }



  auto waddr_2_net = netMap.find(std::string("waddr_2"));
  if (waddr_2_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_2" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_2_net->second, set_simulation_value, &dut->waddr_2)) {
         std::cerr << "cannot initialize callback for net waddr_2" << std::endl;
     }
  }



  auto mask_15_net = netMap.find(std::string("mask_15"));
  if (mask_15_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net mask_15" << std::endl;
  }
  else {
     if (!sp.register_callback(mask_15_net->second, set_simulation_value, &dut->mask_15)) {
         std::cerr << "cannot initialize callback for net mask_15" << std::endl;
     }
  }



  auto din_0_net = netMap.find(std::string("din_0"));
  if (din_0_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_0" << std::endl;
  }
  else {
     if (!sp.register_callback(din_0_net->second, set_simulation_value, &dut->din_0)) {
         std::cerr << "cannot initialize callback for net din_0" << std::endl;
     }
  }



  auto waddr_1_net = netMap.find(std::string("waddr_1"));
  if (waddr_1_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_1" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_1_net->second, set_simulation_value, &dut->waddr_1)) {
         std::cerr << "cannot initialize callback for net waddr_1" << std::endl;
     }
  }



  auto din_9_net = netMap.find(std::string("din_9"));
  if (din_9_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_9" << std::endl;
  }
  else {
     if (!sp.register_callback(din_9_net->second, set_simulation_value, &dut->din_9)) {
         std::cerr << "cannot initialize callback for net din_9" << std::endl;
     }
  }



  auto waddr_5_net = netMap.find(std::string("waddr_5"));
  if (waddr_5_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_5" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_5_net->second, set_simulation_value, &dut->waddr_5)) {
         std::cerr << "cannot initialize callback for net waddr_5" << std::endl;
     }
  }



  auto din_6_net = netMap.find(std::string("din_6"));
  if (din_6_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_6" << std::endl;
  }
  else {
     if (!sp.register_callback(din_6_net->second, set_simulation_value, &dut->din_6)) {
         std::cerr << "cannot initialize callback for net din_6" << std::endl;
     }
  }



  auto rclke_net = netMap.find(std::string("rclke"));
  if (rclke_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net rclke" << std::endl;
  }
  else {
     if (!sp.register_callback(rclke_net->second, set_simulation_value, &dut->rclke)) {
         std::cerr << "cannot initialize callback for net rclke" << std::endl;
     }
  }



  auto waddr_6_net = netMap.find(std::string("waddr_6"));
  if (waddr_6_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_6" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_6_net->second, set_simulation_value, &dut->waddr_6)) {
         std::cerr << "cannot initialize callback for net waddr_6" << std::endl;
     }
  }



  auto raddr_3_net = netMap.find(std::string("raddr_3"));
  if (raddr_3_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_3" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_3_net->second, set_simulation_value, &dut->raddr_3)) {
         std::cerr << "cannot initialize callback for net raddr_3" << std::endl;
     }
  }



  auto din_4_net = netMap.find(std::string("din_4"));
  if (din_4_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_4" << std::endl;
  }
  else {
     if (!sp.register_callback(din_4_net->second, set_simulation_value, &dut->din_4)) {
         std::cerr << "cannot initialize callback for net din_4" << std::endl;
     }
  }



  auto din_7_net = netMap.find(std::string("din_7"));
  if (din_7_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_7" << std::endl;
  }
  else {
     if (!sp.register_callback(din_7_net->second, set_simulation_value, &dut->din_7)) {
         std::cerr << "cannot initialize callback for net din_7" << std::endl;
     }
  }



  auto waddr_3_net = netMap.find(std::string("waddr_3"));
  if (waddr_3_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_3" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_3_net->second, set_simulation_value, &dut->waddr_3)) {
         std::cerr << "cannot initialize callback for net waddr_3" << std::endl;
     }
  }



  auto raddr_6_net = netMap.find(std::string("raddr_6"));
  if (raddr_6_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_6" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_6_net->second, set_simulation_value, &dut->raddr_6)) {
         std::cerr << "cannot initialize callback for net raddr_6" << std::endl;
     }
  }



  auto din_8_net = netMap.find(std::string("din_8"));
  if (din_8_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_8" << std::endl;
  }
  else {
     if (!sp.register_callback(din_8_net->second, set_simulation_value, &dut->din_8)) {
         std::cerr << "cannot initialize callback for net din_8" << std::endl;
     }
  }



  auto din_5_net = netMap.find(std::string("din_5"));
  if (din_5_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_5" << std::endl;
  }
  else {
     if (!sp.register_callback(din_5_net->second, set_simulation_value, &dut->din_5)) {
         std::cerr << "cannot initialize callback for net din_5" << std::endl;
     }
  }



  auto waddr_4_net = netMap.find(std::string("waddr_4"));
  if (waddr_4_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_4" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_4_net->second, set_simulation_value, &dut->waddr_4)) {
         std::cerr << "cannot initialize callback for net waddr_4" << std::endl;
     }
  }



  auto raddr_4_net = netMap.find(std::string("raddr_4"));
  if (raddr_4_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_4" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_4_net->second, set_simulation_value, &dut->raddr_4)) {
         std::cerr << "cannot initialize callback for net raddr_4" << std::endl;
     }
  }



  auto din_11_net = netMap.find(std::string("din_11"));
  if (din_11_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_11" << std::endl;
  }
  else {
     if (!sp.register_callback(din_11_net->second, set_simulation_value, &dut->din_11)) {
         std::cerr << "cannot initialize callback for net din_11" << std::endl;
     }
  }



  auto raddr_1_net = netMap.find(std::string("raddr_1"));
  if (raddr_1_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_1" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_1_net->second, set_simulation_value, &dut->raddr_1)) {
         std::cerr << "cannot initialize callback for net raddr_1" << std::endl;
     }
  }



  auto raddr_0_net = netMap.find(std::string("raddr_0"));
  if (raddr_0_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_0" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_0_net->second, set_simulation_value, &dut->raddr_0)) {
         std::cerr << "cannot initialize callback for net raddr_0" << std::endl;
     }
  }



  auto raddr_5_net = netMap.find(std::string("raddr_5"));
  if (raddr_5_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_5" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_5_net->second, set_simulation_value, &dut->raddr_5)) {
         std::cerr << "cannot initialize callback for net raddr_5" << std::endl;
     }
  }



  auto din_10_net = netMap.find(std::string("din_10"));
  if (din_10_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_10" << std::endl;
  }
  else {
     if (!sp.register_callback(din_10_net->second, set_simulation_value, &dut->din_10)) {
         std::cerr << "cannot initialize callback for net din_10" << std::endl;
     }
  }



  auto din_3_net = netMap.find(std::string("din_3"));
  if (din_3_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_3" << std::endl;
  }
  else {
     if (!sp.register_callback(din_3_net->second, set_simulation_value, &dut->din_3)) {
         std::cerr << "cannot initialize callback for net din_3" << std::endl;
     }
  }



  auto waddr_7_net = netMap.find(std::string("waddr_7"));
  if (waddr_7_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net waddr_7" << std::endl;
  }
  else {
     if (!sp.register_callback(waddr_7_net->second, set_simulation_value, &dut->waddr_7)) {
         std::cerr << "cannot initialize callback for net waddr_7" << std::endl;
     }
  }



  auto write_en_net = netMap.find(std::string("write_en"));
  if (write_en_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net write_en" << std::endl;
  }
  else {
     if (!sp.register_callback(write_en_net->second, set_simulation_value, &dut->write_en)) {
         std::cerr << "cannot initialize callback for net write_en" << std::endl;
     }
  }



  auto din_14_net = netMap.find(std::string("din_14"));
  if (din_14_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_14" << std::endl;
  }
  else {
     if (!sp.register_callback(din_14_net->second, set_simulation_value, &dut->din_14)) {
         std::cerr << "cannot initialize callback for net din_14" << std::endl;
     }
  }



  auto din_15_net = netMap.find(std::string("din_15"));
  if (din_15_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_15" << std::endl;
  }
  else {
     if (!sp.register_callback(din_15_net->second, set_simulation_value, &dut->din_15)) {
         std::cerr << "cannot initialize callback for net din_15" << std::endl;
     }
  }



  auto din_12_net = netMap.find(std::string("din_12"));
  if (din_12_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_12" << std::endl;
  }
  else {
     if (!sp.register_callback(din_12_net->second, set_simulation_value, &dut->din_12)) {
         std::cerr << "cannot initialize callback for net din_12" << std::endl;
     }
  }



  auto clk_net = netMap.find(std::string("clk"));
  if (clk_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net clk" << std::endl;
  }
  else {
     if (!sp.register_callback(clk_net->second, set_simulation_value, &dut->clk)) {
         std::cerr << "cannot initialize callback for net clk" << std::endl;
     }
  }



  auto din_13_net = netMap.find(std::string("din_13"));
  if (din_13_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net din_13" << std::endl;
  }
  else {
     if (!sp.register_callback(din_13_net->second, set_simulation_value, &dut->din_13)) {
         std::cerr << "cannot initialize callback for net din_13" << std::endl;
     }
  }



  auto raddr_2_net = netMap.find(std::string("raddr_2"));
  if (raddr_2_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net raddr_2" << std::endl;
  }
  else {
     if (!sp.register_callback(raddr_2_net->second, set_simulation_value, &dut->raddr_2)) {
         std::cerr << "cannot initialize callback for net raddr_2" << std::endl;
     }
  }



  auto wclke_net = netMap.find(std::string("wclke"));
  if (wclke_net == netMap.end()) {
     std::cerr << "no SALEAE input data found for net wclke" << std::endl;
  }
  else {
     if (!sp.register_callback(wclke_net->second, set_simulation_value, &dut->wclke)) {
         std::cerr << "cannot initialize callback for net wclke" << std::endl;
     }
  }





  int counter = 0;
  while (sp.next_event()) {
    counter++;
    if (counter % 1000000 == 0) {
      printf("%d\n", counter);
    }
  }
  
  dut->eval();
  //printf("dut->eval()\n");
  m_trace->dump(sim_time); // to_simulate already added in event loop
  //printf("m_trace->dump(%d);\n", sim_time);
  // m_trace->final();

  m_trace->close();
  delete dut;
  exit(EXIT_SUCCESS);
}
