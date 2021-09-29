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
//  /   /                  4-Bit Look-Up Table
// /___/   /\     Filename : LUT4.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
//  Revision:
//    03/23/04 - Initial version.
//    02/04/05 - Replace primitive with function; Remove buf.
//    03/11/05 - Add LOC Parameter
//    06/04/07 - Add wire declaration to internal signal.
//    12/13/11 - 524859 - Added `celldefine and `endcelldefine
//    09/12/16 - ANSI ports, speed improvements
//  End Revision:
///////////////////////////////////////////////////////////////////////////////

`timescale 1 ps/1 ps

`celldefine

module LUT4 #(
`ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
`endif
  parameter [15:0] INIT = 16'h0000
)(
  output O,

  input I0,
  input I1,
  input I2,
  input I3
);

// define constants
  localparam MODULE_NAME = "LUT4";

  reg trig_attr = 1'b0;
// include dynamic registers - XILINX test only
`ifdef XIL_DR
  `include "LUT4_dr.v"
`else
  reg [15:0] INIT_REG = INIT;
`endif

// begin behavioral model

  reg O_out;

  assign O = O_out;

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

 always @(I0 or I1 or I2 or I3)  begin
   if ( (I0 ^ I1  ^ I2 ^ I3) === 1'b0 || (I0 ^ I1  ^ I2 ^ I3) === 1'b1)
    O_out = INIT_REG[{I3, I2, I1, I0}];
   else if ( ~(|INIT_REG) || &INIT_REG )
    O_out = INIT_REG[0];
   else
    O_out = lut_mux4_f ({lut_mux4_f (INIT_REG[15:12], {I1, I0}),
                     lut_mux4_f ( INIT_REG[11:8], {I1, I0}),
                     lut_mux4_f (  INIT_REG[7:4], {I1, I0}),
                     lut_mux4_f (  INIT_REG[3:0], {I1, I0})}, {I3, I2});
  end

// end behavioral model

`ifdef XIL_TIMING
  specify
	(I0 => O) = (0:0:0, 0:0:0);
	(I1 => O) = (0:0:0, 0:0:0);
	(I2 => O) = (0:0:0, 0:0:0);
	(I3 => O) = (0:0:0, 0:0:0);
	specparam PATHPULSE$ = 0;
  endspecify
`endif

endmodule

`endcelldefine
