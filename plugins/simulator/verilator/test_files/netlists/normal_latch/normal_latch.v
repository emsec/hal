module weird_latch
(
        input D,
        input G,
        input CD,
        output Q,
        output QN
);
wire D;
wire G;
wire CD;
wire Q;
wire QN;

LD3 LD3_inst (
         .D(D),
         .G(G),
         .CD(CD),
         .Q(Q),
         .QN(QN)
     );

endmodule