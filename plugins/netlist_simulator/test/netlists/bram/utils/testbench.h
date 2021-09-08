////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	testb.h
//
// Project:	dbgbus, a collection of 8b channel to WB bus debugging protocols
//
// Purpose:	A wrapper for a common interface to a clocked FPGA core
//		begin exercised in Verilator.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015,2017-2020, Gisselquist Technology, LLC
//
// This file is part of the debugging interface demonstration.
//
// The debugging interface demonstration is free software (firmware): you can
// redistribute it and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// This debugging interface demonstration is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
// General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  (It's in the $(ROOT)/doc directory.  Run make
// with no target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	LGPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/lgpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
#ifndef TESTB_H
#define TESTB_H

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <verilated_vcd_c.h>

#define TBASSERT(TB, A)        \
    do                         \
    {                          \
        if (!(A))              \
        {                      \
            (TB).closetrace(); \
        }                      \
        assert(A);             \
    } while (0);

template<class VA>
class TESTB
{
public:
    VA* m_core;
    VerilatedVcdC* m_trace;
    unsigned long m_tickcount;

    TESTB(void) : m_trace(NULL), m_tickcount(0l)
    {
        m_core = new VA;
        Verilated::traceEverOn(true);
        m_core->clk = 0;
        eval();    // Get our initial values set properly.
    }
    virtual ~TESTB(void)
    {
        if (m_trace)
            m_trace->close();
        delete m_core;
        m_core = NULL;
    }

    virtual unsigned long tb_sc_time_stamp(void)
    {
        return m_tickcount;
    }

    virtual void opentrace(const char* vcdname)
    {
        if (!m_trace)
        {
            m_trace = new VerilatedVcdC;
            m_core->trace(m_trace, 0);
            m_trace->open(vcdname);
        }
    }

    virtual void closetrace(void)
    {
        if (m_trace)
        {
            m_trace->close();
            m_trace = NULL;
        }
    }

    virtual void eval(void)
    {
        m_core->eval();
    }

    virtual void tick(void)
    {
        m_tickcount++;

        // Toggle the clock

        // Rising edge
        m_core->clk = 1;
        m_core->eval();

        // Falling edge
        m_core->clk = 0;
        m_core->eval();
    }

    virtual void wait_for_n_clocks(int ticks)
    {
        int counter = 0;

        while (counter < ticks)
        {
            tick();
            counter++;
        }
    }
};

#endif
