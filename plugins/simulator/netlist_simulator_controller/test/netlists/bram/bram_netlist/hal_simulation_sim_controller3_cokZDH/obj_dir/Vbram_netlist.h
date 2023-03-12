// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Primary design header
//
// This header should be included by all source files instantiating the design.
// The class here is then constructed to instantiate the design.
// See the Verilator manual for examples.

#ifndef _VBRAM_NETLIST_H_
#define _VBRAM_NETLIST_H_  // guard

#include "verilated_heavy.h"

//==========

class Vbram_netlist__Syms;
class Vbram_netlist_VerilatedVcd;


//----------

VL_MODULE(Vbram_netlist) {
  public:
    
    // PORTS
    // The application code writes and reads these signals to
    // propagate new values into/out from the Verilated model.
    VL_IN8(raddr_4,0,0);
    VL_IN8(mask_6,0,0);
    VL_IN8(din_1,0,0);
    VL_IN8(mask_2,0,0);
    VL_IN8(mask_8,0,0);
    VL_IN8(waddr_0,0,0);
    VL_IN8(read_en,0,0);
    VL_IN8(raddr_7,0,0);
    VL_IN8(waddr_2,0,0);
    VL_IN8(mask_13,0,0);
    VL_IN8(waddr_1,0,0);
    VL_IN8(waddr_3,0,0);
    VL_IN8(mask_9,0,0);
    VL_IN8(mask_10,0,0);
    VL_IN8(din_15,0,0);
    VL_IN8(din_9,0,0);
    VL_IN8(din_7,0,0);
    VL_IN8(din_0,0,0);
    VL_IN8(raddr_5,0,0);
    VL_IN8(wclke,0,0);
    VL_IN8(mask_4,0,0);
    VL_IN8(mask_7,0,0);
    VL_IN8(waddr_5,0,0);
    VL_IN8(raddr_6,0,0);
    VL_IN8(waddr_7,0,0);
    VL_IN8(mask_15,0,0);
    VL_IN8(din_3,0,0);
    VL_IN8(mask_5,0,0);
    VL_IN8(clk,0,0);
    VL_IN8(din_8,0,0);
    VL_IN8(raddr_0,0,0);
    VL_IN8(din_6,0,0);
    VL_IN8(din_13,0,0);
    VL_IN8(mask_0,0,0);
    VL_IN8(mask_11,0,0);
    VL_IN8(mask_14,0,0);
    VL_IN8(raddr_1,0,0);
    VL_IN8(mask_1,0,0);
    VL_IN8(write_en,0,0);
    VL_IN8(raddr_2,0,0);
    VL_IN8(rclke,0,0);
    VL_IN8(din_10,0,0);
    VL_IN8(din_12,0,0);
    VL_IN8(raddr_3,0,0);
    VL_IN8(din_14,0,0);
    VL_IN8(mask_3,0,0);
    VL_IN8(din_2,0,0);
    VL_IN8(din_5,0,0);
    VL_IN8(din_4,0,0);
    VL_IN8(waddr_6,0,0);
    VL_IN8(waddr_4,0,0);
    VL_IN8(din_11,0,0);
    VL_IN8(mask_12,0,0);
    VL_OUT8(dout_10,0,0);
    VL_OUT8(dout_15,0,0);
    VL_OUT8(dout_7,0,0);
    VL_OUT8(dout_5,0,0);
    VL_OUT8(dout_8,0,0);
    VL_OUT8(dout_0,0,0);
    VL_OUT8(dout_2,0,0);
    VL_OUT8(dout_13,0,0);
    VL_OUT8(dout_3,0,0);
    VL_OUT8(dout_14,0,0);
    VL_OUT8(dout_9,0,0);
    VL_OUT8(dout_1,0,0);
    VL_OUT8(dout_4,0,0);
    VL_OUT8(dout_11,0,0);
    VL_OUT8(dout_6,0,0);
    VL_OUT8(dout_12,0,0);
    
    // LOCAL SIGNALS
    // Internals; generally not touched by application code
    CData/*0:0*/ bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g;
    CData/*0:0*/ bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g;
    CData/*0:0*/ bram_netlist__DOT__ram256x16_inst__DOT__WCLKE_sync;
    CData/*0:0*/ bram_netlist__DOT__ram256x16_inst__DOT__RCLKE_sync;
    CData/*0:0*/ bram_netlist__DOT__ram256x16_inst__DOT__Time_Collision_Detected;
    CData/*0:0*/ bram_netlist__DOT__ram256x16_inst__DOT__Address_Collision_Detected;
    IData/*31:0*/ bram_netlist__DOT__ram256x16_inst__DOT__i;
    IData/*31:0*/ bram_netlist__DOT__ram256x16_inst__DOT__j;
    QData/*63:0*/ bram_netlist__DOT__ram256x16_inst__DOT__time_WCLK;
    QData/*63:0*/ bram_netlist__DOT__ram256x16_inst__DOT__time_RCLK;
    CData/*0:0*/ bram_netlist__DOT__ram256x16_inst__DOT__Memory[4096];
    
    // LOCAL VARIABLES
    // Internals; generally not touched by application code
    CData/*7:0*/ bram_netlist__DOT____Vcellinp__ram256x16_inst__RADDR;
    CData/*7:0*/ bram_netlist__DOT____Vcellinp__ram256x16_inst__WADDR;
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
    CData/*0:0*/ __Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__RCLK_g;
    CData/*0:0*/ __Vclklast__TOP__bram_netlist__DOT__ram256x16_inst__DOT__WCLK_g;
    SData/*15:0*/ bram_netlist__DOT____Vcellout__ram256x16_inst__RDATA;
    SData/*15:0*/ bram_netlist__DOT____Vcellinp__ram256x16_inst__WDATA;
    SData/*15:0*/ bram_netlist__DOT____Vcellinp__ram256x16_inst__MASK;
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
    IData/*31:0*/ __Vm_traceActivity;
    
    // INTERNAL VARIABLES
    // Internals; generally not touched by application code
    Vbram_netlist__Syms* __VlSymsp;  // Symbol table
    
    // CONSTRUCTORS
  private:
    VL_UNCOPYABLE(Vbram_netlist);  ///< Copying not allowed
  public:
    /// Construct the model; called by application code
    /// The special name  may be used to make a wrapper with a
    /// single model invisible with respect to DPI scope names.
    Vbram_netlist(const char* name = "TOP");
    /// Destroy the model; called (often implicitly) by application code
    ~Vbram_netlist();
    /// Trace signals in the model; called by application code
    void trace(VerilatedVcdC* tfp, int levels, int options = 0);
    
    // API METHODS
    /// Evaluate the model.  Application must call when inputs change.
    void eval();
    /// Simulation complete, run final blocks.  Application must call on completion.
    void final();
    
    // INTERNAL METHODS
  private:
    static void _eval_initial_loop(Vbram_netlist__Syms* __restrict vlSymsp);
  public:
    void __Vconfigure(Vbram_netlist__Syms* symsp, bool first);
  private:
    static QData _change_request(Vbram_netlist__Syms* __restrict vlSymsp);
  public:
    static void _combo__TOP__2(Vbram_netlist__Syms* __restrict vlSymsp);
    static void _combo__TOP__8(Vbram_netlist__Syms* __restrict vlSymsp);
  private:
    void _ctor_var_reset() VL_ATTR_COLD;
  public:
    static void _eval(Vbram_netlist__Syms* __restrict vlSymsp);
  private:
#ifdef VL_DEBUG
    void _eval_debug_assertions();
#endif  // VL_DEBUG
  public:
    static void _eval_initial(Vbram_netlist__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _eval_settle(Vbram_netlist__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _initial__TOP__1(Vbram_netlist__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _sequent__TOP__10(Vbram_netlist__Syms* __restrict vlSymsp);
    static void _sequent__TOP__4(Vbram_netlist__Syms* __restrict vlSymsp);
    static void _sequent__TOP__6(Vbram_netlist__Syms* __restrict vlSymsp);
    static void _sequent__TOP__7(Vbram_netlist__Syms* __restrict vlSymsp);
    static void _sequent__TOP__9(Vbram_netlist__Syms* __restrict vlSymsp);
    static void _settle__TOP__3(Vbram_netlist__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void traceChgThis(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceChgThis__2(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceChgThis__3(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceChgThis__4(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceChgThis__5(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceChgThis__6(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceChgThis__7(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceFullThis(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) VL_ATTR_COLD;
    static void traceFullThis__1(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) VL_ATTR_COLD;
    static void traceInitThis(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) VL_ATTR_COLD;
    static void traceInitThis__1(Vbram_netlist__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) VL_ATTR_COLD;
    static void traceInit(VerilatedVcd* vcdp, void* userthis, uint32_t code);
    static void traceFull(VerilatedVcd* vcdp, void* userthis, uint32_t code);
    static void traceChg(VerilatedVcd* vcdp, void* userthis, uint32_t code);
} VL_ATTR_ALIGNED(VL_CACHE_LINE_BYTES);

//----------


#endif  // guard
