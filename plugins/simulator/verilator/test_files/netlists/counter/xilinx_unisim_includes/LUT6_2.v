///////////////////////////////////////////////////////////////////////////////
//    Copyright (c) 1995/2016 Xilinx, Inc.
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor      : Xilinx
// \   \   \/     Version     : 2017.1
//  \   \         Description : Xilinx Unified Simulation Library Component
//  /   /                  6-Bit Look-Up Table with Two Outputs
// /___/   /\     Filename : LUT6_2.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
//  Revision:
//    08/08/06 - Initial version.
//    06/04/07 - Change timescale form 100ps/10ps to 1ps/1ps.
//               Add wire definition.
//    06/19/07 - 441956 - Add LOC Parameter
//    12/13/11 - 524859 - Added `celldefine and `endcelldefine
//    09/12/16 - ANSI ports, speed improvements
//  End Revision:
///////////////////////////////////////////////////////////////////////////////

`timescale 1 ps/1 ps

`celldefine

module LUT6_2 #(
`ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
`endif
  parameter [63:0] INIT = 64'h0000000000000000
)(
  output O5,
  output O6,

  input I0,
  input I1,
  input I2,
  input I3,
  input I4,
  input I5
);

// define constants
  localparam MODULE_NAME = "LUT6_2";

  reg trig_attr = 1'b0;
// include dynamic registers - XILINX test only
`ifdef XIL_DR
  `include "LUT6_2_dr.v"
`else
  reg [63:0] INIT_REG = INIT;
`endif

// begin behavioral model

  reg O5_out;
  reg O6_out;

  assign O5 = O5_out;
  assign O6 = O6_out;

  function lut_mux4_f;
  input [3:0] d;
  input [1:0] s;
  begin
    if (((s[1]^s[0]) === 1'b1) || ((s[1]^s[0]) === 1'b0))
      lut_mux4_f = d[s];
    else if ( ~(|d) || &d)
      lut_mux4_f = d[0];
    else if (((s[0] === 1'b1) || (s[0] === 1'b0)) && (d[{1'b0,s[0]}] === d[{1'b1,s[0]}]))
      lut_mux4_f = d[{1'b0,s[0]}];
    else if (((s[1] === 1'b1) || (s[1] === 1'b0)) && (d[{s[1],1'b0}] === d[{s[1],1'b1}]))
      lut_mux4_f = d[{s[1],1'b0}];
    else
      lut_mux4_f = 1'bx;
  end
  endfunction

  function lut_mux8_f;
  input [7:0] d;
  input [2:0] s;
  begin
    if (((s[2]^s[1]^s[0]) === 1'b1) || ((s[2]^s[1]^s[0]) === 1'b0))
      lut_mux8_f = d[s];
    else if ( ~(|d) || &d)
      lut_mux8_f = d[0];
    else if ((((s[1]^s[0]) === 1'b1) || ((s[1]^s[0]) === 1'b0)) &&
             (d[{1'b0,s[1:0]}] === d[{1'b1,s[1:0]}]))
      lut_mux8_f = d[{1'b0,s[1:0]}];
    else if ((((s[2]^s[0]) === 1'b1) || ((s[2]^s[0]) === 1'b0)) &&
             (d[{s[2],1'b0,s[0]}] === d[{s[2],1'b1,s[0]}]))
      lut_mux8_f = d[{s[2],1'b0,s[0]}];
    else if ((((s[2]^s[1]) === 1'b1) || ((s[2]^s[1]) === 1'b0)) &&
             (d[{s[2],s[1],1'b0}] === d[{s[2],s[1],1'b1}]))
      lut_mux8_f = d[{s[2:1],1'b0}];
    else if (((s[0] === 1'b1) || (s[0] === 1'b0)) &&
             (d[{1'b0,1'b0,s[0]}] === d[{1'b0,1'b1,s[0]}]) &&
             (d[{1'b0,1'b0,s[0]}] === d[{1'b1,1'b0,s[0]}]) &&
             (d[{1'b0,1'b0,s[0]}] === d[{1'b1,1'b1,s[0]}]))
      lut_mux8_f = d[{1'b0,1'b0,s[0]}];
    else if (((s[1] === 1'b1) || (s[1] === 1'b0)) &&
             (d[{1'b0,s[1],1'b0}] === d[{1'b0,s[1],1'b1}]) &&
             (d[{1'b0,s[1],1'b0}] === d[{1'b1,s[1],1'b0}]) &&
             (d[{1'b0,s[1],1'b0}] === d[{1'b1,s[1],1'b1}]))
      lut_mux8_f = d[{1'b0,s[1],1'b0}];
    else if (((s[2] === 1'b1) || (s[2] === 1'b0)) &&
             (d[{s[2],1'b0,1'b0}] === d[{s[2],1'b0,1'b1}]) &&
             (d[{s[2],1'b0,1'b0}] === d[{s[2],1'b1,1'b0}]) &&
             (d[{s[2],1'b0,1'b0}] === d[{s[2],1'b1,1'b1}]))
      lut_mux8_f = d[{s[2],1'b0,1'b0}];
    else
      lut_mux8_f = 1'bx;
  end
  endfunction

 always @(I0 or I1 or I2 or I3 or I4)  begin
   if ( (I0 ^ I1 ^ I2 ^ I3 ^ I4) === 1'b0 || (I0 ^ I1 ^ I2 ^ I3 ^ I4) === 1'b1)
     O5_out = INIT_REG[{I4, I3, I2, I1, I0}];
   else if ( ~(|INIT_REG[31:0]) || &INIT_REG[31:0] )
     O5_out = INIT_REG[0];
   else
     O5_out = lut_mux4_f ({lut_mux8_f (INIT_REG[31:24], {I2, I1, I0}),
                       lut_mux8_f (INIT_REG[23:16], {I2, I1, I0}),
                       lut_mux8_f ( INIT_REG[15:8], {I2, I1, I0}),
                       lut_mux8_f (  INIT_REG[7:0], {I2, I1, I0})}, {I4, I3});
 end

 always @(I0 or I1 or I2 or I3 or I4 or I5)  begin
   if ( (I0 ^ I1 ^ I2 ^ I3 ^ I4 ^ I5) === 1'b0 || (I0 ^ I1 ^ I2 ^ I3 ^ I4 ^ I5) === 1'b1)
     O6_out = INIT_REG[{I5, I4, I3, I2, I1, I0}];
   else if ( ~(|INIT_REG) || &INIT_REG )
     O6_out = INIT_REG[0];
   else
     O6_out = lut_mux8_f ({lut_mux8_f (INIT_REG[63:56], {I2, I1, I0}),
                       lut_mux8_f (INIT_REG[55:48], {I2, I1, I0}),
                       lut_mux8_f (INIT_REG[47:40], {I2, I1, I0}),
                       lut_mux8_f (INIT_REG[39:32], {I2, I1, I0}),
                       lut_mux8_f (INIT_REG[31:24], {I2, I1, I0}),
                       lut_mux8_f (INIT_REG[23:16], {I2, I1, I0}),
                       lut_mux8_f ( INIT_REG[15:8], {I2, I1, I0}),
                       lut_mux8_f (  INIT_REG[7:0], {I2, I1, I0})}, {I5, I4, I3});
 end

// end behavioral model

`ifdef XIL_TIMING
  specify
   (I0 => O5) = (0:0:0, 0:0:0);
   (I1 => O5) = (0:0:0, 0:0:0);
   (I2 => O5) = (0:0:0, 0:0:0);
   (I3 => O5) = (0:0:0, 0:0:0);
   (I4 => O5) = (0:0:0, 0:0:0);
   (I0 => O6) = (0:0:0, 0:0:0);
   (I1 => O6) = (0:0:0, 0:0:0);
   (I2 => O6) = (0:0:0, 0:0:0);
   (I3 => O6) = (0:0:0, 0:0:0);
   (I4 => O6) = (0:0:0, 0:0:0);
   (I5 => O6) = (0:0:0, 0:0:0);
	specparam PATHPULSE$ = 0;
  endspecify
`endif

endmodule

`endcelldefine
