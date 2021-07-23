#include "Vbram_netlist.h"
#include "utils/testbench.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include <ctype.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

TESTB<Vbram_netlist>* tb;

double sc_time_stamp()
{
    return (double)tb->tb_sc_time_stamp();
}

void write_data(uint16_t din)
{
    tb->m_core->din_15 = 0x1 & (din >> 15);
    tb->m_core->din_14 = 0x1 & (din >> 14);
    tb->m_core->din_13 = 0x1 & (din >> 13);
    tb->m_core->din_12 = 0x1 & (din >> 12);
    tb->m_core->din_11 = 0x1 & (din >> 11);
    tb->m_core->din_10 = 0x1 & (din >> 10);
    tb->m_core->din_9  = 0x1 & (din >> 9);
    tb->m_core->din_8  = 0x1 & (din >> 8);
    tb->m_core->din_7  = 0x1 & (din >> 7);
    tb->m_core->din_6  = 0x1 & (din >> 6);
    tb->m_core->din_5  = 0x1 & (din >> 5);
    tb->m_core->din_4  = 0x1 & (din >> 4);
    tb->m_core->din_3  = 0x1 & (din >> 3);
    tb->m_core->din_2  = 0x1 & (din >> 2);
    tb->m_core->din_1  = 0x1 & (din >> 1);
    tb->m_core->din_0  = 0x1 & (din);
}

uint16_t read_data()
{
    uint16_t dout = 0x0;
    dout |= tb->m_core->dout_15 << 15;
    dout |= tb->m_core->dout_14 << 14;
    dout |= tb->m_core->dout_13 << 13;
    dout |= tb->m_core->dout_12 << 12;
    dout |= tb->m_core->dout_11 << 11;
    dout |= tb->m_core->dout_10 << 10;
    dout |= tb->m_core->dout_9 << 9;
    dout |= tb->m_core->dout_8 << 8;
    dout |= tb->m_core->dout_7 << 7;
    dout |= tb->m_core->dout_6 << 6;
    dout |= tb->m_core->dout_5 << 5;
    dout |= tb->m_core->dout_4 << 4;
    dout |= tb->m_core->dout_3 << 3;
    dout |= tb->m_core->dout_2 << 2;
    dout |= tb->m_core->dout_1 << 1;
    dout |= tb->m_core->dout_0 << 0;
    return dout;
}

void set_write_addr(uint8_t waddr)
{
    tb->m_core->waddr_7 = 0x1 & (waddr >> 7);
    tb->m_core->waddr_6 = 0x1 & (waddr >> 6);
    tb->m_core->waddr_5 = 0x1 & (waddr >> 5);
    tb->m_core->waddr_4 = 0x1 & (waddr >> 4);
    tb->m_core->waddr_3 = 0x1 & (waddr >> 3);
    tb->m_core->waddr_2 = 0x1 & (waddr >> 2);
    tb->m_core->waddr_1 = 0x1 & (waddr >> 1);
    tb->m_core->waddr_0 = 0x1 & (waddr >> 0);
}

void set_read_addr(uint8_t raddr)
{
    tb->m_core->raddr_7 = 0x1 & (raddr >> 7);
    tb->m_core->raddr_6 = 0x1 & (raddr >> 6);
    tb->m_core->raddr_5 = 0x1 & (raddr >> 5);
    tb->m_core->raddr_4 = 0x1 & (raddr >> 4);
    tb->m_core->raddr_3 = 0x1 & (raddr >> 3);
    tb->m_core->raddr_2 = 0x1 & (raddr >> 2);
    tb->m_core->raddr_1 = 0x1 & (raddr >> 1);
    tb->m_core->raddr_0 = 0x1 & (raddr >> 0);
}

void set_write_en(char write_en)
{
    tb->m_core->write_en = write_en;
}
void set_read_en(char read_en)
{
    tb->m_core->read_en = read_en;
}
void set_rclke(char rclke)
{
    tb->m_core->rclke = rclke;
}
void set_wclke(char wclke)
{
    tb->m_core->wclke = wclke;
}

int main(int argc, char** argv)
{
    Verilated::commandArgs(argc, argv);
    tb = new TESTB<Vbram_netlist>();

    uint16_t data_write = 0xffff;
    uint16_t data_read  = 0x0000;
    uint8_t addr        = 0xff;

    tb->m_core->rclke = 0x0;
    tb->m_core->wclke = 0x0;

    tb->wait_for_n_clocks(1);

    tb->opentrace("trace.vcd");

    tb->wait_for_n_clocks(5);

    // write data without wclke
    set_write_addr(addr);
    write_data(data_write);

    tb->wait_for_n_clocks(1);

    set_write_en(0x1);

    tb->wait_for_n_clocks(1);

    set_write_en(0x0);
    tb->wait_for_n_clocks(1);

    // read data without rclke
    set_read_en(0x1);
    set_read_addr(addr);

    tb->wait_for_n_clocks(2);

    data_read = read_data();

    tb->wait_for_n_clocks(5);
    printf("sent %04x, received: %04x\n", data_write, data_read);

    // write data with wclke
    set_write_addr(addr);
    write_data(data_write);

    tb->wait_for_n_clocks(1);

    set_write_en(0x1);
    set_wclke(0x1);

    tb->wait_for_n_clocks(1);

    set_write_en(0x0);
    set_wclke(0x0);

    tb->wait_for_n_clocks(1);

    // read data without rclke
    set_read_en(0x1);
    set_read_addr(addr);

    tb->wait_for_n_clocks(2);

    data_read = read_data();

    tb->wait_for_n_clocks(5);
    printf("sent %04x, received: %04x\n", data_write, data_read);

    // read data with rclke
    set_read_en(0x1);
    set_read_addr(addr);
    set_rclke(0x1);

    tb->wait_for_n_clocks(2);
    set_rclke(0x0);

    data_read = read_data();

    tb->wait_for_n_clocks(5);
    printf("sent %04x, received: %04x\n", data_write, data_read);

    // read some address and see what the result from INIT value is
    set_read_en(0x1);
    set_read_addr(0x60);
    set_rclke(0x1);

    tb->wait_for_n_clocks(2);
    set_rclke(0x0);

    data_read = read_data();
    printf("addr %02x, received: %04x\n", 0x60, data_read);

    tb->wait_for_n_clocks(100);

    printf("simulation done\n");

    return 0;
}
