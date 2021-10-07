module weird_latch
(
        input S,
        input R,
        output Q,
        output QN
);
wire S;
wire R;
wire Q;
wire QN;

LSR0 LSR0_inst (
         .R(R),
         .S(S),
         .Q(Q),
         .QN(QN)
     );

endmodule