// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2022.2 (lin64) Build 3671981 Fri Oct 14 04:59:54 MDT 2022
// Date        : Thu Mar 30 22:43:32 2023
// Host        : XPS15-linux running 64-bit Ubuntu 22.04.1 LTS
// Command     : write_verilog -force
//               /home/simon/projects/benchmarks/netlists/vivado/xc7k70tfbg484-3/basic/arithmetic/addition/addition_carry_enable/addition_carry_enable/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v
// Design      : top
// Purpose     : This is a Verilog netlist of the current design or from a specific cell of the design. The output is an
//               IEEE 1364-2001 compliant Verilog HDL file that contains netlist information obtained from the input
//               design files.
// Device      : xc7k70tfbg484-3
// --------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* WIDTH = "32" *) 
(* STRUCTURAL_NETLIST = "yes" *)
module top
   (a,
    b,
    enable,
    out);
  input [31:0]a;
  input [31:0]b;
  input enable;
  output [32:0]out;

  wire \<const0> ;
  wire \<const1> ;
  wire [31:0]a;
  wire [31:0]a_IBUF;
  wire [31:0]b;
  wire [31:0]b_IBUF;
  wire enable;
  wire enable_IBUF;
  wire [32:0]out;
  wire [32:0]out0;
  wire [32:0]out_OBUF;
  wire \out_OBUF[11]_inst_i_2_n_0 ;
  wire \out_OBUF[11]_inst_i_2_n_1 ;
  wire \out_OBUF[11]_inst_i_2_n_2 ;
  wire \out_OBUF[11]_inst_i_2_n_3 ;
  wire \out_OBUF[11]_inst_i_3_n_0 ;
  wire \out_OBUF[11]_inst_i_4_n_0 ;
  wire \out_OBUF[11]_inst_i_5_n_0 ;
  wire \out_OBUF[11]_inst_i_6_n_0 ;
  wire \out_OBUF[15]_inst_i_2_n_0 ;
  wire \out_OBUF[15]_inst_i_2_n_1 ;
  wire \out_OBUF[15]_inst_i_2_n_2 ;
  wire \out_OBUF[15]_inst_i_2_n_3 ;
  wire \out_OBUF[15]_inst_i_3_n_0 ;
  wire \out_OBUF[15]_inst_i_4_n_0 ;
  wire \out_OBUF[15]_inst_i_5_n_0 ;
  wire \out_OBUF[15]_inst_i_6_n_0 ;
  wire \out_OBUF[19]_inst_i_2_n_0 ;
  wire \out_OBUF[19]_inst_i_2_n_1 ;
  wire \out_OBUF[19]_inst_i_2_n_2 ;
  wire \out_OBUF[19]_inst_i_2_n_3 ;
  wire \out_OBUF[19]_inst_i_3_n_0 ;
  wire \out_OBUF[19]_inst_i_4_n_0 ;
  wire \out_OBUF[19]_inst_i_5_n_0 ;
  wire \out_OBUF[19]_inst_i_6_n_0 ;
  wire \out_OBUF[23]_inst_i_2_n_0 ;
  wire \out_OBUF[23]_inst_i_2_n_1 ;
  wire \out_OBUF[23]_inst_i_2_n_2 ;
  wire \out_OBUF[23]_inst_i_2_n_3 ;
  wire \out_OBUF[23]_inst_i_3_n_0 ;
  wire \out_OBUF[23]_inst_i_4_n_0 ;
  wire \out_OBUF[23]_inst_i_5_n_0 ;
  wire \out_OBUF[23]_inst_i_6_n_0 ;
  wire \out_OBUF[27]_inst_i_2_n_0 ;
  wire \out_OBUF[27]_inst_i_2_n_1 ;
  wire \out_OBUF[27]_inst_i_2_n_2 ;
  wire \out_OBUF[27]_inst_i_2_n_3 ;
  wire \out_OBUF[27]_inst_i_3_n_0 ;
  wire \out_OBUF[27]_inst_i_4_n_0 ;
  wire \out_OBUF[27]_inst_i_5_n_0 ;
  wire \out_OBUF[27]_inst_i_6_n_0 ;
  wire \out_OBUF[31]_inst_i_2_n_0 ;
  wire \out_OBUF[31]_inst_i_2_n_1 ;
  wire \out_OBUF[31]_inst_i_2_n_2 ;
  wire \out_OBUF[31]_inst_i_2_n_3 ;
  wire \out_OBUF[31]_inst_i_3_n_0 ;
  wire \out_OBUF[31]_inst_i_4_n_0 ;
  wire \out_OBUF[31]_inst_i_5_n_0 ;
  wire \out_OBUF[31]_inst_i_6_n_0 ;
  wire \out_OBUF[3]_inst_i_2_n_0 ;
  wire \out_OBUF[3]_inst_i_2_n_1 ;
  wire \out_OBUF[3]_inst_i_2_n_2 ;
  wire \out_OBUF[3]_inst_i_2_n_3 ;
  wire \out_OBUF[3]_inst_i_3_n_0 ;
  wire \out_OBUF[3]_inst_i_4_n_0 ;
  wire \out_OBUF[3]_inst_i_5_n_0 ;
  wire \out_OBUF[3]_inst_i_6_n_0 ;
  wire \out_OBUF[7]_inst_i_2_n_0 ;
  wire \out_OBUF[7]_inst_i_2_n_1 ;
  wire \out_OBUF[7]_inst_i_2_n_2 ;
  wire \out_OBUF[7]_inst_i_2_n_3 ;
  wire \out_OBUF[7]_inst_i_3_n_0 ;
  wire \out_OBUF[7]_inst_i_4_n_0 ;
  wire \out_OBUF[7]_inst_i_5_n_0 ;
  wire \out_OBUF[7]_inst_i_6_n_0 ;

  GND GND
       (.G(\<const0> ));
  VCC VCC
       (.P(\<const1> ));
  IBUF \a_IBUF[0]_inst 
       (.I(a[0]),
        .O(a_IBUF[0]));
  IBUF \a_IBUF[10]_inst 
       (.I(a[10]),
        .O(a_IBUF[10]));
  IBUF \a_IBUF[11]_inst 
       (.I(a[11]),
        .O(a_IBUF[11]));
  IBUF \a_IBUF[12]_inst 
       (.I(a[12]),
        .O(a_IBUF[12]));
  IBUF \a_IBUF[13]_inst 
       (.I(a[13]),
        .O(a_IBUF[13]));
  IBUF \a_IBUF[14]_inst 
       (.I(a[14]),
        .O(a_IBUF[14]));
  IBUF \a_IBUF[15]_inst 
       (.I(a[15]),
        .O(a_IBUF[15]));
  IBUF \a_IBUF[16]_inst 
       (.I(a[16]),
        .O(a_IBUF[16]));
  IBUF \a_IBUF[17]_inst 
       (.I(a[17]),
        .O(a_IBUF[17]));
  IBUF \a_IBUF[18]_inst 
       (.I(a[18]),
        .O(a_IBUF[18]));
  IBUF \a_IBUF[19]_inst 
       (.I(a[19]),
        .O(a_IBUF[19]));
  IBUF \a_IBUF[1]_inst 
       (.I(a[1]),
        .O(a_IBUF[1]));
  IBUF \a_IBUF[20]_inst 
       (.I(a[20]),
        .O(a_IBUF[20]));
  IBUF \a_IBUF[21]_inst 
       (.I(a[21]),
        .O(a_IBUF[21]));
  IBUF \a_IBUF[22]_inst 
       (.I(a[22]),
        .O(a_IBUF[22]));
  IBUF \a_IBUF[23]_inst 
       (.I(a[23]),
        .O(a_IBUF[23]));
  IBUF \a_IBUF[24]_inst 
       (.I(a[24]),
        .O(a_IBUF[24]));
  IBUF \a_IBUF[25]_inst 
       (.I(a[25]),
        .O(a_IBUF[25]));
  IBUF \a_IBUF[26]_inst 
       (.I(a[26]),
        .O(a_IBUF[26]));
  IBUF \a_IBUF[27]_inst 
       (.I(a[27]),
        .O(a_IBUF[27]));
  IBUF \a_IBUF[28]_inst 
       (.I(a[28]),
        .O(a_IBUF[28]));
  IBUF \a_IBUF[29]_inst 
       (.I(a[29]),
        .O(a_IBUF[29]));
  IBUF \a_IBUF[2]_inst 
       (.I(a[2]),
        .O(a_IBUF[2]));
  IBUF \a_IBUF[30]_inst 
       (.I(a[30]),
        .O(a_IBUF[30]));
  IBUF \a_IBUF[31]_inst 
       (.I(a[31]),
        .O(a_IBUF[31]));
  IBUF \a_IBUF[3]_inst 
       (.I(a[3]),
        .O(a_IBUF[3]));
  IBUF \a_IBUF[4]_inst 
       (.I(a[4]),
        .O(a_IBUF[4]));
  IBUF \a_IBUF[5]_inst 
       (.I(a[5]),
        .O(a_IBUF[5]));
  IBUF \a_IBUF[6]_inst 
       (.I(a[6]),
        .O(a_IBUF[6]));
  IBUF \a_IBUF[7]_inst 
       (.I(a[7]),
        .O(a_IBUF[7]));
  IBUF \a_IBUF[8]_inst 
       (.I(a[8]),
        .O(a_IBUF[8]));
  IBUF \a_IBUF[9]_inst 
       (.I(a[9]),
        .O(a_IBUF[9]));
  IBUF \b_IBUF[0]_inst 
       (.I(b[0]),
        .O(b_IBUF[0]));
  IBUF \b_IBUF[10]_inst 
       (.I(b[10]),
        .O(b_IBUF[10]));
  IBUF \b_IBUF[11]_inst 
       (.I(b[11]),
        .O(b_IBUF[11]));
  IBUF \b_IBUF[12]_inst 
       (.I(b[12]),
        .O(b_IBUF[12]));
  IBUF \b_IBUF[13]_inst 
       (.I(b[13]),
        .O(b_IBUF[13]));
  IBUF \b_IBUF[14]_inst 
       (.I(b[14]),
        .O(b_IBUF[14]));
  IBUF \b_IBUF[15]_inst 
       (.I(b[15]),
        .O(b_IBUF[15]));
  IBUF \b_IBUF[16]_inst 
       (.I(b[16]),
        .O(b_IBUF[16]));
  IBUF \b_IBUF[17]_inst 
       (.I(b[17]),
        .O(b_IBUF[17]));
  IBUF \b_IBUF[18]_inst 
       (.I(b[18]),
        .O(b_IBUF[18]));
  IBUF \b_IBUF[19]_inst 
       (.I(b[19]),
        .O(b_IBUF[19]));
  IBUF \b_IBUF[1]_inst 
       (.I(b[1]),
        .O(b_IBUF[1]));
  IBUF \b_IBUF[20]_inst 
       (.I(b[20]),
        .O(b_IBUF[20]));
  IBUF \b_IBUF[21]_inst 
       (.I(b[21]),
        .O(b_IBUF[21]));
  IBUF \b_IBUF[22]_inst 
       (.I(b[22]),
        .O(b_IBUF[22]));
  IBUF \b_IBUF[23]_inst 
       (.I(b[23]),
        .O(b_IBUF[23]));
  IBUF \b_IBUF[24]_inst 
       (.I(b[24]),
        .O(b_IBUF[24]));
  IBUF \b_IBUF[25]_inst 
       (.I(b[25]),
        .O(b_IBUF[25]));
  IBUF \b_IBUF[26]_inst 
       (.I(b[26]),
        .O(b_IBUF[26]));
  IBUF \b_IBUF[27]_inst 
       (.I(b[27]),
        .O(b_IBUF[27]));
  IBUF \b_IBUF[28]_inst 
       (.I(b[28]),
        .O(b_IBUF[28]));
  IBUF \b_IBUF[29]_inst 
       (.I(b[29]),
        .O(b_IBUF[29]));
  IBUF \b_IBUF[2]_inst 
       (.I(b[2]),
        .O(b_IBUF[2]));
  IBUF \b_IBUF[30]_inst 
       (.I(b[30]),
        .O(b_IBUF[30]));
  IBUF \b_IBUF[31]_inst 
       (.I(b[31]),
        .O(b_IBUF[31]));
  IBUF \b_IBUF[3]_inst 
       (.I(b[3]),
        .O(b_IBUF[3]));
  IBUF \b_IBUF[4]_inst 
       (.I(b[4]),
        .O(b_IBUF[4]));
  IBUF \b_IBUF[5]_inst 
       (.I(b[5]),
        .O(b_IBUF[5]));
  IBUF \b_IBUF[6]_inst 
       (.I(b[6]),
        .O(b_IBUF[6]));
  IBUF \b_IBUF[7]_inst 
       (.I(b[7]),
        .O(b_IBUF[7]));
  IBUF \b_IBUF[8]_inst 
       (.I(b[8]),
        .O(b_IBUF[8]));
  IBUF \b_IBUF[9]_inst 
       (.I(b[9]),
        .O(b_IBUF[9]));
  IBUF enable_IBUF_inst
       (.I(enable),
        .O(enable_IBUF));
  OBUF \out_OBUF[0]_inst 
       (.I(out_OBUF[0]),
        .O(out[0]));
  (* SOFT_HLUTNM = "soft_lutpair0" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[0]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[0]),
        .O(out_OBUF[0]));
  OBUF \out_OBUF[10]_inst 
       (.I(out_OBUF[10]),
        .O(out[10]));
  (* SOFT_HLUTNM = "soft_lutpair5" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[10]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[10]),
        .O(out_OBUF[10]));
  OBUF \out_OBUF[11]_inst 
       (.I(out_OBUF[11]),
        .O(out[11]));
  (* SOFT_HLUTNM = "soft_lutpair5" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[11]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[11]),
        .O(out_OBUF[11]));
  CARRY4 \out_OBUF[11]_inst_i_2 
       (.CI(\out_OBUF[7]_inst_i_2_n_0 ),
        .CO({\out_OBUF[11]_inst_i_2_n_0 ,\out_OBUF[11]_inst_i_2_n_1 ,\out_OBUF[11]_inst_i_2_n_2 ,\out_OBUF[11]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[11:8]),
        .O(out0[11:8]),
        .S({\out_OBUF[11]_inst_i_3_n_0 ,\out_OBUF[11]_inst_i_4_n_0 ,\out_OBUF[11]_inst_i_5_n_0 ,\out_OBUF[11]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[11]_inst_i_3 
       (.I0(a_IBUF[11]),
        .I1(b_IBUF[11]),
        .O(\out_OBUF[11]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[11]_inst_i_4 
       (.I0(a_IBUF[10]),
        .I1(b_IBUF[10]),
        .O(\out_OBUF[11]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[11]_inst_i_5 
       (.I0(a_IBUF[9]),
        .I1(b_IBUF[9]),
        .O(\out_OBUF[11]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[11]_inst_i_6 
       (.I0(a_IBUF[8]),
        .I1(b_IBUF[8]),
        .O(\out_OBUF[11]_inst_i_6_n_0 ));
  OBUF \out_OBUF[12]_inst 
       (.I(out_OBUF[12]),
        .O(out[12]));
  (* SOFT_HLUTNM = "soft_lutpair6" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[12]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[12]),
        .O(out_OBUF[12]));
  OBUF \out_OBUF[13]_inst 
       (.I(out_OBUF[13]),
        .O(out[13]));
  (* SOFT_HLUTNM = "soft_lutpair6" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[13]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[13]),
        .O(out_OBUF[13]));
  OBUF \out_OBUF[14]_inst 
       (.I(out_OBUF[14]),
        .O(out[14]));
  (* SOFT_HLUTNM = "soft_lutpair7" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[14]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[14]),
        .O(out_OBUF[14]));
  OBUF \out_OBUF[15]_inst 
       (.I(out_OBUF[15]),
        .O(out[15]));
  (* SOFT_HLUTNM = "soft_lutpair7" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[15]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[15]),
        .O(out_OBUF[15]));
  CARRY4 \out_OBUF[15]_inst_i_2 
       (.CI(\out_OBUF[11]_inst_i_2_n_0 ),
        .CO({\out_OBUF[15]_inst_i_2_n_0 ,\out_OBUF[15]_inst_i_2_n_1 ,\out_OBUF[15]_inst_i_2_n_2 ,\out_OBUF[15]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[15:12]),
        .O(out0[15:12]),
        .S({\out_OBUF[15]_inst_i_3_n_0 ,\out_OBUF[15]_inst_i_4_n_0 ,\out_OBUF[15]_inst_i_5_n_0 ,\out_OBUF[15]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[15]_inst_i_3 
       (.I0(a_IBUF[15]),
        .I1(b_IBUF[15]),
        .O(\out_OBUF[15]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[15]_inst_i_4 
       (.I0(a_IBUF[14]),
        .I1(b_IBUF[14]),
        .O(\out_OBUF[15]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[15]_inst_i_5 
       (.I0(a_IBUF[13]),
        .I1(b_IBUF[13]),
        .O(\out_OBUF[15]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[15]_inst_i_6 
       (.I0(a_IBUF[12]),
        .I1(b_IBUF[12]),
        .O(\out_OBUF[15]_inst_i_6_n_0 ));
  OBUF \out_OBUF[16]_inst 
       (.I(out_OBUF[16]),
        .O(out[16]));
  (* SOFT_HLUTNM = "soft_lutpair8" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[16]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[16]),
        .O(out_OBUF[16]));
  OBUF \out_OBUF[17]_inst 
       (.I(out_OBUF[17]),
        .O(out[17]));
  (* SOFT_HLUTNM = "soft_lutpair8" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[17]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[17]),
        .O(out_OBUF[17]));
  OBUF \out_OBUF[18]_inst 
       (.I(out_OBUF[18]),
        .O(out[18]));
  (* SOFT_HLUTNM = "soft_lutpair9" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[18]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[18]),
        .O(out_OBUF[18]));
  OBUF \out_OBUF[19]_inst 
       (.I(out_OBUF[19]),
        .O(out[19]));
  (* SOFT_HLUTNM = "soft_lutpair9" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[19]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[19]),
        .O(out_OBUF[19]));
  CARRY4 \out_OBUF[19]_inst_i_2 
       (.CI(\out_OBUF[15]_inst_i_2_n_0 ),
        .CO({\out_OBUF[19]_inst_i_2_n_0 ,\out_OBUF[19]_inst_i_2_n_1 ,\out_OBUF[19]_inst_i_2_n_2 ,\out_OBUF[19]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[19:16]),
        .O(out0[19:16]),
        .S({\out_OBUF[19]_inst_i_3_n_0 ,\out_OBUF[19]_inst_i_4_n_0 ,\out_OBUF[19]_inst_i_5_n_0 ,\out_OBUF[19]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[19]_inst_i_3 
       (.I0(a_IBUF[19]),
        .I1(b_IBUF[19]),
        .O(\out_OBUF[19]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[19]_inst_i_4 
       (.I0(a_IBUF[18]),
        .I1(b_IBUF[18]),
        .O(\out_OBUF[19]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[19]_inst_i_5 
       (.I0(a_IBUF[17]),
        .I1(b_IBUF[17]),
        .O(\out_OBUF[19]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[19]_inst_i_6 
       (.I0(a_IBUF[16]),
        .I1(b_IBUF[16]),
        .O(\out_OBUF[19]_inst_i_6_n_0 ));
  OBUF \out_OBUF[1]_inst 
       (.I(out_OBUF[1]),
        .O(out[1]));
  (* SOFT_HLUTNM = "soft_lutpair0" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[1]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[1]),
        .O(out_OBUF[1]));
  OBUF \out_OBUF[20]_inst 
       (.I(out_OBUF[20]),
        .O(out[20]));
  (* SOFT_HLUTNM = "soft_lutpair10" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[20]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[20]),
        .O(out_OBUF[20]));
  OBUF \out_OBUF[21]_inst 
       (.I(out_OBUF[21]),
        .O(out[21]));
  (* SOFT_HLUTNM = "soft_lutpair10" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[21]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[21]),
        .O(out_OBUF[21]));
  OBUF \out_OBUF[22]_inst 
       (.I(out_OBUF[22]),
        .O(out[22]));
  (* SOFT_HLUTNM = "soft_lutpair11" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[22]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[22]),
        .O(out_OBUF[22]));
  OBUF \out_OBUF[23]_inst 
       (.I(out_OBUF[23]),
        .O(out[23]));
  (* SOFT_HLUTNM = "soft_lutpair11" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[23]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[23]),
        .O(out_OBUF[23]));
  CARRY4 \out_OBUF[23]_inst_i_2 
       (.CI(\out_OBUF[19]_inst_i_2_n_0 ),
        .CO({\out_OBUF[23]_inst_i_2_n_0 ,\out_OBUF[23]_inst_i_2_n_1 ,\out_OBUF[23]_inst_i_2_n_2 ,\out_OBUF[23]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[23:20]),
        .O(out0[23:20]),
        .S({\out_OBUF[23]_inst_i_3_n_0 ,\out_OBUF[23]_inst_i_4_n_0 ,\out_OBUF[23]_inst_i_5_n_0 ,\out_OBUF[23]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[23]_inst_i_3 
       (.I0(a_IBUF[23]),
        .I1(b_IBUF[23]),
        .O(\out_OBUF[23]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[23]_inst_i_4 
       (.I0(a_IBUF[22]),
        .I1(b_IBUF[22]),
        .O(\out_OBUF[23]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[23]_inst_i_5 
       (.I0(a_IBUF[21]),
        .I1(b_IBUF[21]),
        .O(\out_OBUF[23]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[23]_inst_i_6 
       (.I0(a_IBUF[20]),
        .I1(b_IBUF[20]),
        .O(\out_OBUF[23]_inst_i_6_n_0 ));
  OBUF \out_OBUF[24]_inst 
       (.I(out_OBUF[24]),
        .O(out[24]));
  (* SOFT_HLUTNM = "soft_lutpair12" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[24]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[24]),
        .O(out_OBUF[24]));
  OBUF \out_OBUF[25]_inst 
       (.I(out_OBUF[25]),
        .O(out[25]));
  (* SOFT_HLUTNM = "soft_lutpair12" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[25]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[25]),
        .O(out_OBUF[25]));
  OBUF \out_OBUF[26]_inst 
       (.I(out_OBUF[26]),
        .O(out[26]));
  (* SOFT_HLUTNM = "soft_lutpair13" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[26]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[26]),
        .O(out_OBUF[26]));
  OBUF \out_OBUF[27]_inst 
       (.I(out_OBUF[27]),
        .O(out[27]));
  (* SOFT_HLUTNM = "soft_lutpair13" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[27]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[27]),
        .O(out_OBUF[27]));
  CARRY4 \out_OBUF[27]_inst_i_2 
       (.CI(\out_OBUF[23]_inst_i_2_n_0 ),
        .CO({\out_OBUF[27]_inst_i_2_n_0 ,\out_OBUF[27]_inst_i_2_n_1 ,\out_OBUF[27]_inst_i_2_n_2 ,\out_OBUF[27]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[27:24]),
        .O(out0[27:24]),
        .S({\out_OBUF[27]_inst_i_3_n_0 ,\out_OBUF[27]_inst_i_4_n_0 ,\out_OBUF[27]_inst_i_5_n_0 ,\out_OBUF[27]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[27]_inst_i_3 
       (.I0(a_IBUF[27]),
        .I1(b_IBUF[27]),
        .O(\out_OBUF[27]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[27]_inst_i_4 
       (.I0(a_IBUF[26]),
        .I1(b_IBUF[26]),
        .O(\out_OBUF[27]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[27]_inst_i_5 
       (.I0(a_IBUF[25]),
        .I1(b_IBUF[25]),
        .O(\out_OBUF[27]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[27]_inst_i_6 
       (.I0(a_IBUF[24]),
        .I1(b_IBUF[24]),
        .O(\out_OBUF[27]_inst_i_6_n_0 ));
  OBUF \out_OBUF[28]_inst 
       (.I(out_OBUF[28]),
        .O(out[28]));
  (* SOFT_HLUTNM = "soft_lutpair14" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[28]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[28]),
        .O(out_OBUF[28]));
  OBUF \out_OBUF[29]_inst 
       (.I(out_OBUF[29]),
        .O(out[29]));
  (* SOFT_HLUTNM = "soft_lutpair14" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[29]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[29]),
        .O(out_OBUF[29]));
  OBUF \out_OBUF[2]_inst 
       (.I(out_OBUF[2]),
        .O(out[2]));
  (* SOFT_HLUTNM = "soft_lutpair1" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[2]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[2]),
        .O(out_OBUF[2]));
  OBUF \out_OBUF[30]_inst 
       (.I(out_OBUF[30]),
        .O(out[30]));
  (* SOFT_HLUTNM = "soft_lutpair15" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[30]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[30]),
        .O(out_OBUF[30]));
  OBUF \out_OBUF[31]_inst 
       (.I(out_OBUF[31]),
        .O(out[31]));
  (* SOFT_HLUTNM = "soft_lutpair15" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[31]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[31]),
        .O(out_OBUF[31]));
  CARRY4 \out_OBUF[31]_inst_i_2 
       (.CI(\out_OBUF[27]_inst_i_2_n_0 ),
        .CO({\out_OBUF[31]_inst_i_2_n_0 ,\out_OBUF[31]_inst_i_2_n_1 ,\out_OBUF[31]_inst_i_2_n_2 ,\out_OBUF[31]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[31:28]),
        .O(out0[31:28]),
        .S({\out_OBUF[31]_inst_i_3_n_0 ,\out_OBUF[31]_inst_i_4_n_0 ,\out_OBUF[31]_inst_i_5_n_0 ,\out_OBUF[31]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[31]_inst_i_3 
       (.I0(a_IBUF[31]),
        .I1(b_IBUF[31]),
        .O(\out_OBUF[31]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[31]_inst_i_4 
       (.I0(a_IBUF[30]),
        .I1(b_IBUF[30]),
        .O(\out_OBUF[31]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[31]_inst_i_5 
       (.I0(a_IBUF[29]),
        .I1(b_IBUF[29]),
        .O(\out_OBUF[31]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[31]_inst_i_6 
       (.I0(a_IBUF[28]),
        .I1(b_IBUF[28]),
        .O(\out_OBUF[31]_inst_i_6_n_0 ));
  OBUF \out_OBUF[32]_inst 
       (.I(out_OBUF[32]),
        .O(out[32]));
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[32]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[32]),
        .O(out_OBUF[32]));
  CARRY4 \out_OBUF[32]_inst_i_2 
       (.CI(\out_OBUF[31]_inst_i_2_n_0 ),
        .CO(out0[32]),
        .CYINIT(\<const0> ),
        .DI({\<const0> ,\<const0> ,\<const0> ,\<const0> }),
        .S({\<const0> ,\<const0> ,\<const0> ,\<const1> }));
  OBUF \out_OBUF[3]_inst 
       (.I(out_OBUF[3]),
        .O(out[3]));
  (* SOFT_HLUTNM = "soft_lutpair1" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[3]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[3]),
        .O(out_OBUF[3]));
  CARRY4 \out_OBUF[3]_inst_i_2 
       (.CI(\<const0> ),
        .CO({\out_OBUF[3]_inst_i_2_n_0 ,\out_OBUF[3]_inst_i_2_n_1 ,\out_OBUF[3]_inst_i_2_n_2 ,\out_OBUF[3]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[3:0]),
        .O(out0[3:0]),
        .S({\out_OBUF[3]_inst_i_3_n_0 ,\out_OBUF[3]_inst_i_4_n_0 ,\out_OBUF[3]_inst_i_5_n_0 ,\out_OBUF[3]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[3]_inst_i_3 
       (.I0(a_IBUF[3]),
        .I1(b_IBUF[3]),
        .O(\out_OBUF[3]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[3]_inst_i_4 
       (.I0(a_IBUF[2]),
        .I1(b_IBUF[2]),
        .O(\out_OBUF[3]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[3]_inst_i_5 
       (.I0(a_IBUF[1]),
        .I1(b_IBUF[1]),
        .O(\out_OBUF[3]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[3]_inst_i_6 
       (.I0(a_IBUF[0]),
        .I1(b_IBUF[0]),
        .O(\out_OBUF[3]_inst_i_6_n_0 ));
  OBUF \out_OBUF[4]_inst 
       (.I(out_OBUF[4]),
        .O(out[4]));
  (* SOFT_HLUTNM = "soft_lutpair2" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[4]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[4]),
        .O(out_OBUF[4]));
  OBUF \out_OBUF[5]_inst 
       (.I(out_OBUF[5]),
        .O(out[5]));
  (* SOFT_HLUTNM = "soft_lutpair2" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[5]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[5]),
        .O(out_OBUF[5]));
  OBUF \out_OBUF[6]_inst 
       (.I(out_OBUF[6]),
        .O(out[6]));
  (* SOFT_HLUTNM = "soft_lutpair3" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[6]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[6]),
        .O(out_OBUF[6]));
  OBUF \out_OBUF[7]_inst 
       (.I(out_OBUF[7]),
        .O(out[7]));
  (* SOFT_HLUTNM = "soft_lutpair3" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[7]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[7]),
        .O(out_OBUF[7]));
  CARRY4 \out_OBUF[7]_inst_i_2 
       (.CI(\out_OBUF[3]_inst_i_2_n_0 ),
        .CO({\out_OBUF[7]_inst_i_2_n_0 ,\out_OBUF[7]_inst_i_2_n_1 ,\out_OBUF[7]_inst_i_2_n_2 ,\out_OBUF[7]_inst_i_2_n_3 }),
        .CYINIT(\<const0> ),
        .DI(a_IBUF[7:4]),
        .O(out0[7:4]),
        .S({\out_OBUF[7]_inst_i_3_n_0 ,\out_OBUF[7]_inst_i_4_n_0 ,\out_OBUF[7]_inst_i_5_n_0 ,\out_OBUF[7]_inst_i_6_n_0 }));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[7]_inst_i_3 
       (.I0(a_IBUF[7]),
        .I1(b_IBUF[7]),
        .O(\out_OBUF[7]_inst_i_3_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[7]_inst_i_4 
       (.I0(a_IBUF[6]),
        .I1(b_IBUF[6]),
        .O(\out_OBUF[7]_inst_i_4_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[7]_inst_i_5 
       (.I0(a_IBUF[5]),
        .I1(b_IBUF[5]),
        .O(\out_OBUF[7]_inst_i_5_n_0 ));
  LUT2 #(
    .INIT(4'h6)) 
    \out_OBUF[7]_inst_i_6 
       (.I0(a_IBUF[4]),
        .I1(b_IBUF[4]),
        .O(\out_OBUF[7]_inst_i_6_n_0 ));
  OBUF \out_OBUF[8]_inst 
       (.I(out_OBUF[8]),
        .O(out[8]));
  (* SOFT_HLUTNM = "soft_lutpair4" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[8]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[8]),
        .O(out_OBUF[8]));
  OBUF \out_OBUF[9]_inst 
       (.I(out_OBUF[9]),
        .O(out[9]));
  (* SOFT_HLUTNM = "soft_lutpair4" *) 
  LUT2 #(
    .INIT(4'h8)) 
    \out_OBUF[9]_inst_i_1 
       (.I0(enable_IBUF),
        .I1(out0[9]),
        .O(out_OBUF[9]));
endmodule
