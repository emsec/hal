`timescale 1 ps/1 ps
module USFFT64_2B(\U_FFT1/s4r(12) ,\U_FFT1/s4r(8) ,\U_FFT1/s4r(16) ,\U_FFT1/s4r(10) ,\U_FFT1/s4r(7) ,\U_FFT1/s4r(3) ,\U_FFT1/s4r(17) ,\U_FFT1/s4r(4) ,\U_FFT1/s4r(15) ,\U_FFT1/s4r(6) ,\U_FFT1/s4r(13) ,\U_FFT1/s4r(0) ,\U_FFT1/s4r(1) ,\U_FFT1/s4r(14) ,\U_FFT1/s4r(9) ,\U_FFT1/s4r(2) ,\U_FFT1/s4r(11) ,\U_FFT1/s4r(5) ,\U_FFT1/UMR/dx5_1(13) ,\U_FFT1/UMR/dx5_1(11) ,\U_FFT1/UMR/dx5_1(19) ,\U_FFT1/UMR/dx5_1(14) ,\U_FFT1/UMR/dx5_1(16) ,\U_FFT1/UMR/dx5_1(9) ,\U_FFT1/UMR/dx5_1(6) ,\U_FFT1/UMR/dx5_1(18) ,\U_FFT1/UMR/dx5_1(15) ,\U_FFT1/UMR/dx5_1(2) ,\U_FFT1/UMR/dx5_1(5) ,\U_FFT1/UMR/dx5_1(7) ,\U_FFT1/UMR/dx5_1(17) ,\U_FFT1/UMR/dx5_1(12) ,\U_FFT1/UMR/dx5_1(10) ,\U_FFT1/UMR/dx5_1(4) ,\U_FFT1/UMR/dx5_1(8) ,\U_FFT1/UMR/dx5_1(3) );
    input \U_FFT1/s4r(12) ;
    input \U_FFT1/s4r(8) ;
    input \U_FFT1/s4r(16) ;
    input \U_FFT1/s4r(10) ;
    input \U_FFT1/s4r(7) ;
    input \U_FFT1/s4r(3) ;
    input \U_FFT1/s4r(17) ;
    input \U_FFT1/s4r(4) ;
    input \U_FFT1/s4r(15) ;
    input \U_FFT1/s4r(6) ;
    input \U_FFT1/s4r(13) ;
    input \U_FFT1/s4r(0) ;
    input \U_FFT1/s4r(1) ;
    input \U_FFT1/s4r(14) ;
    input \U_FFT1/s4r(9) ;
    input \U_FFT1/s4r(2) ;
    input \U_FFT1/s4r(11) ;
    input \U_FFT1/s4r(5) ;
    output \U_FFT1/UMR/dx5_1(13) ;
    output \U_FFT1/UMR/dx5_1(11) ;
    output \U_FFT1/UMR/dx5_1(19) ;
    output \U_FFT1/UMR/dx5_1(14) ;
    output \U_FFT1/UMR/dx5_1(16) ;
    output \U_FFT1/UMR/dx5_1(9) ;
    output \U_FFT1/UMR/dx5_1(6) ;
    output \U_FFT1/UMR/dx5_1(18) ;
    output \U_FFT1/UMR/dx5_1(15) ;
    output \U_FFT1/UMR/dx5_1(2) ;
    output \U_FFT1/UMR/dx5_1(5) ;
    output \U_FFT1/UMR/dx5_1(7) ;
    output \U_FFT1/UMR/dx5_1(17) ;
    output \U_FFT1/UMR/dx5_1(12) ;
    output \U_FFT1/UMR/dx5_1(10) ;
    output \U_FFT1/UMR/dx5_1(4) ;
    output \U_FFT1/UMR/dx5_1(8) ;
    output \U_FFT1/UMR/dx5_1(3) ;
    wire \U_FFT1/UMR/dx5_1_cry_5 ;
    wire \U_FFT1/UMR/dx5_1_cry_16 ;
    wire \U_FFT1/UMR/dx5_1_cry_13 ;
    wire \U_FFT1/UMR/dx5_1_cry_12 ;
    wire \U_FFT1/UMR/dx5_1_cry_9 ;
    wire \U_FFT1/UMR/dx5_1_cry_2 ;
    wire \U_FFT1/UMR/dx5_1_cry_15 ;
    wire \U_FFT1/UMR/dx5_1_cry_14 ;
    wire \U_FFT1/UMR/dx5_1_cry_8 ;
    wire \U_FFT1/UMR/dx5_1_cry_11 ;
    wire \U_FFT1/UMR/dx5_1_cry_17 ;
    wire \U_FFT1/UMR/dx5_1_cry_10 ;
    wire \U_FFT1/UMR/dx5_1_cry_4 ;
    wire \U_FFT1/UMR/dx5_1_cry_7 ;
    wire \U_FFT1/UMR/dx5_1_cry_3 ;
    wire \U_FFT1/UMR/dx5_1_cry_6 ;

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_12_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_11 ),
        .I0(\U_FFT1/s4r(12) ),
        .I1(\U_FFT1/s4r(10) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_12 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_13_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_12 ),
        .I0(\U_FFT1/s4r(13) ),
        .I1(\U_FFT1/s4r(11) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_13 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_10_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_9 ),
        .I0(\U_FFT1/s4r(10) ),
        .I1(\U_FFT1/s4r(8) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_10 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_11_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_10 ),
        .I0(\U_FFT1/s4r(11) ),
        .I1(\U_FFT1/s4r(9) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_11 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_16_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_15 ),
        .I0(\U_FFT1/s4r(16) ),
        .I1(\U_FFT1/s4r(14) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_16 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_17_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_16 ),
        .I0(\U_FFT1/s4r(17) ),
        .I1(\U_FFT1/s4r(15) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_17 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_14_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_13 ),
        .I0(\U_FFT1/s4r(14) ),
        .I1(\U_FFT1/s4r(12) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_14 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_15_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_14 ),
        .I0(\U_FFT1/s4r(15) ),
        .I1(\U_FFT1/s4r(13) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_15 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_18_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_17 ),
        .I0(\U_FFT1/s4r(17) ),
        .I1(\U_FFT1/s4r(16) ),
        .CO(\U_FFT1/UMR/dx5_1(19) )
    );

    HAL_XOR3 _15__11650_RESYNTH_GATE (
        .A(\U_FFT1/s4r(8) ),
        .B(\U_FFT1/UMR/dx5_1_cry_9 ),
        .C(\U_FFT1/s4r(10) ),
        .O(\U_FFT1/UMR/dx5_1(10) )
    );

    HAL_XOR3 _15__11651_RESYNTH_GATE (
        .A(\U_FFT1/s4r(7) ),
        .B(\U_FFT1/UMR/dx5_1_cry_8 ),
        .C(\U_FFT1/s4r(9) ),
        .O(\U_FFT1/UMR/dx5_1(9) )
    );

    HAL_XOR3 _15__11642_RESYNTH_GATE (
        .A(\U_FFT1/s4r(16) ),
        .B(\U_FFT1/UMR/dx5_1_cry_17 ),
        .C(\U_FFT1/s4r(17) ),
        .O(\U_FFT1/UMR/dx5_1(18) )
    );

    HAL_XOR3 _15__11648_RESYNTH_GATE (
        .A(\U_FFT1/s4r(10) ),
        .B(\U_FFT1/UMR/dx5_1_cry_11 ),
        .C(\U_FFT1/s4r(12) ),
        .O(\U_FFT1/UMR/dx5_1(12) )
    );

    HAL_XOR3 _15__11643_RESYNTH_GATE (
        .A(\U_FFT1/s4r(15) ),
        .B(\U_FFT1/UMR/dx5_1_cry_16 ),
        .C(\U_FFT1/s4r(17) ),
        .O(\U_FFT1/UMR/dx5_1(17) )
    );

    HAL_XOR3 _15__11649_RESYNTH_GATE (
        .A(\U_FFT1/s4r(9) ),
        .B(\U_FFT1/UMR/dx5_1_cry_10 ),
        .C(\U_FFT1/s4r(11) ),
        .O(\U_FFT1/UMR/dx5_1(11) )
    );

    HAL_XOR3 _15__11654_RESYNTH_GATE (
        .A(\U_FFT1/s4r(4) ),
        .B(\U_FFT1/UMR/dx5_1_cry_5 ),
        .C(\U_FFT1/s4r(6) ),
        .O(\U_FFT1/UMR/dx5_1(6) )
    );

    HAL_XOR3 _15__11655_RESYNTH_GATE (
        .A(\U_FFT1/s4r(3) ),
        .B(\U_FFT1/UMR/dx5_1_cry_4 ),
        .C(\U_FFT1/s4r(5) ),
        .O(\U_FFT1/UMR/dx5_1(5) )
    );

    HAL_XOR3 _15__11652_RESYNTH_GATE (
        .A(\U_FFT1/s4r(6) ),
        .B(\U_FFT1/UMR/dx5_1_cry_7 ),
        .C(\U_FFT1/s4r(8) ),
        .O(\U_FFT1/UMR/dx5_1(8) )
    );

    HAL_XOR3 _15__11646_RESYNTH_GATE (
        .A(\U_FFT1/s4r(12) ),
        .B(\U_FFT1/UMR/dx5_1_cry_13 ),
        .C(\U_FFT1/s4r(14) ),
        .O(\U_FFT1/UMR/dx5_1(14) )
    );

    HAL_XOR3 _15__11653_RESYNTH_GATE (
        .A(\U_FFT1/s4r(5) ),
        .B(\U_FFT1/UMR/dx5_1_cry_6 ),
        .C(\U_FFT1/s4r(7) ),
        .O(\U_FFT1/UMR/dx5_1(7) )
    );

    HAL_XOR3 _15__11647_RESYNTH_GATE (
        .A(\U_FFT1/s4r(11) ),
        .B(\U_FFT1/UMR/dx5_1_cry_12 ),
        .C(\U_FFT1/s4r(13) ),
        .O(\U_FFT1/UMR/dx5_1(13) )
    );

    HAL_XOR3 _15__11644_RESYNTH_GATE (
        .A(\U_FFT1/s4r(14) ),
        .B(\U_FFT1/UMR/dx5_1_cry_15 ),
        .C(\U_FFT1/s4r(16) ),
        .O(\U_FFT1/UMR/dx5_1(16) )
    );

    HAL_XOR3 _15__11645_RESYNTH_GATE (
        .A(\U_FFT1/s4r(13) ),
        .B(\U_FFT1/UMR/dx5_1_cry_14 ),
        .C(\U_FFT1/s4r(15) ),
        .O(\U_FFT1/UMR/dx5_1(15) )
    );

    HAL_XOR3 _15__11656_RESYNTH_GATE (
        .A(\U_FFT1/s4r(2) ),
        .B(\U_FFT1/UMR/dx5_1_cry_3 ),
        .C(\U_FFT1/s4r(4) ),
        .O(\U_FFT1/UMR/dx5_1(4) )
    );

    HAL_XOR3 _15__11657_RESYNTH_GATE (
        .A(\U_FFT1/s4r(1) ),
        .B(\U_FFT1/UMR/dx5_1_cry_2 ),
        .C(\U_FFT1/s4r(3) ),
        .O(\U_FFT1/UMR/dx5_1(3) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_4_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_3 ),
        .I0(\U_FFT1/s4r(4) ),
        .I1(\U_FFT1/s4r(2) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_4 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_5_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_4 ),
        .I0(\U_FFT1/s4r(5) ),
        .I1(\U_FFT1/s4r(3) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_5 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_2_c  (
        .CI(1'b0 ),
        .I0(\U_FFT1/s4r(2) ),
        .I1(\U_FFT1/s4r(0) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_2 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_3_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_2 ),
        .I0(\U_FFT1/s4r(3) ),
        .I1(\U_FFT1/s4r(1) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_3 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_8_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_7 ),
        .I0(\U_FFT1/s4r(8) ),
        .I1(\U_FFT1/s4r(6) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_8 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_9_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_8 ),
        .I0(\U_FFT1/s4r(9) ),
        .I1(\U_FFT1/s4r(7) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_9 )
    );

    HAL_XOR2 _7__11598_RESYNTH_GATE (
        .A(\U_FFT1/s4r(2) ),
        .B(\U_FFT1/s4r(0) ),
        .O(\U_FFT1/UMR/dx5_1(2) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_6_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_5 ),
        .I0(\U_FFT1/s4r(6) ),
        .I1(\U_FFT1/s4r(4) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_6 )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_7_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_6 ),
        .I0(\U_FFT1/s4r(7) ),
        .I1(\U_FFT1/s4r(5) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_7 )
    );
endmodule

