`timescale 1 ps/1 ps
module USFFT64_2B(\U_FFT2/di(17) ,\U_FFT2/d4i(17) ,\U_FFT2/d4i(3) ,\U_FFT2/di(16) ,\U_FFT2/di(12) ,\U_FFT2/d4i(8) ,\U_FFT2/d4i(4) ,\U_FFT2/d4i(10) ,\U_FFT2/di(6) ,\U_FFT2/di(13) ,\U_FFT2/di(0) ,\U_FFT2/d4i(0) ,\U_FFT2/d4i(14) ,\U_FFT2/di(8) ,\U_FFT2/d4i(1) ,\U_FFT2/d4i(11) ,\U_FFT2/d4i(7) ,\U_FFT2/di(7) ,\U_FFT2/d4i(16) ,\U_FFT2/di(4) ,\U_FFT2/d4i(13) ,\U_FFT2/di(15) ,\U_FFT2/di(11) ,\U_FFT2/di(1) ,\U_FFT2/di(9) ,\U_FFT2/d4i(5) ,\U_FFT2/d4i(2) ,\U_FFT2/di(5) ,\U_FFT2/d4i(6) ,\U_FFT2/di(14) ,\U_FFT2/di(10) ,\U_FFT2/d4i(12) ,\U_FFT2/di(3) ,\U_FFT2/d4i(9) ,\U_FFT2/di(2) ,\U_FFT2/d4i(15) ,\U_FFT2/s2i0(0) ,\U_FFT2/s2i0(8) ,\U_FFT2/s2i0(11) ,\U_FFT2/s2i0(9) ,\U_FFT2/s2i0(16) ,\U_FFT2/s2i0(17) ,\U_FFT2/s2i0(18) ,\U_FFT2/s2i0(4) ,\U_FFT2/s2i0(15) ,\U_FFT2/s2i0(6) ,\U_FFT2/s2i0(12) ,\U_FFT2/s2i0(2) ,\U_FFT2/s2i0(14) ,\U_FFT2/s2i0(10) ,\U_FFT2/s2i0(7) ,\U_FFT2/s2i0(5) ,\U_FFT2/s2i0(3) ,\U_FFT2/s2i0(1) ,\U_FFT2/s2i0(13) );
    input \U_FFT2/di(17) ;
    input \U_FFT2/d4i(17) ;
    input \U_FFT2/d4i(3) ;
    input \U_FFT2/di(16) ;
    input \U_FFT2/di(12) ;
    input \U_FFT2/d4i(8) ;
    input \U_FFT2/d4i(4) ;
    input \U_FFT2/d4i(10) ;
    input \U_FFT2/di(6) ;
    input \U_FFT2/di(13) ;
    input \U_FFT2/di(0) ;
    input \U_FFT2/d4i(0) ;
    input \U_FFT2/d4i(14) ;
    input \U_FFT2/di(8) ;
    input \U_FFT2/d4i(1) ;
    input \U_FFT2/d4i(11) ;
    input \U_FFT2/d4i(7) ;
    input \U_FFT2/di(7) ;
    input \U_FFT2/d4i(16) ;
    input \U_FFT2/di(4) ;
    input \U_FFT2/d4i(13) ;
    input \U_FFT2/di(15) ;
    input \U_FFT2/di(11) ;
    input \U_FFT2/di(1) ;
    input \U_FFT2/di(9) ;
    input \U_FFT2/d4i(5) ;
    input \U_FFT2/d4i(2) ;
    input \U_FFT2/di(5) ;
    input \U_FFT2/d4i(6) ;
    input \U_FFT2/di(14) ;
    input \U_FFT2/di(10) ;
    input \U_FFT2/d4i(12) ;
    input \U_FFT2/di(3) ;
    input \U_FFT2/d4i(9) ;
    input \U_FFT2/di(2) ;
    input \U_FFT2/d4i(15) ;
    output \U_FFT2/s2i0(0) ;
    output \U_FFT2/s2i0(8) ;
    output \U_FFT2/s2i0(11) ;
    output \U_FFT2/s2i0(9) ;
    output \U_FFT2/s2i0(16) ;
    output \U_FFT2/s2i0(17) ;
    output \U_FFT2/s2i0(18) ;
    output \U_FFT2/s2i0(4) ;
    output \U_FFT2/s2i0(15) ;
    output \U_FFT2/s2i0(6) ;
    output \U_FFT2/s2i0(12) ;
    output \U_FFT2/s2i0(2) ;
    output \U_FFT2/s2i0(14) ;
    output \U_FFT2/s2i0(10) ;
    output \U_FFT2/s2i0(7) ;
    output \U_FFT2/s2i0(5) ;
    output \U_FFT2/s2i0(3) ;
    output \U_FFT2/s2i0(1) ;
    output \U_FFT2/s2i0(13) ;
    wire \s2i[15]_i_5__0_n_0 ;
    wire \s2i_reg[11]_i_1__0_n_3 ;
    wire \s2i_reg[18]_i_1_n_3 ;
    wire \s2i[7]_i_4__0_n_0 ;
    wire \s2i_reg[11]_i_1__0_n_1 ;
    wire \s2i[11]_i_4__0_n_0 ;
    wire \s2i[11]_i_3__0_n_0 ;
    wire \s2i_reg[15]_i_1__0_n_0 ;
    wire \s2i_reg[7]_i_1__0_n_0 ;
    wire \s2i_reg[15]_i_1__0_n_2 ;
    wire \s2i_reg[15]_i_1__0_n_3 ;
    wire \s2i[3]_i_5__0_n_0 ;
    wire \s2i[15]_i_4__0_n_0 ;
    wire \s2i[11]_i_5__0_n_0 ;
    wire \s2i_reg[3]_i_1__0_n_1 ;
    wire \s2i[18]_i_2_n_0 ;
    wire \s2i_reg[7]_i_1__0_n_1 ;
    wire \s2i[11]_i_2__0_n_0 ;
    wire \s2i_reg[11]_i_1__0_n_2 ;
    wire \s2i[15]_i_2__0_n_0 ;
    wire \s2i[7]_i_5__0_n_0 ;
    wire \s2i[15]_i_3__0_n_0 ;
    wire \s2i_reg[11]_i_1__0_n_0 ;
    wire \s2i_reg[15]_i_1__0_n_1 ;
    wire \s2i_reg[18]_i_1_n_2 ;
    wire \s2i_reg[3]_i_1__0_n_3 ;
    wire \s2i[7]_i_2__0_n_0 ;
    wire \s2i[3]_i_2__0_n_0 ;
    wire \s2i_reg[7]_i_1__0_n_3 ;
    wire \s2i[7]_i_3__0_n_0 ;
    wire \s2i_reg[7]_i_1__0_n_2 ;
    wire \s2i[3]_i_3__0_n_0 ;
    wire \s2i_reg[3]_i_1__0_n_0 ;
    wire \s2i[3]_i_4__0_n_0 ;
    wire \s2i[18]_i_3_n_0 ;
    wire \s2i_reg[3]_i_1__0_n_2 ;

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[18]_i_2  (
        .I0(\U_FFT2/d4i(17) ),
        .I1(\U_FFT2/di(17) ),
        .O(\s2i[18]_i_2_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[15]_i_5__0  (
        .I0(\U_FFT2/d4i(12) ),
        .I1(\U_FFT2/di(12) ),
        .O(\s2i[15]_i_5__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[15]_i_4__0  (
        .I0(\U_FFT2/d4i(13) ),
        .I1(\U_FFT2/di(13) ),
        .O(\s2i[15]_i_4__0_n_0 )
    );

    CARRY4 \s2i_reg[11]_i_1__0  (
        .CI(\s2i_reg[7]_i_1__0_n_0 ),
        .CYINIT(1'b0 ),
        .DI({\U_FFT2/d4i(11) ,
\U_FFT2/d4i(10) ,
\U_FFT2/d4i(9) ,
\U_FFT2/d4i(8) }),
        .S({\s2i[11]_i_2__0_n_0 ,
\s2i[11]_i_3__0_n_0 ,
\s2i[11]_i_4__0_n_0 ,
\s2i[11]_i_5__0_n_0 }),
        .CO({\s2i_reg[11]_i_1__0_n_0 ,
\s2i_reg[11]_i_1__0_n_1 ,
\s2i_reg[11]_i_1__0_n_2 ,
\s2i_reg[11]_i_1__0_n_3 }),
        .O({\U_FFT2/s2i0(11) ,
\U_FFT2/s2i0(10) ,
\U_FFT2/s2i0(9) ,
\U_FFT2/s2i0(8) })
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[7]_i_5__0  (
        .I0(\U_FFT2/d4i(4) ),
        .I1(\U_FFT2/di(4) ),
        .O(\s2i[7]_i_5__0_n_0 )
    );

    CARRY4 \s2i_reg[18]_i_1  (
        .CI(\s2i_reg[15]_i_1__0_n_0 ),
        .CYINIT(1'b0 ),
        .DI({1'b0 ,
1'b0 ,
\U_FFT2/di(17) ,
\U_FFT2/d4i(16) }),
        .S({1'b0 ,
1'b1 ,
\s2i[18]_i_2_n_0 ,
\s2i[18]_i_3_n_0 }),
        .CO({1'bz,
1'bz,
\s2i_reg[18]_i_1_n_2 ,
\s2i_reg[18]_i_1_n_3 }),
        .O({1'bz,
\U_FFT2/s2i0(18) ,
\U_FFT2/s2i0(17) ,
\U_FFT2/s2i0(16) })
    );

    CARRY4 \s2i_reg[15]_i_1__0  (
        .CI(\s2i_reg[11]_i_1__0_n_0 ),
        .CYINIT(1'b0 ),
        .DI({\U_FFT2/d4i(15) ,
\U_FFT2/d4i(14) ,
\U_FFT2/d4i(13) ,
\U_FFT2/d4i(12) }),
        .S({\s2i[15]_i_2__0_n_0 ,
\s2i[15]_i_3__0_n_0 ,
\s2i[15]_i_4__0_n_0 ,
\s2i[15]_i_5__0_n_0 }),
        .CO({\s2i_reg[15]_i_1__0_n_0 ,
\s2i_reg[15]_i_1__0_n_1 ,
\s2i_reg[15]_i_1__0_n_2 ,
\s2i_reg[15]_i_1__0_n_3 }),
        .O({\U_FFT2/s2i0(15) ,
\U_FFT2/s2i0(14) ,
\U_FFT2/s2i0(13) ,
\U_FFT2/s2i0(12) })
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[7]_i_2__0  (
        .I0(\U_FFT2/d4i(7) ),
        .I1(\U_FFT2/di(7) ),
        .O(\s2i[7]_i_2__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[3]_i_5__0  (
        .I0(\U_FFT2/d4i(0) ),
        .I1(\U_FFT2/di(0) ),
        .O(\s2i[3]_i_5__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[7]_i_4__0  (
        .I0(\U_FFT2/d4i(5) ),
        .I1(\U_FFT2/di(5) ),
        .O(\s2i[7]_i_4__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[7]_i_3__0  (
        .I0(\U_FFT2/d4i(6) ),
        .I1(\U_FFT2/di(6) ),
        .O(\s2i[7]_i_3__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[3]_i_2__0  (
        .I0(\U_FFT2/d4i(3) ),
        .I1(\U_FFT2/di(3) ),
        .O(\s2i[3]_i_2__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[18]_i_3  (
        .I0(\U_FFT2/d4i(16) ),
        .I1(\U_FFT2/di(16) ),
        .O(\s2i[18]_i_3_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[11]_i_3__0  (
        .I0(\U_FFT2/d4i(10) ),
        .I1(\U_FFT2/di(10) ),
        .O(\s2i[11]_i_3__0_n_0 )
    );

    CARRY4 \s2i_reg[7]_i_1__0  (
        .CI(\s2i_reg[3]_i_1__0_n_0 ),
        .CYINIT(1'b0 ),
        .DI({\U_FFT2/d4i(7) ,
\U_FFT2/d4i(6) ,
\U_FFT2/d4i(5) ,
\U_FFT2/d4i(4) }),
        .S({\s2i[7]_i_2__0_n_0 ,
\s2i[7]_i_3__0_n_0 ,
\s2i[7]_i_4__0_n_0 ,
\s2i[7]_i_5__0_n_0 }),
        .CO({\s2i_reg[7]_i_1__0_n_0 ,
\s2i_reg[7]_i_1__0_n_1 ,
\s2i_reg[7]_i_1__0_n_2 ,
\s2i_reg[7]_i_1__0_n_3 }),
        .O({\U_FFT2/s2i0(7) ,
\U_FFT2/s2i0(6) ,
\U_FFT2/s2i0(5) ,
\U_FFT2/s2i0(4) })
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[11]_i_2__0  (
        .I0(\U_FFT2/d4i(11) ),
        .I1(\U_FFT2/di(11) ),
        .O(\s2i[11]_i_2__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[3]_i_4__0  (
        .I0(\U_FFT2/d4i(1) ),
        .I1(\U_FFT2/di(1) ),
        .O(\s2i[3]_i_4__0_n_0 )
    );

    CARRY4 \s2i_reg[3]_i_1__0  (
        .CI(1'b0 ),
        .CYINIT(1'b1 ),
        .DI({\U_FFT2/d4i(3) ,
\U_FFT2/d4i(2) ,
\U_FFT2/d4i(1) ,
\U_FFT2/d4i(0) }),
        .S({\s2i[3]_i_2__0_n_0 ,
\s2i[3]_i_3__0_n_0 ,
\s2i[3]_i_4__0_n_0 ,
\s2i[3]_i_5__0_n_0 }),
        .CO({\s2i_reg[3]_i_1__0_n_0 ,
\s2i_reg[3]_i_1__0_n_1 ,
\s2i_reg[3]_i_1__0_n_2 ,
\s2i_reg[3]_i_1__0_n_3 }),
        .O({\U_FFT2/s2i0(3) ,
\U_FFT2/s2i0(2) ,
\U_FFT2/s2i0(1) ,
\U_FFT2/s2i0(0) })
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[11]_i_5__0  (
        .I0(\U_FFT2/d4i(8) ),
        .I1(\U_FFT2/di(8) ),
        .O(\s2i[11]_i_5__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[3]_i_3__0  (
        .I0(\U_FFT2/d4i(2) ),
        .I1(\U_FFT2/di(2) ),
        .O(\s2i[3]_i_3__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[11]_i_4__0  (
        .I0(\U_FFT2/d4i(9) ),
        .I1(\U_FFT2/di(9) ),
        .O(\s2i[11]_i_4__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[15]_i_3__0  (
        .I0(\U_FFT2/d4i(14) ),
        .I1(\U_FFT2/di(14) ),
        .O(\s2i[15]_i_3__0_n_0 )
    );

    LUT2 #(
        .INIT(4'h9)
    ) \s2i[15]_i_2__0  (
        .I0(\U_FFT2/d4i(15) ),
        .I1(\U_FFT2/di(15) ),
        .O(\s2i[15]_i_2__0_n_0 )
    );
endmodule

