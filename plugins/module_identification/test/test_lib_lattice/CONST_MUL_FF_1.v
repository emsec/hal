`timescale 1 ps/1 ps
module USFFT64_2B(\s4r_RNO_0(15) ,\s4r_RNO_0(14) ,\s4r_RNO_0(9) ,\s4r_RNO_0(17) ,\U_FFT1/em_g ,\s4r_RNO_0(7) ,\top_module/CLK_c_g ,\s4r_RNO_0(6) ,\s4r_RNO_0(12) ,\s4r_RNO_0(0) ,\s4r_RNO_0(4) ,\s4r_RNO_0(11) ,\U_FFT1/un1_dr3_2_0 ,\s4r_RNO_0(2) ,\s4r_RNO_0(8) ,\s4r_RNO_0(13) ,\s4r_RNO_0(3) ,\s4r_RNO_0(16) ,\s4r_RNO_0(10) ,\s4r_RNO_0(1) ,\s4r_RNO_0(5) ,\U_FFT1/UMR/dx5(13) ,\U_FFT1/UMR/dx5(8) ,\U_FFT1/UMR/dx5(6) ,\U_FFT1/UMR/dx5(3) ,\U_FFT1/UMR/dx5(10) ,\U_FFT1/UMR/dx5(4) ,\U_FFT1/UMR/dx5(14) ,\U_FFT1/UMR/dx5(11) ,\U_FFT1/UMR/dx5(5) ,\U_FFT1/UMR/dx5(1) ,\U_FFT1/UMR/dx5(17) ,\U_FFT1/UMR/dx5(9) ,\U_FFT1/UMR/dx5(19) ,\U_FFT1/UMR/dx5(12) ,\U_FFT1/UMR/dx5(18) ,\U_FFT1/UMR/dx5(0) ,\U_FFT1/UMR/dx5(16) ,\U_FFT1/UMR/dx5(7) ,\U_FFT1/UMR/dx5(2) ,\U_FFT1/UMR/dx5(15) );
    input \s4r_RNO_0(15) ;
    input \s4r_RNO_0(14) ;
    input \s4r_RNO_0(9) ;
    input \s4r_RNO_0(17) ;
    input \U_FFT1/em_g ;
    input \s4r_RNO_0(7) ;
    input \top_module/CLK_c_g ;
    input \s4r_RNO_0(6) ;
    input \s4r_RNO_0(12) ;
    input \s4r_RNO_0(0) ;
    input \s4r_RNO_0(4) ;
    input \s4r_RNO_0(11) ;
    input \U_FFT1/un1_dr3_2_0 ;
    input \s4r_RNO_0(2) ;
    input \s4r_RNO_0(8) ;
    input \s4r_RNO_0(13) ;
    input \s4r_RNO_0(3) ;
    input \s4r_RNO_0(16) ;
    input \s4r_RNO_0(10) ;
    input \s4r_RNO_0(1) ;
    input \s4r_RNO_0(5) ;
    output \U_FFT1/UMR/dx5(13) ;
    output \U_FFT1/UMR/dx5(8) ;
    output \U_FFT1/UMR/dx5(6) ;
    output \U_FFT1/UMR/dx5(3) ;
    output \U_FFT1/UMR/dx5(10) ;
    output \U_FFT1/UMR/dx5(4) ;
    output \U_FFT1/UMR/dx5(14) ;
    output \U_FFT1/UMR/dx5(11) ;
    output \U_FFT1/UMR/dx5(5) ;
    output \U_FFT1/UMR/dx5(1) ;
    output \U_FFT1/UMR/dx5(17) ;
    output \U_FFT1/UMR/dx5(9) ;
    output \U_FFT1/UMR/dx5(19) ;
    output \U_FFT1/UMR/dx5(12) ;
    output \U_FFT1/UMR/dx5(18) ;
    output \U_FFT1/UMR/dx5(0) ;
    output \U_FFT1/UMR/dx5(16) ;
    output \U_FFT1/UMR/dx5(7) ;
    output \U_FFT1/UMR/dx5(2) ;
    output \U_FFT1/UMR/dx5(15) ;
    wire \U_FFT1/UMR/dx5_1_cry_11 ;
    wire \U_FFT1/UMR/dx5_1_cry_5 ;
    wire \U_FFT1/s4r(12) ;
    wire \U_FFT1/s4r(1) ;
    wire \U_FFT1/UMR/dx5_1_cry_4 ;
    wire \U_FFT1/UMR/dx5_1(8) ;
    wire \U_FFT1/UMR/dx5_1(12) ;
    wire \U_FFT1/s4r(0) ;
    wire \U_FFT1/s4r(9) ;
    wire \U_FFT1/s4r(11) ;
    wire \U_FFT1/s4r(14) ;
    wire \U_FFT1/s4r(4) ;
    wire \U_FFT1/UMR/dx5_1(6) ;
    wire \U_FFT1/UMR/dx5_1(15) ;
    wire \U_FFT1/s4r(3) ;
    wire \U_FFT1/s4r(5) ;
    wire \U_FFT1/s4r(6) ;
    wire \U_FFT1/UMR/dx5_1_cry_17 ;
    wire \U_FFT1/s4r(8) ;
    wire \U_FFT1/s4r(2) ;
    wire \U_FFT1/UMR/dx5_1_cry_9 ;
    wire \U_FFT1/UMR/dx5_1(7) ;
    wire \U_FFT1/UMR/dx5_1_cry_3 ;
    wire \U_FFT1/UMR/dx5_1(4) ;
    wire \U_FFT1/UMR/dx5_1_cry_2 ;
    wire \U_FFT1/UMR/dx5_1(11) ;
    wire \U_FFT1/UMR/dx5_1(3) ;
    wire \U_FFT1/s4r(10) ;
    wire \U_FFT1/UMR/dx5_1_cry_8 ;
    wire \U_FFT1/UMR/dx5_1(18) ;
    wire \U_FFT1/UMR/dx5_1(10) ;
    wire \U_FFT1/UMR/dx5_1_cry_6 ;
    wire \U_FFT1/UMR/dx5_1(9) ;
    wire \U_FFT1/UMR/dx5_1_cry_7 ;
    wire \U_FFT1/UMR/dx5_1(5) ;
    wire \U_FFT1/UMR/dx5_1(14) ;
    wire \U_FFT1/UMR/dx5_1_cry_10 ;
    wire \U_FFT1/UMR/dx5_1(13) ;
    wire \U_FFT1/s4r(7) ;
    wire \U_FFT1/s4r(15) ;
    wire \U_FFT1/UMR/dx5_1(16) ;
    wire \U_FFT1/UMR/dx5_1(17) ;
    wire \U_FFT1/UMR/dx5_1(19) ;
    wire \U_FFT1/s4r(16) ;
    wire \U_FFT1/s4r(17) ;
    wire \U_FFT1/UMR/dx5_1_cry_16 ;
    wire \U_FFT1/UMR/dx5_1_cry_13 ;
    wire \U_FFT1/s4r(13) ;
    wire \U_FFT1/UMR/dx5_1_cry_14 ;
    wire \U_FFT1/UMR/dx5_1(2) ;
    wire \U_FFT1/UMR/dx5_1_cry_12 ;
    wire \U_FFT1/UMR/dx5_1_cry_15 ;

    HAL_XOR3 _15__11648_RESYNTH_GATE (
        .A(\U_FFT1/s4r(10) ),
        .B(\U_FFT1/UMR/dx5_1_cry_11 ),
        .C(\U_FFT1/s4r(12) ),
        .O(\U_FFT1/UMR/dx5_1(12) )
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

    HAL_XOR3 _15__11653_RESYNTH_GATE (
        .A(\U_FFT1/s4r(5) ),
        .B(\U_FFT1/UMR/dx5_1_cry_6 ),
        .C(\U_FFT1/s4r(7) ),
        .O(\U_FFT1/UMR/dx5_1(7) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_4_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_3 ),
        .I0(\U_FFT1/s4r(4) ),
        .I1(\U_FFT1/s4r(2) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_4 )
    );

    HAL_XOR3 _15__11656_RESYNTH_GATE (
        .A(\U_FFT1/s4r(2) ),
        .B(\U_FFT1/UMR/dx5_1_cry_3 ),
        .C(\U_FFT1/s4r(4) ),
        .O(\U_FFT1/UMR/dx5_1(4) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_5_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_4 ),
        .I0(\U_FFT1/s4r(5) ),
        .I1(\U_FFT1/s4r(3) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_5 )
    );

    HAL_XOR3 _15__11657_RESYNTH_GATE (
        .A(\U_FFT1/s4r(1) ),
        .B(\U_FFT1/UMR/dx5_1_cry_2 ),
        .C(\U_FFT1/s4r(3) ),
        .O(\U_FFT1/UMR/dx5_1(3) )
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

    SB_DFFE \U_FFT1/UMR/dx5_Z[5]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(5) ),
        .Q(\U_FFT1/UMR/dx5(5) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[6]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(6) ),
        .Q(\U_FFT1/UMR/dx5(6) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[3]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(3) ),
        .Q(\U_FFT1/UMR/dx5(3) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[4]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(4) ),
        .Q(\U_FFT1/UMR/dx5(4) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[9]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(9) ),
        .Q(\U_FFT1/UMR/dx5(9) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[10]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(10) ),
        .Q(\U_FFT1/UMR/dx5(10) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[7]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(7) ),
        .Q(\U_FFT1/UMR/dx5(7) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[8]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(8) ),
        .Q(\U_FFT1/UMR/dx5(8) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[13]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(13) ),
        .Q(\U_FFT1/UMR/dx5(13) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[14]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(14) ),
        .Q(\U_FFT1/UMR/dx5(14) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[11]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(11) ),
        .Q(\U_FFT1/UMR/dx5(11) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[12]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(12) ),
        .Q(\U_FFT1/UMR/dx5(12) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[17]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(17) ),
        .Q(\U_FFT1/UMR/dx5(17) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[18]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(18) ),
        .Q(\U_FFT1/UMR/dx5(18) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[15]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(15) ),
        .Q(\U_FFT1/UMR/dx5(15) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[16]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(16) ),
        .Q(\U_FFT1/UMR/dx5(16) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[19]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(19) ),
        .Q(\U_FFT1/UMR/dx5(19) )
    );

    HAL_XOR3 _15__11642_RESYNTH_GATE (
        .A(\U_FFT1/s4r(16) ),
        .B(\U_FFT1/UMR/dx5_1_cry_17 ),
        .C(\U_FFT1/s4r(17) ),
        .O(\U_FFT1/UMR/dx5_1(18) )
    );

    HAL_XOR3 _15__11643_RESYNTH_GATE (
        .A(\U_FFT1/s4r(15) ),
        .B(\U_FFT1/UMR/dx5_1_cry_16 ),
        .C(\U_FFT1/s4r(17) ),
        .O(\U_FFT1/UMR/dx5_1(17) )
    );

    HAL_XOR3 _15__11646_RESYNTH_GATE (
        .A(\U_FFT1/s4r(12) ),
        .B(\U_FFT1/UMR/dx5_1_cry_13 ),
        .C(\U_FFT1/s4r(14) ),
        .O(\U_FFT1/UMR/dx5_1(14) )
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

    HAL_XOR2 _7__11598_RESYNTH_GATE (
        .A(\U_FFT1/s4r(2) ),
        .B(\U_FFT1/s4r(0) ),
        .O(\U_FFT1/UMR/dx5_1(2) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[1]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/s4r(1) ),
        .Q(\U_FFT1/UMR/dx5(1) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[2]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/UMR/dx5_1(2) ),
        .Q(\U_FFT1/UMR/dx5(2) )
    );

    SB_DFFE \U_FFT1/UMR/dx5_Z[0]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/em_g ),
        .D(\U_FFT1/s4r(0) ),
        .Q(\U_FFT1/UMR/dx5(0) )
    );

    SB_DFFE \U_FFT1/s4r_Z[0]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(0) ),
        .Q(\U_FFT1/s4r(0) )
    );

    SB_DFFE \U_FFT1/s4r_Z[17]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(17) ),
        .Q(\U_FFT1/s4r(17) )
    );

    SB_DFFE \U_FFT1/s4r_Z[3]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(3) ),
        .Q(\U_FFT1/s4r(3) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_12_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_11 ),
        .I0(\U_FFT1/s4r(12) ),
        .I1(\U_FFT1/s4r(10) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_12 )
    );

    SB_DFFE \U_FFT1/s4r_Z[4]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(4) ),
        .Q(\U_FFT1/s4r(4) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_13_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_12 ),
        .I0(\U_FFT1/s4r(13) ),
        .I1(\U_FFT1/s4r(11) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_13 )
    );

    SB_DFFE \U_FFT1/s4r_Z[1]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(1) ),
        .Q(\U_FFT1/s4r(1) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_10_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_9 ),
        .I0(\U_FFT1/s4r(10) ),
        .I1(\U_FFT1/s4r(8) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_10 )
    );

    SB_DFFE \U_FFT1/s4r_Z[2]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(2) ),
        .Q(\U_FFT1/s4r(2) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_11_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_10 ),
        .I0(\U_FFT1/s4r(11) ),
        .I1(\U_FFT1/s4r(9) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_11 )
    );

    SB_DFFE \U_FFT1/s4r_Z[7]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(7) ),
        .Q(\U_FFT1/s4r(7) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_16_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_15 ),
        .I0(\U_FFT1/s4r(16) ),
        .I1(\U_FFT1/s4r(14) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_16 )
    );

    SB_DFFE \U_FFT1/s4r_Z[8]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(8) ),
        .Q(\U_FFT1/s4r(8) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_17_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_16 ),
        .I0(\U_FFT1/s4r(17) ),
        .I1(\U_FFT1/s4r(15) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_17 )
    );

    SB_DFFE \U_FFT1/s4r_Z[5]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(5) ),
        .Q(\U_FFT1/s4r(5) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_14_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_13 ),
        .I0(\U_FFT1/s4r(14) ),
        .I1(\U_FFT1/s4r(12) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_14 )
    );

    SB_DFFE \U_FFT1/s4r_Z[6]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(6) ),
        .Q(\U_FFT1/s4r(6) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_15_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_14 ),
        .I0(\U_FFT1/s4r(15) ),
        .I1(\U_FFT1/s4r(13) ),
        .CO(\U_FFT1/UMR/dx5_1_cry_15 )
    );

    SB_DFFE \U_FFT1/s4r_Z[11]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(11) ),
        .Q(\U_FFT1/s4r(11) )
    );

    SB_DFFE \U_FFT1/s4r_Z[12]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(12) ),
        .Q(\U_FFT1/s4r(12) )
    );

    SB_DFFE \U_FFT1/s4r_Z[9]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(9) ),
        .Q(\U_FFT1/s4r(9) )
    );

    SB_CARRY \U_FFT1/UMR/dx5_1_cry_18_c  (
        .CI(\U_FFT1/UMR/dx5_1_cry_17 ),
        .I0(\U_FFT1/s4r(17) ),
        .I1(\U_FFT1/s4r(16) ),
        .CO(\U_FFT1/UMR/dx5_1(19) )
    );

    SB_DFFE \U_FFT1/s4r_Z[10]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(10) ),
        .Q(\U_FFT1/s4r(10) )
    );

    SB_DFFE \U_FFT1/s4r_Z[15]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(15) ),
        .Q(\U_FFT1/s4r(15) )
    );

    SB_DFFE \U_FFT1/s4r_Z[16]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(16) ),
        .Q(\U_FFT1/s4r(16) )
    );

    SB_DFFE \U_FFT1/s4r_Z[13]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(13) ),
        .Q(\U_FFT1/s4r(13) )
    );

    SB_DFFE \U_FFT1/s4r_Z[14]  (
        .C(\top_module/CLK_c_g ),
        .E(\U_FFT1/un1_dr3_2_0 ),
        .D(\s4r_RNO_0(14) ),
        .Q(\U_FFT1/s4r(14) )
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
endmodule

