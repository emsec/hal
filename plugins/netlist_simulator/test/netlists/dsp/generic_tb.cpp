#include "Vdsp_netlist.h"
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

TESTB<Vdsp_netlist>* tb;

double sc_time_stamp()
{
    return (double)tb->tb_sc_time_stamp();
}

void set_a(uint16_t A)
{
    tb->m_core->A_15 = 0x1 & (A >> 15);
    tb->m_core->A_14 = 0x1 & (A >> 14);
    tb->m_core->A_13 = 0x1 & (A >> 13);
    tb->m_core->A_12 = 0x1 & (A >> 12);
    tb->m_core->A_11 = 0x1 & (A >> 11);
    tb->m_core->A_10 = 0x1 & (A >> 10);
    tb->m_core->A_9  = 0x1 & (A >> 9);
    tb->m_core->A_8  = 0x1 & (A >> 8);
    tb->m_core->A_7  = 0x1 & (A >> 7);
    tb->m_core->A_6  = 0x1 & (A >> 6);
    tb->m_core->A_5  = 0x1 & (A >> 5);
    tb->m_core->A_4  = 0x1 & (A >> 4);
    tb->m_core->A_3  = 0x1 & (A >> 3);
    tb->m_core->A_2  = 0x1 & (A >> 2);
    tb->m_core->A_1  = 0x1 & (A >> 1);
    tb->m_core->A_0  = 0x1 & (A);
}

void set_b(uint16_t B)
{
    tb->m_core->B_15 = 0x1 & (B >> 15);
    tb->m_core->B_14 = 0x1 & (B >> 14);
    tb->m_core->B_13 = 0x1 & (B >> 13);
    tb->m_core->B_12 = 0x1 & (B >> 12);
    tb->m_core->B_11 = 0x1 & (B >> 11);
    tb->m_core->B_10 = 0x1 & (B >> 10);
    tb->m_core->B_9  = 0x1 & (B >> 9);
    tb->m_core->B_8  = 0x1 & (B >> 8);
    tb->m_core->B_7  = 0x1 & (B >> 7);
    tb->m_core->B_6  = 0x1 & (B >> 6);
    tb->m_core->B_5  = 0x1 & (B >> 5);
    tb->m_core->B_4  = 0x1 & (B >> 4);
    tb->m_core->B_3  = 0x1 & (B >> 3);
    tb->m_core->B_2  = 0x1 & (B >> 2);
    tb->m_core->B_1  = 0x1 & (B >> 1);
    tb->m_core->B_0  = 0x1 & (B);
}

void set_c(uint16_t C)
{
    tb->m_core->C_15 = 0x1 & (C >> 15);
    tb->m_core->C_14 = 0x1 & (C >> 14);
    tb->m_core->C_13 = 0x1 & (C >> 13);
    tb->m_core->C_12 = 0x1 & (C >> 12);
    tb->m_core->C_11 = 0x1 & (C >> 11);
    tb->m_core->C_10 = 0x1 & (C >> 10);
    tb->m_core->C_9  = 0x1 & (C >> 9);
    tb->m_core->C_8  = 0x1 & (C >> 8);
    tb->m_core->C_7  = 0x1 & (C >> 7);
    tb->m_core->C_6  = 0x1 & (C >> 6);
    tb->m_core->C_5  = 0x1 & (C >> 5);
    tb->m_core->C_4  = 0x1 & (C >> 4);
    tb->m_core->C_3  = 0x1 & (C >> 3);
    tb->m_core->C_2  = 0x1 & (C >> 2);
    tb->m_core->C_1  = 0x1 & (C >> 1);
    tb->m_core->C_0  = 0x1 & (C);
}

void set_d(uint16_t D)
{
    tb->m_core->D_15 = 0x1 & (D >> 15);
    tb->m_core->D_14 = 0x1 & (D >> 14);
    tb->m_core->D_13 = 0x1 & (D >> 13);
    tb->m_core->D_12 = 0x1 & (D >> 12);
    tb->m_core->D_11 = 0x1 & (D >> 11);
    tb->m_core->D_10 = 0x1 & (D >> 10);
    tb->m_core->D_9  = 0x1 & (D >> 9);
    tb->m_core->D_8  = 0x1 & (D >> 8);
    tb->m_core->D_7  = 0x1 & (D >> 7);
    tb->m_core->D_6  = 0x1 & (D >> 6);
    tb->m_core->D_5  = 0x1 & (D >> 5);
    tb->m_core->D_4  = 0x1 & (D >> 4);
    tb->m_core->D_3  = 0x1 & (D >> 3);
    tb->m_core->D_2  = 0x1 & (D >> 2);
    tb->m_core->D_1  = 0x1 & (D >> 1);
    tb->m_core->D_0  = 0x1 & (D);
}

uint32_t get_o()
{
    uint32_t O = 0;
    O |= tb->m_core->O_31 >> 31;
    O |= tb->m_core->O_30 >> 30;
    O |= tb->m_core->O_29 >> 29;
    O |= tb->m_core->O_28 >> 28;
    O |= tb->m_core->O_27 >> 27;
    O |= tb->m_core->O_26 >> 26;
    O |= tb->m_core->O_25 >> 25;
    O |= tb->m_core->O_24 >> 24;
    O |= tb->m_core->O_23 >> 23;
    O |= tb->m_core->O_22 >> 22;
    O |= tb->m_core->O_21 >> 21;
    O |= tb->m_core->O_20 >> 20;
    O |= tb->m_core->O_19 >> 19;
    O |= tb->m_core->O_18 >> 18;
    O |= tb->m_core->O_17 >> 17;
    O |= tb->m_core->O_16 >> 16;
    O |= tb->m_core->O_15 >> 15;
    O |= tb->m_core->O_14 >> 14;
    O |= tb->m_core->O_13 >> 13;
    O |= tb->m_core->O_12 >> 12;
    O |= tb->m_core->O_11 >> 11;
    O |= tb->m_core->O_10 >> 10;
    O |= tb->m_core->O_9 >> 9;
    O |= tb->m_core->O_8 >> 8;
    O |= tb->m_core->O_7 >> 7;
    O |= tb->m_core->O_6 >> 6;
    O |= tb->m_core->O_5 >> 5;
    O |= tb->m_core->O_4 >> 4;
    O |= tb->m_core->O_3 >> 3;
    O |= tb->m_core->O_2 >> 2;
    O |= tb->m_core->O_1 >> 1;
    O |= tb->m_core->O_0 >> 0;
    return O;
}

int main(int argc, char** argv)
{
    Verilated::commandArgs(argc, argv);
    tb = new TESTB<Vdsp_netlist>();

    set_a(6666);
    set_b(1234);
    set_c(4371);
    set_d(43714);

    tb->m_core->AHOLD     = 0x0;
    tb->m_core->BHOLD     = 0x0;
    tb->m_core->CHOLD     = 0x0;
    tb->m_core->DHOLD     = 0x0;
    tb->m_core->OHOLDTOP  = 0x0;
    tb->m_core->OHOLDBOT  = 0x0;
    tb->m_core->OLOADTOP  = 0x0;
    tb->m_core->OLOADBOT  = 0x0;
    tb->m_core->ADDSUBTOP = 0x0;
    tb->m_core->ADDSUBBOT = 0x0;
    tb->m_core->CI        = 0x0;
    tb->m_core->CE        = 0x0;

    tb->opentrace("trace.vcd");

    tb->wait_for_n_clocks(1);

    tb->m_core->CE = 0x0;

    tb->m_core->IRSTTOP = 0x1;
    tb->m_core->IRSTBOT = 0x1;
    tb->m_core->ORSTTOP = 0x1;
    tb->m_core->ORSTBOT = 0x1;

    tb->wait_for_n_clocks(10);

    tb->m_core->CE = 0x1;

    tb->wait_for_n_clocks(10);

    tb->m_core->IRSTTOP = 0x0;
    tb->m_core->IRSTBOT = 0x0;
    tb->m_core->ORSTTOP = 0x0;
    tb->m_core->ORSTBOT = 0x0;

    tb->wait_for_n_clocks(10);

    tb->m_core->AHOLD = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->OHOLDTOP = 0x1;
    tb->m_core->OHOLDBOT = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->AHOLD    = 0x0;
    tb->m_core->OHOLDTOP = 0x0;
    tb->m_core->OHOLDBOT = 0x0;
    tb->wait_for_n_clocks(5);

    tb->m_core->OLOADTOP  = 0x1;
    tb->m_core->ADDSUBTOP = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->OLOADTOP  = 0x0;
    tb->m_core->ADDSUBTOP = 0x0;
    tb->m_core->ADDSUBBOT = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->ORSTTOP = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->ADDSUBBOT = 0x0;
    tb->m_core->ORSTTOP   = 0x0;
    tb->wait_for_n_clocks(5);

    tb->m_core->IRSTTOP = 0x1;
    tb->m_core->CHOLD   = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->IRSTTOP = 0x0;
    tb->m_core->CHOLD   = 0x0;

    set_a(1632);
    set_b(764254);
    set_c(12523);
    set_d(263);
    tb->wait_for_n_clocks(5);

    tb->m_core->OLOADTOP = 0x1;
    tb->m_core->OLOADBOT = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->OLOADTOP = 0x0;
    tb->m_core->OLOADBOT = 0x0;
    tb->wait_for_n_clocks(5);

    tb->m_core->ADDSUBTOP = 0x1;
    tb->m_core->ADDSUBBOT = 0x0;
    tb->wait_for_n_clocks(5);

    tb->m_core->ADDSUBTOP = 0x0;
    tb->m_core->ADDSUBBOT = 0x1;
    tb->wait_for_n_clocks(5);

    tb->m_core->ADDSUBTOP = 0x0;
    tb->m_core->ADDSUBBOT = 0x0;
    tb->wait_for_n_clocks(5);

    tb->wait_for_n_clocks(5);

    printf("simulation done\n");

    return 0;
}
