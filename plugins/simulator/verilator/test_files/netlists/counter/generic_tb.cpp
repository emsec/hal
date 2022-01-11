#include "Vcounter.h"
#include "testbench.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include <ctype.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

TESTB<Vcounter>* tb;

double sc_time_stamp()
{
    return (double)tb->tb_sc_time_stamp();
}

int main(int argc, char** argv)
{
    Verilated::commandArgs(argc, argv);
    tb = new TESTB<Vcounter>();

    tb->opentrace("trace.vcd");

    tb->m_core->Reset = 0x1;
    tb->wait_for_n_clocks(1);

    tb->m_core->Reset = 0x0;
    tb->wait_for_n_clocks(1);

    tb->m_core->Clock_enable_B = 0x1;
    tb->wait_for_n_clocks(5);


    tb->m_core->Clock_enable_B = 0x0;
    tb->wait_for_n_clocks(2);


    tb->m_core->Reset = 0x1;
    tb->wait_for_n_clocks(2);


    tb->m_core->Reset = 0x0;
    tb->wait_for_n_clocks(1);


    tb->m_core->Clock_enable_B = 0x1;
    tb->wait_for_n_clocks(20);

    printf("simulation done\n");

    return 0;
}