// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vbram_netlist.h for the primary calling header

#include "Vbram_netlist.h"
#include "Vbram_netlist__Syms.h"

//==========

Vbram_netlist::Vbram_netlist(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModule{_vcname__}
 {
    Vbram_netlist__Syms* __restrict vlSymsp = __VlSymsp = new Vbram_netlist__Syms(_vcontextp__, this, name());
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Reset internal values
    
    // Reset structure values
    _ctor_var_reset();
}

void Vbram_netlist::__Vconfigure(Vbram_netlist__Syms* vlSymsp, bool first) {
    if (false && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
    if (false && this->__VlSymsp) {}  // Prevent unused
    vlSymsp->_vm_contextp__->timeunit(-12);
    vlSymsp->_vm_contextp__->timeprecision(-12);
}

Vbram_netlist::~Vbram_netlist() {
    VL_DO_CLEAR(delete __VlSymsp, __VlSymsp = nullptr);
}

void Vbram_netlist::_initial__TOP__1(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_initial__TOP__1\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Variables
    IData/*31:0*/ __Vilp;
    // Body
    vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
        = (0xffffU & VL_RANDOM_I(16));
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK = 0xc8ULL;
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK = 0x640ULL;
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Time_Collision_Detected = 0U;
    __Vilp = 0U;
    while ((__Vilp <= 0xfffU)) {
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vilp] = 0U;
        __Vilp = ((IData)(1U) + __Vilp);
    }
}

void Vbram_netlist::_settle__TOP__3(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_settle__TOP__3\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    if ((1U & (~ (IData)(vlTOPp->clk)))) {
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLKE_sync 
            = vlTOPp->rclke;
    }
    if ((1U & (~ (IData)(vlTOPp->clk)))) {
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLKE_sync 
            = vlTOPp->wclke;
    }
    vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA 
        = (((IData)(vlTOPp->din_15) << 0xfU) | (((IData)(vlTOPp->din_14) 
                                                 << 0xeU) 
                                                | (((IData)(vlTOPp->din_13) 
                                                    << 0xdU) 
                                                   | (((IData)(vlTOPp->din_12) 
                                                       << 0xcU) 
                                                      | (((IData)(vlTOPp->din_11) 
                                                          << 0xbU) 
                                                         | (((IData)(vlTOPp->din_10) 
                                                             << 0xaU) 
                                                            | (((IData)(vlTOPp->din_9) 
                                                                << 9U) 
                                                               | (((IData)(vlTOPp->din_8) 
                                                                   << 8U) 
                                                                  | (((IData)(vlTOPp->din_7) 
                                                                      << 7U) 
                                                                     | (((IData)(vlTOPp->din_6) 
                                                                         << 6U) 
                                                                        | (((IData)(vlTOPp->din_5) 
                                                                            << 5U) 
                                                                           | (((IData)(vlTOPp->din_4) 
                                                                               << 4U) 
                                                                              | (((IData)(vlTOPp->din_3) 
                                                                                << 3U) 
                                                                                | (((IData)(vlTOPp->din_2) 
                                                                                << 2U) 
                                                                                | (((IData)(vlTOPp->din_1) 
                                                                                << 1U) 
                                                                                | (IData)(vlTOPp->din_0))))))))))))))));
    vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR 
        = (((IData)(vlTOPp->raddr_7) << 7U) | (((IData)(vlTOPp->raddr_6) 
                                                << 6U) 
                                               | (((IData)(vlTOPp->raddr_5) 
                                                   << 5U) 
                                                  | (((IData)(vlTOPp->raddr_4) 
                                                      << 4U) 
                                                     | (((IData)(vlTOPp->raddr_3) 
                                                         << 3U) 
                                                        | (((IData)(vlTOPp->raddr_2) 
                                                            << 2U) 
                                                           | (((IData)(vlTOPp->raddr_1) 
                                                               << 1U) 
                                                              | (IData)(vlTOPp->raddr_0))))))));
    vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR 
        = (((IData)(vlTOPp->waddr_7) << 7U) | (((IData)(vlTOPp->waddr_6) 
                                                << 6U) 
                                               | (((IData)(vlTOPp->waddr_5) 
                                                   << 5U) 
                                                  | (((IData)(vlTOPp->waddr_4) 
                                                      << 4U) 
                                                     | (((IData)(vlTOPp->waddr_3) 
                                                         << 3U) 
                                                        | (((IData)(vlTOPp->waddr_2) 
                                                            << 2U) 
                                                           | (((IData)(vlTOPp->waddr_1) 
                                                               << 1U) 
                                                              | (IData)(vlTOPp->waddr_0))))))));
    vlTOPp->dout_15 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                             >> 0xfU));
    vlTOPp->dout_14 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                             >> 0xeU));
    vlTOPp->dout_13 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                             >> 0xdU));
    vlTOPp->dout_12 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                             >> 0xcU));
    vlTOPp->dout_11 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                             >> 0xbU));
    vlTOPp->dout_10 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                             >> 0xaU));
    vlTOPp->dout_9 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 9U));
    vlTOPp->dout_8 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 8U));
    vlTOPp->dout_7 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 7U));
    vlTOPp->dout_6 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 6U));
    vlTOPp->dout_5 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 5U));
    vlTOPp->dout_4 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 4U));
    vlTOPp->dout_3 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 3U));
    vlTOPp->dout_2 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 2U));
    vlTOPp->dout_1 = (1U & ((IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA) 
                            >> 1U));
    vlTOPp->dout_0 = (1U & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA));
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g 
        = ((IData)(vlTOPp->clk) & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLKE_sync));
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g 
        = ((IData)(vlTOPp->clk) & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLKE_sync));
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Address_Collision_Detected 
        = (((((IData)(vlTOPp->read_en) & (IData)(vlTOPp->write_en)) 
             & (IData)(vlTOPp->wclke)) & (IData)(vlTOPp->rclke)) 
           & ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
              == (IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR)));
}

void Vbram_netlist::_eval_initial(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_eval_initial\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->_initial__TOP__1(vlSymsp);
    vlTOPp->__Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g 
        = vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g;
    vlTOPp->__Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g 
        = vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g;
}

void Vbram_netlist::final() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::final\n"); );
    // Variables
    Vbram_netlist__Syms* __restrict vlSymsp = this->__VlSymsp;
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
}

void Vbram_netlist::_eval_settle(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_eval_settle\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->_settle__TOP__3(vlSymsp);
}

void Vbram_netlist::_ctor_var_reset() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_ctor_var_reset\n"); );
    // Body
    clk = VL_RAND_RESET_I(1);
    raddr_4 = VL_RAND_RESET_I(1);
    waddr_4 = VL_RAND_RESET_I(1);
    din_11 = VL_RAND_RESET_I(1);
    raddr_6 = VL_RAND_RESET_I(1);
    raddr_3 = VL_RAND_RESET_I(1);
    raddr_5 = VL_RAND_RESET_I(1);
    raddr_1 = VL_RAND_RESET_I(1);
    waddr_3 = VL_RAND_RESET_I(1);
    din_4 = VL_RAND_RESET_I(1);
    din_5 = VL_RAND_RESET_I(1);
    din_9 = VL_RAND_RESET_I(1);
    din_10 = VL_RAND_RESET_I(1);
    raddr_0 = VL_RAND_RESET_I(1);
    din_8 = VL_RAND_RESET_I(1);
    din_6 = VL_RAND_RESET_I(1);
    din_7 = VL_RAND_RESET_I(1);
    waddr_5 = VL_RAND_RESET_I(1);
    waddr_6 = VL_RAND_RESET_I(1);
    rclke = VL_RAND_RESET_I(1);
    din_3 = VL_RAND_RESET_I(1);
    waddr_7 = VL_RAND_RESET_I(1);
    write_en = VL_RAND_RESET_I(1);
    din_14 = VL_RAND_RESET_I(1);
    din_15 = VL_RAND_RESET_I(1);
    din_12 = VL_RAND_RESET_I(1);
    din_13 = VL_RAND_RESET_I(1);
    raddr_2 = VL_RAND_RESET_I(1);
    wclke = VL_RAND_RESET_I(1);
    waddr_1 = VL_RAND_RESET_I(1);
    din_0 = VL_RAND_RESET_I(1);
    waddr_2 = VL_RAND_RESET_I(1);
    din_1 = VL_RAND_RESET_I(1);
    din_2 = VL_RAND_RESET_I(1);
    raddr_7 = VL_RAND_RESET_I(1);
    read_en = VL_RAND_RESET_I(1);
    waddr_0 = VL_RAND_RESET_I(1);
    dout_12 = VL_RAND_RESET_I(1);
    dout_6 = VL_RAND_RESET_I(1);
    dout_3 = VL_RAND_RESET_I(1);
    dout_1 = VL_RAND_RESET_I(1);
    dout_10 = VL_RAND_RESET_I(1);
    dout_0 = VL_RAND_RESET_I(1);
    dout_8 = VL_RAND_RESET_I(1);
    dout_11 = VL_RAND_RESET_I(1);
    dout_4 = VL_RAND_RESET_I(1);
    dout_5 = VL_RAND_RESET_I(1);
    dout_2 = VL_RAND_RESET_I(1);
    dout_9 = VL_RAND_RESET_I(1);
    dout_7 = VL_RAND_RESET_I(1);
    dout_13 = VL_RAND_RESET_I(1);
    dout_14 = VL_RAND_RESET_I(1);
    dout_15 = VL_RAND_RESET_I(1);
    bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA = VL_RAND_RESET_I(16);
    bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA = VL_RAND_RESET_I(16);
    bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR = VL_RAND_RESET_I(8);
    bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR = VL_RAND_RESET_I(8);
    bram_netlist__DOT__ram256x16_inst__DOT__MASK = VL_RAND_RESET_I(16);
    bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g = VL_RAND_RESET_I(1);
    bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g = VL_RAND_RESET_I(1);
    bram_netlist__DOT__ram256x16_inst__DOT__WCLKE_sync = VL_RAND_RESET_I(1);
    bram_netlist__DOT__ram256x16_inst__DOT__RCLKE_sync = VL_RAND_RESET_I(1);
    for (int __Vi0=0; __Vi0<4096; ++__Vi0) {
        bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vi0] = VL_RAND_RESET_I(1);
    }
    bram_netlist__DOT__ram256x16_inst__DOT__Time_Collision_Detected = VL_RAND_RESET_I(1);
    bram_netlist__DOT__ram256x16_inst__DOT__Address_Collision_Detected = VL_RAND_RESET_I(1);
    bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK = VL_RAND_RESET_Q(64);
    bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK = VL_RAND_RESET_Q(64);
    for (int __Vi0=0; __Vi0<1; ++__Vi0) {
        __Vm_traceActivity[__Vi0] = VL_RAND_RESET_I(1);
    }
}
