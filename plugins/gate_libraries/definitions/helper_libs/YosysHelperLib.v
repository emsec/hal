// GND
module GND (Y);
output Y;
assign Y = 1'b0;
endmodule

// VCC
module VCC (Y);
output Y;
assign Y = 1'b1;
endmodule

// BUF
module HAL_BUF (A, O);
input A;
output O;
assign O = A;
endmodule

// INV
module HAL_INV (A, O);
input A;
output O;
assign O = ~A;
endmodule

// AND2
module HAL_AND2 (A, B, O);
input A, B;
output O;
assign O = A & B;
endmodule

// AND3
module HAL_AND3 (A, B, C, O);
input A, B, C;
output O;
assign O = A & B & C;
endmodule

// AND4
module HAL_AND4 (A, B, C, D, O);
input A, B, C, D;
output O;
assign O = A & B & C & D;
endmodule

// OR2
module HAL_OR2 (A, B, O);
input A, B;
output O;
assign O = A | B;
endmodule

// OR3
module HAL_OR3 (A, B, C, O);
input A, B, C;
output O;
assign O = A | B | C;
endmodule

// OR4
module HAL_OR4 (A, B, C, D, O);
input A, B, C, D;
output O;
assign O = A | B | C | D;
endmodule

// XOR2
module HAL_XOR2 (A, B, O);
input A, B;
output O;
assign O = A ^ B;
endmodule

// XOR3
module HAL_XOR3 (A, B, C, O);
input A, B, C;
output O;
assign O = A ^ B ^ C;
endmodule

// XOR4
module HAL_XOR4 (A, B, C, D, O);
input A, B, C, D;
output O;
assign O = A ^ B ^ C ^ D;
endmodule

// XNOR2
module HAL_XNOR2 (A, B, O);
input A, B;
output O;
assign O = (! (A ^ B));
endmodule

// XNOR3
module HAL_XNOR3 (A, B, C, O);
input A, B, C;
output O;
assign O =(! (A ^ (B ^ C)));
endmodule

// XNOR4
module HAL_XNOR4 (A, B, C, D, O);
input A, B, C, D;
output O;
assign O = (! (A ^ (B ^ (C ^ D))));
endmodule

module HAL_MUX (A, B, S, O);
input A, B, S;
output O;
assign O = ((A & S) | (B & (! S)));
endmodule

module HAL_MUX3 (A, B, C, S1, S2, O);
input A, B, C, S1, S2;
output O;
assign O = ((A & S1) | (!S1 & ((B & S2) | (C & !S2))));
endmodule

module HAL_MUX4 (A, B, C, D, S1, S2, O);
input A, B, C, D, S1, S2;
output O;
assign O = ((A & S1 & S2) | (B & S1 & !S2) | (C & !S1 & S2) | (D & !S1 & !S2));
endmodule


// LATTICE


// SB_GB
module SB_GB (USER_SIGNAL_TO_GLOBAL_BUFFER, GLOBAL_BUFFER_OUTPUT);
input USER_SIGNAL_TO_GLOBAL_BUFFER;
output GLOBAL_BUFFER_OUTPUT;
assign GLOBAL_BUFFER_OUTPUT = USER_SIGNAL_TO_GLOBAL_BUFFER;
endmodule
