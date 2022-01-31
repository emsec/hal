#include "Vbram_netlist.h"

#include <iostream>
#include <stdlib.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env)
{
    Vbram_netlist* dut = new Vbram_netlist;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;

    dut->trace(m_trace, 1);
    m_trace->open("waveform.vcd");

    uint16_t data_write = 0xffff;
    uint16_t data_read  = 0x0000;
    uint8_t addr        = 0xff;

    dut->din_0  = 0x0;
    dut->din_1  = 0x0;
    dut->din_2  = 0x0;
    dut->din_3  = 0x0;
    dut->din_4  = 0x0;
    dut->din_5  = 0x0;
    dut->din_6  = 0x0;
    dut->din_7  = 0x0;
    dut->din_8  = 0x0;
    dut->din_9  = 0x0;
    dut->din_10 = 0x0;
    dut->din_11 = 0x0;
    dut->din_12 = 0x0;
    dut->din_13 = 0x0;
    dut->din_14 = 0x0;
    dut->din_15 = 0x0;


    dut->raddr_0  = 0x0;
    dut->raddr_1  = 0x0;
    dut->raddr_2  = 0x0;
    dut->raddr_3  = 0x0;
    dut->raddr_4  = 0x0;
    dut->raddr_5  = 0x0;
    dut->raddr_6  = 0x0;
    dut->raddr_7  = 0x0;

    dut->waddr_0  = 0x0;
    dut->waddr_1  = 0x0;
    dut->waddr_2  = 0x0;
    dut->waddr_3  = 0x0;
    dut->waddr_4  = 0x0;
    dut->waddr_5  = 0x0;
    dut->waddr_6  = 0x0;
    dut->waddr_7  = 0x0;


    dut->mask_0  = 0x0;
    dut->mask_1  = 0x0;
    dut->mask_2  = 0x0;
    dut->mask_3  = 0x0;
    dut->mask_4  = 0x0;
    dut->mask_5  = 0x0;
    dut->mask_6  = 0x0;
    dut->mask_7  = 0x0;
    dut->mask_8  = 0x0;
    dut->mask_9  = 0x0;
    dut->mask_10 = 0x0;
    dut->mask_11 = 0x0;
    dut->mask_12 = 0x0;
    dut->mask_13 = 0x0;
    dut->mask_14 = 0x0;
    dut->mask_15 = 0x0;

    dut->rclke = 0x0;
    dut->read_en = 0x0;
    dut->wclke = 0x0;


    dut->clk = 0x0;
    dut->clk = 0x1;

    sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

    // write data without wclke
    // waddr       <= x"ff";
    for (const auto& write_addr_net : write_addr)
    {
        sim_ctrl_verilator->set_input(write_addr_net, BooleanFunction::Value::ONE);
    }
    // din         <= x"ffff";
    for (const auto& din_net : din)
    {
        sim_ctrl_verilator->set_input(din_net, BooleanFunction::Value::ONE);
    }

    sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

    sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= '1';

    sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

    sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ZERO);    // write_en    <= '0';
    sim_ctrl_verilator->simulate(1 * clock_period);                           // WAIT FOR 10 NS;

    // read data without rclke
    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en     <= '1';

    // raddr       <= x"ff";
    for (const auto& read_addr_net : read_addr)
    {
        sim_ctrl_verilator->set_input(read_addr_net, BooleanFunction::Value::ONE);
    }

    sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

    sim_ctrl_verilator->simulate(5 * clock_period);    // WAIT FOR 50 NS;
    // printf("sent %08x, received: %08x\n", data_write, data_read);

    // // write data with wclke
    //  waddr   <= x"ff";
    for (const auto& write_addr_net : write_addr)
    {
        sim_ctrl_verilator->set_input(write_addr_net, BooleanFunction::Value::ONE);
    }
    // din     <= x"ffff";
    for (const auto& din_net : din)
    {
        sim_ctrl_verilator->set_input(din_net, BooleanFunction::Value::ONE);
    }

    sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

    sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= '1';
    sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ONE);       // wclke       <= '1';

    sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

    sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ZERO);    // write_en    <= '0';
    sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ZERO);       // wclke       <= '0';

    sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

    // // read data without rclke
    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= '1';

    // raddr      <= x"ff";
    for (const auto& read_addr_net : read_addr)
    {
        sim_ctrl_verilator->set_input(read_addr_net, BooleanFunction::Value::ONE);
    }

    sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

    // data_read = read_data();

    sim_ctrl_verilator->simulate(5 * clock_period);    // WAIT FOR 50 NS;
    // printf("sent %08x, received: %08x\n", data_write, data_read);

    // // read data with rclke
    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= '1';
    // raddr      <= x"ff";
    for (const auto& read_addr_net : read_addr)
    {
        sim_ctrl_verilator->set_input(read_addr_net, BooleanFunction::Value::ONE);
    }
    sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);    // rclke      <= '1';

    sim_ctrl_verilator->simulate(2 * clock_period);                        // WAIT FOR 20 NS;
    sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ZERO);    // rclke      <= '0';

    sim_ctrl_verilator->simulate(5 * clock_period);    // WAIT FOR 50 NS;

    // read some address and see what the result from INIT value is
    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= '1';

    // todo: bitorder could be wrong?
    //raddr      <= x"66";
    sim_ctrl_verilator->set_input(read_addr.at(7), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(read_addr.at(6), BooleanFunction::Value::ONE);
    //            sim_ctrl_verilator->set_input(read_addr.at(5), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(read_addr.at(4), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(read_addr.at(3), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(read_addr.at(2), BooleanFunction::Value::ONE);
    //            sim_ctrl_verilator->set_input(read_addr.at(1), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(read_addr.at(0), BooleanFunction::Value::ZERO);

    sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);    // rclke      <= '1';

    sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

    //data_read = read_data();

    // waddr       <= x"43"; 0100 0011
    sim_ctrl_verilator->set_input(write_addr.at(7), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(write_addr.at(6), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(write_addr.at(5), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(write_addr.at(4), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(write_addr.at(3), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(write_addr.at(2), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(write_addr.at(1), BooleanFunction::Value::ONE);
    //            sim_ctrl_verilator->set_input(write_addr.at(0), BooleanFunction::Value::ONE);

    // din         <= x"1111";
    sim_ctrl_verilator->set_input(din.at(15), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(14), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(13), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(12), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(11), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(10), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(9), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(8), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(7), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(6), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(5), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(4), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(3), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(2), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(din.at(1), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(0), BooleanFunction::Value::ONE);

    sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

    sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= '1';
    sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ONE);       // wclke       <= '1';
    sim_ctrl_verilator->simulate(2 * clock_period);                          // WAIT FOR 20 NS;

    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= 1';
    sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);      // rclke       <= '1';

    // raddr      <= x"43";
    //            sim_ctrl_verilator->set_input(read_addr.at(7), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(read_addr.at(6), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(read_addr.at(5), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(read_addr.at(4), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(read_addr.at(3), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(read_addr.at(2), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(read_addr.at(1), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(read_addr.at(0), BooleanFunction::Value::ONE);

    sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

    // din <= x "ff11";
    sim_ctrl_verilator->set_input(din.at(15), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(14), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(13), BooleanFunction::Value::ONE);
    //            sim_ctrl_verilator->set_input(din.at(12), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(11), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(10), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(din.at(9), BooleanFunction::Value::ONE);
    //            sim_ctrl_verilator->set_input(din.at(8), BooleanFunction::Value::ONE);
    //            sim_ctrl_verilator->set_input(din.at(7), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(6), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(5), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(4), BooleanFunction::Value::ONE);
    //            sim_ctrl_verilator->set_input(din.at(3), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(2), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(1), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(din.at(0), BooleanFunction::Value::ONE);

    sim_ctrl_verilator->simulate(20 * clock_period);    // WAIT FOR 20 NS;

    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ZERO);    // read_en    <= 0';
    sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ZERO);      // rclke       <= '0';
    sim_ctrl_verilator->simulate(2 * clock_period);                          // WAIT FOR 20 NS;

    // mask        <= x"ffff";
    sim_ctrl_verilator->set_input(mask.at(15), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(14), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(13), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(12), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(11), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(10), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(9), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(8), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(7), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(6), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(5), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(4), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(3), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(2), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(1), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(0), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

    sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ZERO);    // write_en    <= 0';
    sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ZERO);       // wclke       <= '0';
    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);      // read_en    <= 1';
    sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);        // rclke       <= '1';
    sim_ctrl_verilator->simulate(2 * clock_period);                           // WAIT FOR 20 NS;

    sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= 1';
    sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ONE);       // wclke       <= '1';
    sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ZERO);    // read_en    <= 0';
    sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ZERO);      // rclke       <= '0';

    // mask        <= x"1111";
    sim_ctrl_verilator->set_input(mask.at(15), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(14), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(13), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(mask.at(12), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(11), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(10), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(9), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(mask.at(8), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(7), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(6), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(5), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(mask.at(4), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->set_input(mask.at(3), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(2), BooleanFunction::Value::ZERO);
    sim_ctrl_verilator->set_input(mask.at(1), BooleanFunction::Value::ZERO);
    //            sim_ctrl_verilator->set_input(mask.at(0), BooleanFunction::Value::ONE);
    sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

    sim_ctrl_verilator->simulate(100 * clock_period);    // WAIT FOR 100*10 NS;
    m_trace->dump(sim_time);

    m_trace->close();
    delete dut;
    exit(EXIT_SUCCESS);
}