// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vbram_netlist.h for the primary calling header

#include "Vbram_netlist.h"
#include "Vbram_netlist__Syms.h"

//==========

VerilatedContext* Vbram_netlist::contextp() {
    return __VlSymsp->_vm_contextp__;
}

void Vbram_netlist::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vbram_netlist::eval\n"); );
    Vbram_netlist__Syms* __restrict vlSymsp = this->__VlSymsp;  // Setup global symbol table
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
#ifdef VL_DEBUG
    // Debug assertions
    _eval_debug_assertions();
#endif  // VL_DEBUG
    // Initialize
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) _eval_initial_loop(vlSymsp);
    // Evaluate till stable
    int __VclockLoop = 0;
    QData __Vchange = 1;
    do {
        VL_DEBUG_IF(VL_DBG_MSGF("+ Clock loop\n"););
        vlSymsp->__Vm_activity = true;
        _eval(vlSymsp);
        if (VL_UNLIKELY(++__VclockLoop > 100)) {
            // About to fail, so enable debug to see what's not settling.
            // Note you must run make with OPT=-DVL_DEBUG for debug prints.
            int __Vsaved_debug = Verilated::debug();
            Verilated::debug(1);
            __Vchange = _change_request(vlSymsp);
            Verilated::debug(__Vsaved_debug);
            VL_FATAL_MT("bram_netlist.v", 3, "",
                "Verilated model didn't converge\n"
                "- See DIDNOTCONVERGE in the Verilator manual");
        } else {
            __Vchange = _change_request(vlSymsp);
        }
    } while (VL_UNLIKELY(__Vchange));
}

void Vbram_netlist::_eval_initial_loop(Vbram_netlist__Syms* __restrict vlSymsp) {
    vlSymsp->__Vm_didInit = true;
    _eval_initial(vlSymsp);
    vlSymsp->__Vm_activity = true;
    // Evaluate till stable
    int __VclockLoop = 0;
    QData __Vchange = 1;
    do {
        _eval_settle(vlSymsp);
        _eval(vlSymsp);
        if (VL_UNLIKELY(++__VclockLoop > 100)) {
            // About to fail, so enable debug to see what's not settling.
            // Note you must run make with OPT=-DVL_DEBUG for debug prints.
            int __Vsaved_debug = Verilated::debug();
            Verilated::debug(1);
            __Vchange = _change_request(vlSymsp);
            Verilated::debug(__Vsaved_debug);
            VL_FATAL_MT("bram_netlist.v", 3, "",
                "Verilated model didn't DC converge\n"
                "- See DIDNOTCONVERGE in the Verilator manual");
        } else {
            __Vchange = _change_request(vlSymsp);
        }
    } while (VL_UNLIKELY(__Vchange));
}

VL_INLINE_OPT void Vbram_netlist::_combo__TOP__2(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_combo__TOP__2\n"); );
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
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g 
        = ((IData)(vlTOPp->clk) & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLKE_sync));
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g 
        = ((IData)(vlTOPp->clk) & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLKE_sync));
}

VL_INLINE_OPT void Vbram_netlist::_sequent__TOP__4(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_sequent__TOP__4\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK 
        = VL_TIME_UNITED_Q(1);
    if (vlTOPp->read_en) {
        if (((IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Time_Collision_Detected) 
             & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Address_Collision_Detected))) {
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA = 0U;
        } else {
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xfffeU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                   [(0xfffU & ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                               << 4U))]);
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xfffdU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(1U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 1U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xfffbU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(2U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 2U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xfff7U & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(3U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 3U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xffefU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(4U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 4U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xffdfU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(5U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 5U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xffbfU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(6U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 6U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xff7fU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(7U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 7U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xfeffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(8U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 8U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xfdffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(9U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                 << 4U)))] 
                      << 9U));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xfbffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(0xaU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                   << 4U)))] 
                      << 0xaU));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xf7ffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(0xbU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                   << 4U)))] 
                      << 0xbU));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xefffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(0xcU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                   << 4U)))] 
                      << 0xcU));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xdfffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(0xdU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                   << 4U)))] 
                      << 0xdU));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0xbfffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(0xeU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                   << 4U)))] 
                      << 0xeU));
            vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA 
                = ((0x7fffU & (IData)(vlTOPp->bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA)) 
                   | (vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                      [(0xfffU & ((IData)(0xfU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR) 
                                                   << 4U)))] 
                      << 0xfU));
        }
    }
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
}

VL_INLINE_OPT void Vbram_netlist::_sequent__TOP__5(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_sequent__TOP__5\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Variables
    CData/*0:0*/ __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__Vfuncout;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0;
    CData/*0:0*/ __Vdlyvset__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v1;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v2;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v3;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v4;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v5;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v6;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v7;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v8;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v9;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v10;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v11;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v12;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v13;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v14;
    CData/*0:0*/ __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v15;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v1;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v2;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v3;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v4;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v5;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v6;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v7;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v8;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v9;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v10;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v11;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v12;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v13;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v14;
    SData/*11:0*/ __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v15;
    QData/*63:0*/ __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__T1;
    QData/*63:0*/ __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__T2;
    QData/*63:0*/ __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__Difference;
    // Body
    __Vdlyvset__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0 = 0U;
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK 
        = VL_TIME_UNITED_Q(1);
    __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__T2 
        = vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK;
    __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__T1 
        = vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK;
    __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__Difference 
        = (__Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__T1 
           - __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__T2);
    __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__Vfuncout 
        = (0x19ULL > __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__Difference);
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Time_Collision_Detected 
        = __Vfunc_bram_netlist__DOT__ram256x16_inst__DOT__Check_Timed_Window_Violation__0__Vfuncout;
    if (VL_UNLIKELY(((IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Time_Collision_Detected) 
                     & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Address_Collision_Detected)))) {
        VL_WRITEF("Warning: Write-Read collision detected, Data read value is XXXX\n\nWCLK Time: %.3f   RCLK Time:%.3f  WADDR: %3#   RADDR:%3#\n\n",
                  64,VL_ITOR_D_Q(64, vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK),
                  64,VL_ITOR_D_Q(64, vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK),
                  8,(IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR),
                  8,vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR);
        VL_FWRITEF(0x2e747874U,"Warning: Write-Read collision detected, Data read value is XXXX\n\nWCLK Time: %.3f   RCLK Time:%.3f  WADDR: %3#   RADDR:%3#\n\n",
                   64,VL_ITOR_D_Q(64, vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK),
                   64,VL_ITOR_D_Q(64, vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK),
                   8,(IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR),
                   8,vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR);
    }
    if (vlTOPp->write_en) {
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0 
            = (1U & ((1U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                 << 4U))] : (IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA)));
        __Vdlyvset__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0 = 1U;
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0 
            = (0xfffU & ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                         << 4U));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v1 
            = (1U & ((2U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(1U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 1U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v1 
            = (0xfffU & ((IData)(1U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v2 
            = (1U & ((4U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(2U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 2U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v2 
            = (0xfffU & ((IData)(2U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v3 
            = (1U & ((8U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(3U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 3U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v3 
            = (0xfffU & ((IData)(3U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v4 
            = (1U & ((0x10U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(4U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 4U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v4 
            = (0xfffU & ((IData)(4U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v5 
            = (1U & ((0x20U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(5U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 5U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v5 
            = (0xfffU & ((IData)(5U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v6 
            = (1U & ((0x40U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(6U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 6U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v6 
            = (0xfffU & ((IData)(6U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v7 
            = (1U & ((0x80U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(7U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 7U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v7 
            = (0xfffU & ((IData)(7U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v8 
            = (1U & ((0x100U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(8U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 8U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v8 
            = (0xfffU & ((IData)(8U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v9 
            = (1U & ((0x200U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(9U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 9U)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v9 
            = (0xfffU & ((IData)(9U) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                        << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v10 
            = (1U & ((0x400U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(0xaU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                  << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 0xaU)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v10 
            = (0xfffU & ((IData)(0xaU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                          << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v11 
            = (1U & ((0x800U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(0xbU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                  << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 0xbU)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v11 
            = (0xfffU & ((IData)(0xbU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                          << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v12 
            = (1U & ((0x1000U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(0xcU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                  << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 0xcU)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v12 
            = (0xfffU & ((IData)(0xcU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                          << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v13 
            = (1U & ((0x2000U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(0xdU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                  << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 0xdU)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v13 
            = (0xfffU & ((IData)(0xdU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                          << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v14 
            = (1U & ((0x4000U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(0xeU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                  << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 0xeU)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v14 
            = (0xfffU & ((IData)(0xeU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                          << 4U)));
        __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v15 
            = (1U & ((0x8000U & (IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__MASK))
                      ? vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory
                     [(0xfffU & ((IData)(0xfU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                                  << 4U)))]
                      : ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA) 
                         >> 0xfU)));
        __Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v15 
            = (0xfffU & ((IData)(0xfU) + ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
                                          << 4U)));
    }
    if (__Vdlyvset__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0) {
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v0;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v1] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v1;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v2] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v2;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v3] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v3;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v4] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v4;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v5] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v5;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v6] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v6;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v7] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v7;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v8] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v8;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v9] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v9;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v10] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v10;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v11] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v11;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v12] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v12;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v13] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v13;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v14] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v14;
        vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Memory[__Vdlyvdim0__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v15] 
            = __Vdlyvval__bram_netlist__DOT__ram256x16_inst__DOT__Memory__v15;
    }
}

VL_INLINE_OPT void Vbram_netlist::_combo__TOP__6(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_combo__TOP__6\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
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
    vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__Address_Collision_Detected 
        = (((((IData)(vlTOPp->read_en) & (IData)(vlTOPp->write_en)) 
             & (IData)(vlTOPp->wclke)) & (IData)(vlTOPp->rclke)) 
           & ((IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR) 
              == (IData)(vlTOPp->bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR)));
}

void Vbram_netlist::_eval(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_eval\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->_combo__TOP__2(vlSymsp);
    if (((IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g) 
         & (~ (IData)(vlTOPp->__Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g)))) {
        vlTOPp->_sequent__TOP__4(vlSymsp);
    }
    if (((IData)(vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g) 
         & (~ (IData)(vlTOPp->__Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g)))) {
        vlTOPp->_sequent__TOP__5(vlSymsp);
    }
    vlTOPp->_combo__TOP__6(vlSymsp);
    // Final
    vlTOPp->__Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g 
        = vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g;
    vlTOPp->__Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g 
        = vlTOPp->bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g;
}

VL_INLINE_OPT QData Vbram_netlist::_change_request(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_change_request\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    return (vlTOPp->_change_request_1(vlSymsp));
}

VL_INLINE_OPT QData Vbram_netlist::_change_request_1(Vbram_netlist__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_change_request_1\n"); );
    Vbram_netlist* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // Change detection
    QData __req = false;  // Logically a bool
    return __req;
}

#ifdef VL_DEBUG
void Vbram_netlist::_eval_debug_assertions() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vbram_netlist::_eval_debug_assertions\n"); );
    // Body
    if (VL_UNLIKELY((clk & 0xfeU))) {
        Verilated::overWidthError("clk");}
    if (VL_UNLIKELY((raddr_4 & 0xfeU))) {
        Verilated::overWidthError("raddr_4");}
    if (VL_UNLIKELY((waddr_4 & 0xfeU))) {
        Verilated::overWidthError("waddr_4");}
    if (VL_UNLIKELY((din_11 & 0xfeU))) {
        Verilated::overWidthError("din_11");}
    if (VL_UNLIKELY((raddr_6 & 0xfeU))) {
        Verilated::overWidthError("raddr_6");}
    if (VL_UNLIKELY((raddr_3 & 0xfeU))) {
        Verilated::overWidthError("raddr_3");}
    if (VL_UNLIKELY((raddr_5 & 0xfeU))) {
        Verilated::overWidthError("raddr_5");}
    if (VL_UNLIKELY((raddr_1 & 0xfeU))) {
        Verilated::overWidthError("raddr_1");}
    if (VL_UNLIKELY((waddr_3 & 0xfeU))) {
        Verilated::overWidthError("waddr_3");}
    if (VL_UNLIKELY((din_4 & 0xfeU))) {
        Verilated::overWidthError("din_4");}
    if (VL_UNLIKELY((din_5 & 0xfeU))) {
        Verilated::overWidthError("din_5");}
    if (VL_UNLIKELY((din_9 & 0xfeU))) {
        Verilated::overWidthError("din_9");}
    if (VL_UNLIKELY((din_10 & 0xfeU))) {
        Verilated::overWidthError("din_10");}
    if (VL_UNLIKELY((raddr_0 & 0xfeU))) {
        Verilated::overWidthError("raddr_0");}
    if (VL_UNLIKELY((din_8 & 0xfeU))) {
        Verilated::overWidthError("din_8");}
    if (VL_UNLIKELY((din_6 & 0xfeU))) {
        Verilated::overWidthError("din_6");}
    if (VL_UNLIKELY((din_7 & 0xfeU))) {
        Verilated::overWidthError("din_7");}
    if (VL_UNLIKELY((waddr_5 & 0xfeU))) {
        Verilated::overWidthError("waddr_5");}
    if (VL_UNLIKELY((waddr_6 & 0xfeU))) {
        Verilated::overWidthError("waddr_6");}
    if (VL_UNLIKELY((rclke & 0xfeU))) {
        Verilated::overWidthError("rclke");}
    if (VL_UNLIKELY((din_3 & 0xfeU))) {
        Verilated::overWidthError("din_3");}
    if (VL_UNLIKELY((waddr_7 & 0xfeU))) {
        Verilated::overWidthError("waddr_7");}
    if (VL_UNLIKELY((write_en & 0xfeU))) {
        Verilated::overWidthError("write_en");}
    if (VL_UNLIKELY((din_14 & 0xfeU))) {
        Verilated::overWidthError("din_14");}
    if (VL_UNLIKELY((din_15 & 0xfeU))) {
        Verilated::overWidthError("din_15");}
    if (VL_UNLIKELY((din_12 & 0xfeU))) {
        Verilated::overWidthError("din_12");}
    if (VL_UNLIKELY((din_13 & 0xfeU))) {
        Verilated::overWidthError("din_13");}
    if (VL_UNLIKELY((raddr_2 & 0xfeU))) {
        Verilated::overWidthError("raddr_2");}
    if (VL_UNLIKELY((wclke & 0xfeU))) {
        Verilated::overWidthError("wclke");}
    if (VL_UNLIKELY((waddr_1 & 0xfeU))) {
        Verilated::overWidthError("waddr_1");}
    if (VL_UNLIKELY((din_0 & 0xfeU))) {
        Verilated::overWidthError("din_0");}
    if (VL_UNLIKELY((waddr_2 & 0xfeU))) {
        Verilated::overWidthError("waddr_2");}
    if (VL_UNLIKELY((din_1 & 0xfeU))) {
        Verilated::overWidthError("din_1");}
    if (VL_UNLIKELY((din_2 & 0xfeU))) {
        Verilated::overWidthError("din_2");}
    if (VL_UNLIKELY((raddr_7 & 0xfeU))) {
        Verilated::overWidthError("raddr_7");}
    if (VL_UNLIKELY((read_en & 0xfeU))) {
        Verilated::overWidthError("read_en");}
    if (VL_UNLIKELY((waddr_0 & 0xfeU))) {
        Verilated::overWidthError("waddr_0");}
}
#endif  // VL_DEBUG
