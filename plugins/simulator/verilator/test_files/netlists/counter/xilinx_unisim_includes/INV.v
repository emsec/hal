///////////////////////////////////////////////////////////////////////////////
//    Copyright (c) 1995/2009 Xilinx, Inc.
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
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 8.1i (I.13)
//  \   \         Description : Xilinx Timing Simulation Library Component
//  /   /                  Inverter
// /___/   /\     Filename : INV.v
// \   \  /  \    Timestamp : Thu Mar 25 16:43:55 PST 2004
//  \___\/\___\
//
// Revision:
//    03/23/04 - Initial version.
//    03/11/05 - Add LOC paramter;
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
// End Revision

`timescale 1 ps/1 ps

`celldefine

module INV (O, I);


`ifdef XIL_TIMING

  parameter LOC = "UNPLACED";

`endif

  output O;
  input I;

  not n1 (O, I);

`ifdef XIL_TIMING

  specify

	(I => O) = (0:0:0, 0:0:0);
	specparam PATHPULSE$ = 0;

  endspecify

`endif

endmodule

`endcelldefine
