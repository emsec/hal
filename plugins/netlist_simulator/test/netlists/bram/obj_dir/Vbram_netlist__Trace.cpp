// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals
#include "verilated_vcd_c.h"
#include "Vbram_netlist__Syms.h"


void Vbram_netlist::traceChgTop0(void* userp, VerilatedVcd* tracep) {
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Variables
    if (VL_UNLIKELY(!vlSymsp->__Vm_activity)) return;
    // Body
    {
        vlTOPp->traceChgSub0(userp, tracep);
    }
}

void Vbram_netlist::traceChgSub0(void* userp, VerilatedVcd* tracep) {
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    vluint32_t* const oldp = tracep->oldp(vlSymsp->__Vm_baseCode + 1);
    if (false && oldp) {}  // Prevent unused
    // Body
    {
        tracep->chgBit(oldp+0,(vlTOPp->clk));
        tracep->chgBit(oldp+1,(vlTOPp->raddr_4));
        tracep->chgBit(oldp+2,(vlTOPp->waddr_4));
        tracep->chgBit(oldp+3,(vlTOPp->din_11));
        tracep->chgBit(oldp+4,(vlTOPp->raddr_6));
        tracep->chgBit(oldp+5,(vlTOPp->raddr_3));
        tracep->chgBit(oldp+6,(vlTOPp->raddr_5));
        tracep->chgBit(oldp+7,(vlTOPp->raddr_1));
        tracep->chgBit(oldp+8,(vlTOPp->waddr_3));
        tracep->chgBit(oldp+9,(vlTOPp->din_4));
        tracep->chgBit(oldp+10,(vlTOPp->din_5));
        tracep->chgBit(oldp+11,(vlTOPp->din_9));
        tracep->chgBit(oldp+12,(vlTOPp->din_10));
        tracep->chgBit(oldp+13,(vlTOPp->raddr_0));
        tracep->chgBit(oldp+14,(vlTOPp->din_8));
        tracep->chgBit(oldp+15,(vlTOPp->din_6));
        tracep->chgBit(oldp+16,(vlTOPp->din_7));
        tracep->chgBit(oldp+17,(vlTOPp->waddr_5));
        tracep->chgBit(oldp+18,(vlTOPp->waddr_6));
        tracep->chgBit(oldp+19,(vlTOPp->rclke));
        tracep->chgBit(oldp+20,(vlTOPp->din_3));
        tracep->chgBit(oldp+21,(vlTOPp->waddr_7));
        tracep->chgBit(oldp+22,(vlTOPp->write_en));
        tracep->chgBit(oldp+23,(vlTOPp->din_14));
        tracep->chgBit(oldp+24,(vlTOPp->din_15));
        tracep->chgBit(oldp+25,(vlTOPp->din_12));
        tracep->chgBit(oldp+26,(vlTOPp->din_13));
        tracep->chgBit(oldp+27,(vlTOPp->raddr_2));
        tracep->chgBit(oldp+28,(vlTOPp->wclke));
        tracep->chgBit(oldp+29,(vlTOPp->waddr_1));
        tracep->chgBit(oldp+30,(vlTOPp->din_0));
        tracep->chgBit(oldp+31,(vlTOPp->waddr_2));
        tracep->chgBit(oldp+32,(vlTOPp->din_1));
        tracep->chgBit(oldp+33,(vlTOPp->din_2));
        tracep->chgBit(oldp+34,(vlTOPp->raddr_7));
        tracep->chgBit(oldp+35,(vlTOPp->read_en));
        tracep->chgBit(oldp+36,(vlTOPp->waddr_0));
        tracep->chgBit(oldp+37,(vlTOPp->dout_12));
        tracep->chgBit(oldp+38,(vlTOPp->dout_6));
        tracep->chgBit(oldp+39,(vlTOPp->dout_3));
        tracep->chgBit(oldp+40,(vlTOPp->dout_1));
        tracep->chgBit(oldp+41,(vlTOPp->dout_10));
        tracep->chgBit(oldp+42,(vlTOPp->dout_0));
        tracep->chgBit(oldp+43,(vlTOPp->dout_8));
        tracep->chgBit(oldp+44,(vlTOPp->dout_11));
        tracep->chgBit(oldp+45,(vlTOPp->dout_4));
        tracep->chgBit(oldp+46,(vlTOPp->dout_5));
        tracep->chgBit(oldp+47,(vlTOPp->dout_2));
        tracep->chgBit(oldp+48,(vlTOPp->dout_9));
        tracep->chgBit(oldp+49,(vlTOPp->dout_7));
        tracep->chgBit(oldp+50,(vlTOPp->dout_13));
        tracep->chgBit(oldp+51,(vlTOPp->dout_14));
        tracep->chgBit(oldp+52,(vlTOPp->dout_15));
    }
}

void Vbram_netlist::traceCleanup(void* userp, VerilatedVcd* /*unused*/) {
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    {
        vlSymsp->__Vm_activity = false;
        vlTOPp->__Vm_traceActivity[0U] = 0U;
    }
}
