// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals
#include "verilated_vcd_c.h"
#include "Vbram_netlist__Syms.h"


//======================

void Vbram_netlist::trace(VerilatedVcdC* tfp, int, int) {
    tfp->spTrace()->addInitCb(&traceInit, __VlSymsp);
    traceRegister(tfp->spTrace());
}

void Vbram_netlist::traceInit(void* userp, VerilatedVcd* tracep, uint32_t code) {
    // Callback from tracep->open()
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    if (!vlSymsp->_vm_contextp__->calcUnusedSigs()) {
        VL_FATAL_MT(__FILE__, __LINE__, __FILE__,
                        "Turning on wave traces requires Verilated::traceEverOn(true) call before time 0.");
    }
    vlSymsp->__Vm_baseCode = code;
    tracep->module(vlSymsp->name());
    tracep->scopeEscape(' ');
    Vbram_netlist::traceInitTop(vlSymsp, tracep);
    tracep->scopeEscape('.');
}

//======================


void Vbram_netlist::traceInitTop(void* userp, VerilatedVcd* tracep) {
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    {
        vlTOPp->traceInitSub0(userp, tracep);
    }
}

void Vbram_netlist::traceInitSub0(void* userp, VerilatedVcd* tracep) {
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    const int c = vlSymsp->__Vm_baseCode;
    if (false && tracep && c) {}  // Prevent unused
    // Body
    {
        tracep->declBit(c+1,"clk", false,-1);
        tracep->declBit(c+2,"raddr_4", false,-1);
        tracep->declBit(c+3,"waddr_4", false,-1);
        tracep->declBit(c+4,"din_11", false,-1);
        tracep->declBit(c+5,"raddr_6", false,-1);
        tracep->declBit(c+6,"raddr_3", false,-1);
        tracep->declBit(c+7,"raddr_5", false,-1);
        tracep->declBit(c+8,"raddr_1", false,-1);
        tracep->declBit(c+9,"waddr_3", false,-1);
        tracep->declBit(c+10,"din_4", false,-1);
        tracep->declBit(c+11,"din_5", false,-1);
        tracep->declBit(c+12,"din_9", false,-1);
        tracep->declBit(c+13,"din_10", false,-1);
        tracep->declBit(c+14,"raddr_0", false,-1);
        tracep->declBit(c+15,"din_8", false,-1);
        tracep->declBit(c+16,"din_6", false,-1);
        tracep->declBit(c+17,"din_7", false,-1);
        tracep->declBit(c+18,"waddr_5", false,-1);
        tracep->declBit(c+19,"waddr_6", false,-1);
        tracep->declBit(c+20,"rclke", false,-1);
        tracep->declBit(c+21,"din_3", false,-1);
        tracep->declBit(c+22,"waddr_7", false,-1);
        tracep->declBit(c+23,"write_en", false,-1);
        tracep->declBit(c+24,"din_14", false,-1);
        tracep->declBit(c+25,"din_15", false,-1);
        tracep->declBit(c+26,"din_12", false,-1);
        tracep->declBit(c+27,"din_13", false,-1);
        tracep->declBit(c+28,"raddr_2", false,-1);
        tracep->declBit(c+29,"wclke", false,-1);
        tracep->declBit(c+30,"waddr_1", false,-1);
        tracep->declBit(c+31,"din_0", false,-1);
        tracep->declBit(c+32,"waddr_2", false,-1);
        tracep->declBit(c+33,"din_1", false,-1);
        tracep->declBit(c+34,"din_2", false,-1);
        tracep->declBit(c+35,"raddr_7", false,-1);
        tracep->declBit(c+36,"read_en", false,-1);
        tracep->declBit(c+37,"waddr_0", false,-1);
        tracep->declBit(c+38,"dout_12", false,-1);
        tracep->declBit(c+39,"dout_6", false,-1);
        tracep->declBit(c+40,"dout_3", false,-1);
        tracep->declBit(c+41,"dout_1", false,-1);
        tracep->declBit(c+42,"dout_10", false,-1);
        tracep->declBit(c+43,"dout_0", false,-1);
        tracep->declBit(c+44,"dout_8", false,-1);
        tracep->declBit(c+45,"dout_11", false,-1);
        tracep->declBit(c+46,"dout_4", false,-1);
        tracep->declBit(c+47,"dout_5", false,-1);
        tracep->declBit(c+48,"dout_2", false,-1);
        tracep->declBit(c+49,"dout_9", false,-1);
        tracep->declBit(c+50,"dout_7", false,-1);
        tracep->declBit(c+51,"dout_13", false,-1);
        tracep->declBit(c+52,"dout_14", false,-1);
        tracep->declBit(c+53,"dout_15", false,-1);
        tracep->declBit(c+1,"bram_netlist clk", false,-1);
        tracep->declBit(c+2,"bram_netlist raddr_4", false,-1);
        tracep->declBit(c+3,"bram_netlist waddr_4", false,-1);
        tracep->declBit(c+4,"bram_netlist din_11", false,-1);
        tracep->declBit(c+5,"bram_netlist raddr_6", false,-1);
        tracep->declBit(c+6,"bram_netlist raddr_3", false,-1);
        tracep->declBit(c+7,"bram_netlist raddr_5", false,-1);
        tracep->declBit(c+8,"bram_netlist raddr_1", false,-1);
        tracep->declBit(c+9,"bram_netlist waddr_3", false,-1);
        tracep->declBit(c+10,"bram_netlist din_4", false,-1);
        tracep->declBit(c+11,"bram_netlist din_5", false,-1);
        tracep->declBit(c+12,"bram_netlist din_9", false,-1);
        tracep->declBit(c+13,"bram_netlist din_10", false,-1);
        tracep->declBit(c+14,"bram_netlist raddr_0", false,-1);
        tracep->declBit(c+15,"bram_netlist din_8", false,-1);
        tracep->declBit(c+16,"bram_netlist din_6", false,-1);
        tracep->declBit(c+17,"bram_netlist din_7", false,-1);
        tracep->declBit(c+18,"bram_netlist waddr_5", false,-1);
        tracep->declBit(c+19,"bram_netlist waddr_6", false,-1);
        tracep->declBit(c+20,"bram_netlist rclke", false,-1);
        tracep->declBit(c+21,"bram_netlist din_3", false,-1);
        tracep->declBit(c+22,"bram_netlist waddr_7", false,-1);
        tracep->declBit(c+23,"bram_netlist write_en", false,-1);
        tracep->declBit(c+24,"bram_netlist din_14", false,-1);
        tracep->declBit(c+25,"bram_netlist din_15", false,-1);
        tracep->declBit(c+26,"bram_netlist din_12", false,-1);
        tracep->declBit(c+27,"bram_netlist din_13", false,-1);
        tracep->declBit(c+28,"bram_netlist raddr_2", false,-1);
        tracep->declBit(c+29,"bram_netlist wclke", false,-1);
        tracep->declBit(c+30,"bram_netlist waddr_1", false,-1);
        tracep->declBit(c+31,"bram_netlist din_0", false,-1);
        tracep->declBit(c+32,"bram_netlist waddr_2", false,-1);
        tracep->declBit(c+33,"bram_netlist din_1", false,-1);
        tracep->declBit(c+34,"bram_netlist din_2", false,-1);
        tracep->declBit(c+35,"bram_netlist raddr_7", false,-1);
        tracep->declBit(c+36,"bram_netlist read_en", false,-1);
        tracep->declBit(c+37,"bram_netlist waddr_0", false,-1);
        tracep->declBit(c+38,"bram_netlist dout_12", false,-1);
        tracep->declBit(c+39,"bram_netlist dout_6", false,-1);
        tracep->declBit(c+40,"bram_netlist dout_3", false,-1);
        tracep->declBit(c+41,"bram_netlist dout_1", false,-1);
        tracep->declBit(c+42,"bram_netlist dout_10", false,-1);
        tracep->declBit(c+43,"bram_netlist dout_0", false,-1);
        tracep->declBit(c+44,"bram_netlist dout_8", false,-1);
        tracep->declBit(c+45,"bram_netlist dout_11", false,-1);
        tracep->declBit(c+46,"bram_netlist dout_4", false,-1);
        tracep->declBit(c+47,"bram_netlist dout_5", false,-1);
        tracep->declBit(c+48,"bram_netlist dout_2", false,-1);
        tracep->declBit(c+49,"bram_netlist dout_9", false,-1);
        tracep->declBit(c+50,"bram_netlist dout_7", false,-1);
        tracep->declBit(c+51,"bram_netlist dout_13", false,-1);
        tracep->declBit(c+52,"bram_netlist dout_14", false,-1);
        tracep->declBit(c+53,"bram_netlist dout_15", false,-1);
        tracep->declBit(c+1,"bram_netlist rclk", false,-1);
        tracep->declBit(c+1,"bram_netlist wclk", false,-1);
    }
}

void Vbram_netlist::traceRegister(VerilatedVcd* tracep) {
    // Body
    {
        tracep->addFullCb(&traceFullTop0, __VlSymsp);
        tracep->addChgCb(&traceChgTop0, __VlSymsp);
        tracep->addCleanupCb(&traceCleanup, __VlSymsp);
    }
}

void Vbram_netlist::traceFullTop0(void* userp, VerilatedVcd* tracep) {
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    {
        vlTOPp->traceFullSub0(userp, tracep);
    }
}

void Vbram_netlist::traceFullSub0(void* userp, VerilatedVcd* tracep) {
    Vbram_netlist__Syms* __restrict vlSymsp = static_cast<Vbram_netlist__Syms*>(userp);
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    vluint32_t* const oldp = tracep->oldp(vlSymsp->__Vm_baseCode);
    if (false && oldp) {}  // Prevent unused
    // Body
    {
        tracep->fullBit(oldp+1,(vlTOPp->clk));
        tracep->fullBit(oldp+2,(vlTOPp->raddr_4));
        tracep->fullBit(oldp+3,(vlTOPp->waddr_4));
        tracep->fullBit(oldp+4,(vlTOPp->din_11));
        tracep->fullBit(oldp+5,(vlTOPp->raddr_6));
        tracep->fullBit(oldp+6,(vlTOPp->raddr_3));
        tracep->fullBit(oldp+7,(vlTOPp->raddr_5));
        tracep->fullBit(oldp+8,(vlTOPp->raddr_1));
        tracep->fullBit(oldp+9,(vlTOPp->waddr_3));
        tracep->fullBit(oldp+10,(vlTOPp->din_4));
        tracep->fullBit(oldp+11,(vlTOPp->din_5));
        tracep->fullBit(oldp+12,(vlTOPp->din_9));
        tracep->fullBit(oldp+13,(vlTOPp->din_10));
        tracep->fullBit(oldp+14,(vlTOPp->raddr_0));
        tracep->fullBit(oldp+15,(vlTOPp->din_8));
        tracep->fullBit(oldp+16,(vlTOPp->din_6));
        tracep->fullBit(oldp+17,(vlTOPp->din_7));
        tracep->fullBit(oldp+18,(vlTOPp->waddr_5));
        tracep->fullBit(oldp+19,(vlTOPp->waddr_6));
        tracep->fullBit(oldp+20,(vlTOPp->rclke));
        tracep->fullBit(oldp+21,(vlTOPp->din_3));
        tracep->fullBit(oldp+22,(vlTOPp->waddr_7));
        tracep->fullBit(oldp+23,(vlTOPp->write_en));
        tracep->fullBit(oldp+24,(vlTOPp->din_14));
        tracep->fullBit(oldp+25,(vlTOPp->din_15));
        tracep->fullBit(oldp+26,(vlTOPp->din_12));
        tracep->fullBit(oldp+27,(vlTOPp->din_13));
        tracep->fullBit(oldp+28,(vlTOPp->raddr_2));
        tracep->fullBit(oldp+29,(vlTOPp->wclke));
        tracep->fullBit(oldp+30,(vlTOPp->waddr_1));
        tracep->fullBit(oldp+31,(vlTOPp->din_0));
        tracep->fullBit(oldp+32,(vlTOPp->waddr_2));
        tracep->fullBit(oldp+33,(vlTOPp->din_1));
        tracep->fullBit(oldp+34,(vlTOPp->din_2));
        tracep->fullBit(oldp+35,(vlTOPp->raddr_7));
        tracep->fullBit(oldp+36,(vlTOPp->read_en));
        tracep->fullBit(oldp+37,(vlTOPp->waddr_0));
        tracep->fullBit(oldp+38,(vlTOPp->dout_12));
        tracep->fullBit(oldp+39,(vlTOPp->dout_6));
        tracep->fullBit(oldp+40,(vlTOPp->dout_3));
        tracep->fullBit(oldp+41,(vlTOPp->dout_1));
        tracep->fullBit(oldp+42,(vlTOPp->dout_10));
        tracep->fullBit(oldp+43,(vlTOPp->dout_0));
        tracep->fullBit(oldp+44,(vlTOPp->dout_8));
        tracep->fullBit(oldp+45,(vlTOPp->dout_11));
        tracep->fullBit(oldp+46,(vlTOPp->dout_4));
        tracep->fullBit(oldp+47,(vlTOPp->dout_5));
        tracep->fullBit(oldp+48,(vlTOPp->dout_2));
        tracep->fullBit(oldp+49,(vlTOPp->dout_9));
        tracep->fullBit(oldp+50,(vlTOPp->dout_7));
        tracep->fullBit(oldp+51,(vlTOPp->dout_13));
        tracep->fullBit(oldp+52,(vlTOPp->dout_14));
        tracep->fullBit(oldp+53,(vlTOPp->dout_15));
    }
}
