// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals
#include "verilated_vcd_c.h"
#include "Vbram_netlist__Syms.h"


//======================

void Vbram_netlist::traceChg(VerilatedVcd* vcdp, void* userthis, uint32_t code) {
    // Callback from vcd->dump()
    Vbram_netlist* t = (Vbram_netlist*)userthis;
    Vbram_netlist__Syms* __restrict vlSymsp = t->__VlSymsp;  // Setup global symbol table
    if (vlSymsp->getClearActivity()) {
        t->traceChgThis(vlSymsp, vcdp, code);
    }
}

//======================


void Vbram_netlist::traceChgThis(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vbram_netlist* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        if (VL_UNLIKELY((1U & vlTOPp->__Vm_traceActivity))) {
            vlTOPp->traceChgThis__2(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((1U & (vlTOPp->__Vm_traceActivity 
                               | (vlTOPp->__Vm_traceActivity 
                                  >> 1U))))) {
            vlTOPp->traceChgThis__3(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((1U & (vlTOPp->__Vm_traceActivity 
                               | (vlTOPp->__Vm_traceActivity 
                                  >> 2U))))) {
            vlTOPp->traceChgThis__4(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((1U & (vlTOPp->__Vm_traceActivity 
                               | (vlTOPp->__Vm_traceActivity 
                                  >> 3U))))) {
            vlTOPp->traceChgThis__5(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((1U & ((vlTOPp->__Vm_traceActivity 
                                | (vlTOPp->__Vm_traceActivity 
                                   >> 3U)) | (vlTOPp->__Vm_traceActivity 
                                              >> 4U))))) {
            vlTOPp->traceChgThis__6(vlSymsp, vcdp, code);
        }
        vlTOPp->traceChgThis__7(vlSymsp, vcdp, code);
    }
    // Final
    vlTOPp->__Vm_traceActivity = 0U;
}

void Vbram_netlist::traceChgThis__2(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vbram_netlist* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBus(c+1,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__j),32);
    }
}

void Vbram_netlist::traceChgThis__3(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vbram_netlist* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBus(c+9,(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR),8);
        vcdp->chgBus(c+17,(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR),8);
        vcdp->chgBus(c+25,(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__MASK),16);
        vcdp->chgBus(c+33,(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA),16);
        vcdp->chgBit(c+41,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g));
        vcdp->chgBit(c+49,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g));
        vcdp->chgBit(c+57,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLKE_sync));
        vcdp->chgBit(c+65,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLKE_sync));
        vcdp->chgBit(c+73,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Address_Collision_Detected));
    }
}

void Vbram_netlist::traceChgThis__4(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vbram_netlist* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgQuad(c+81,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK),64);
    }
}

void Vbram_netlist::traceChgThis__5(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vbram_netlist* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgQuad(c+97,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK),64);
    }
}

void Vbram_netlist::traceChgThis__6(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vbram_netlist* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBus(c+113,(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA),16);
        vcdp->chgBit(c+121,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Time_Collision_Detected));
        vcdp->chgBus(c+129,(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__i),32);
    }
}

void Vbram_netlist::traceChgThis__7(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vbram_netlist* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBit(c+137,(vlTOPp->mask_1));
        vcdp->chgBit(c+145,(vlTOPp->mask_2));
        vcdp->chgBit(c+153,(vlTOPp->mask_3));
        vcdp->chgBit(c+161,(vlTOPp->mask_4));
        vcdp->chgBit(c+169,(vlTOPp->mask_5));
        vcdp->chgBit(c+177,(vlTOPp->mask_8));
        vcdp->chgBit(c+185,(vlTOPp->mask_9));
        vcdp->chgBit(c+193,(vlTOPp->mask_11));
        vcdp->chgBit(c+201,(vlTOPp->mask_13));
        vcdp->chgBit(c+209,(vlTOPp->waddr_0));
        vcdp->chgBit(c+217,(vlTOPp->read_en));
        vcdp->chgBit(c+225,(vlTOPp->raddr_7));
        vcdp->chgBit(c+233,(vlTOPp->din_2));
        vcdp->chgBit(c+241,(vlTOPp->din_1));
        vcdp->chgBit(c+249,(vlTOPp->waddr_2));
        vcdp->chgBit(c+257,(vlTOPp->din_12));
        vcdp->chgBit(c+265,(vlTOPp->waddr_1));
        vcdp->chgBit(c+273,(vlTOPp->mask_10));
        vcdp->chgBit(c+281,(vlTOPp->waddr_3));
        vcdp->chgBit(c+289,(vlTOPp->din_7));
        vcdp->chgBit(c+297,(vlTOPp->din_5));
        vcdp->chgBit(c+305,(vlTOPp->mask_12));
        vcdp->chgBit(c+313,(vlTOPp->din_14));
        vcdp->chgBit(c+321,(vlTOPp->raddr_5));
        vcdp->chgBit(c+329,(vlTOPp->waddr_4));
        vcdp->chgBit(c+337,(vlTOPp->mask_15));
        vcdp->chgBit(c+345,(vlTOPp->waddr_5));
        vcdp->chgBit(c+353,(vlTOPp->waddr_6));
        vcdp->chgBit(c+361,(vlTOPp->rclke));
        vcdp->chgBit(c+369,(vlTOPp->din_15));
        vcdp->chgBit(c+377,(vlTOPp->din_13));
        vcdp->chgBit(c+385,(vlTOPp->write_en));
        vcdp->chgBit(c+393,(vlTOPp->din_10));
        vcdp->chgBit(c+401,(vlTOPp->mask_6));
        vcdp->chgBit(c+409,(vlTOPp->mask_0));
        vcdp->chgBit(c+417,(vlTOPp->din_8));
        vcdp->chgBit(c+425,(vlTOPp->din_3));
        vcdp->chgBit(c+433,(vlTOPp->waddr_7));
        vcdp->chgBit(c+441,(vlTOPp->raddr_2));
        vcdp->chgBit(c+449,(vlTOPp->mask_14));
        vcdp->chgBit(c+457,(vlTOPp->wclke));
        vcdp->chgBit(c+465,(vlTOPp->din_6));
        vcdp->chgBit(c+473,(vlTOPp->mask_7));
        vcdp->chgBit(c+481,(vlTOPp->raddr_0));
        vcdp->chgBit(c+489,(vlTOPp->clk));
        vcdp->chgBit(c+497,(vlTOPp->raddr_1));
        vcdp->chgBit(c+505,(vlTOPp->raddr_6));
        vcdp->chgBit(c+513,(vlTOPp->raddr_4));
        vcdp->chgBit(c+521,(vlTOPp->din_9));
        vcdp->chgBit(c+529,(vlTOPp->din_0));
        vcdp->chgBit(c+537,(vlTOPp->din_11));
        vcdp->chgBit(c+545,(vlTOPp->raddr_3));
        vcdp->chgBit(c+553,(vlTOPp->din_4));
        vcdp->chgBit(c+561,(vlTOPp->dout_6));
        vcdp->chgBit(c+569,(vlTOPp->dout_15));
        vcdp->chgBit(c+577,(vlTOPp->dout_5));
        vcdp->chgBit(c+585,(vlTOPp->dout_0));
        vcdp->chgBit(c+593,(vlTOPp->dout_13));
        vcdp->chgBit(c+601,(vlTOPp->dout_11));
        vcdp->chgBit(c+609,(vlTOPp->dout_2));
        vcdp->chgBit(c+617,(vlTOPp->dout_12));
        vcdp->chgBit(c+625,(vlTOPp->dout_4));
        vcdp->chgBit(c+633,(vlTOPp->dout_8));
        vcdp->chgBit(c+641,(vlTOPp->dout_3));
        vcdp->chgBit(c+649,(vlTOPp->dout_7));
        vcdp->chgBit(c+657,(vlTOPp->dout_9));
        vcdp->chgBit(c+665,(vlTOPp->dout_14));
        vcdp->chgBit(c+673,(vlTOPp->dout_10));
        vcdp->chgBit(c+681,(vlTOPp->dout_1));
    }
}
