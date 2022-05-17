module complex_ff
(
        input D,
        input CP,
        input CD,
        input SD,
        output Q,
        output QN
);
wire D;
wire CP;
wire CD;
wire SD;
wire Q;
wire QN;

FD3 FD3_inst (
         .CP(CP),
         .CD(CD),
         .SD(SD),
         .D(D),
         .Q(Q),
         .QN(QN)
     );

endmodule