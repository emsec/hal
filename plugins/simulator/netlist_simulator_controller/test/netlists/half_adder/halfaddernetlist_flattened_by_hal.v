// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2018.2 (lin64) Build 2258646 Thu Jun 14 20:02:38 MDT 2018
// Date        : Thu Jun 27 14:22:14 2019
// Host        : fabian-OptiPlex-5040 running 64-bit Ubuntu 18.04.2 LTS
// Command     : write_verilog /home/fabian/Uni/Vivado/projects/half_adder/half_adder_flattened.v
// Design      : HalfAdder
// Purpose     : This is a Verilog netlist of the current design or from a specific cell of the design. The output is an
//               IEEE 1364-2001 compliant Verilog HDL file that contains netlist information obtained from the input
//               design files.
// Device      : xc7a35tcpg236-1
// --------------------------------------------------------------------------------
//`timescale 1 ps / 1 ps

(* STRUCTURAL_NETLIST = "yes" *)
module HalfAdder
   (A,
    B,
    S,
    C);
  input A;
  input B;
  output S;
  output C;

  wire A;
  wire A_IBUF;
  wire B;
  wire B_IBUF;
  wire C;
  wire C_OBUF;
  wire S;
  wire S_OBUF;

  IBUF A_IBUF_inst
       (.I(A),
        .O(A_IBUF));
  IBUF B_IBUF_inst
       (.I(B),
        .O(B_IBUF));
  OBUF C_OBUF_inst
       (.I(C_OBUF),
        .O(C));
  (* SOFT_HLUTNM = "soft_lutpair0" *) 
  LUT2 #(
    .INIT(4'h8)) 
    C_OBUF_inst_i_1
       (.I0(A_IBUF),
        .I1(B_IBUF),
        .O(C_OBUF));
  OBUF S_OBUF_inst
       (.I(S_OBUF),
        .O(S));
  (* SOFT_HLUTNM = "soft_lutpair0" *) 
  LUT2 #(
    .INIT(4'h6)) 
    S_OBUF_inst_i_1
       (.I0(B_IBUF),
        .I1(A_IBUF),
        .O(S_OBUF));
endmodule
