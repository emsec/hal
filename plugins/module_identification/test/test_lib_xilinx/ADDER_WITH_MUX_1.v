`timescale 1 ps/1 ps
module USFFT64_2B(\U_FFT1/s3r1 ,\U_FFT1/sji_reg_n_0_[16] ,\U_FFT1/m4m7i(4) ,\U_FFT1/m4m7i(16) ,\U_FFT1/m4m7i(8) ,\U_FFT1/m4m7i(12) ,\U_FFT1/m6i_reg_n_0_[17] ,\U_FFT1/m4m7i(15) ,\U_FFT1/sji_reg_n_0_[0] ,\U_FFT1/m4m7i(9) ,\U_FFT1/m6i_reg_n_0_[12] ,\U_FFT1/m4m7i(17) ,\U_FFT1/m4m7i(1) ,\U_FFT1/sji_reg_n_0_[17] ,\U_FFT1/s2d3i_reg_n_0_[14] ,\U_FFT1/m4m7i(3) ,\U_FFT1/s2d3i_reg_n_0_[11] ,\U_FFT1/m6i_reg_n_0_[1] ,\U_FFT1/s2d3i_reg_n_0_[5] ,\U_FFT1/m6i_reg_n_0_[4] ,\U_FFT1/sji_reg_n_0_[2] ,\U_FFT1/m6i_reg_n_0_[2] ,\U_FFT1/m4m7i(5) ,\U_FFT1/m6i_reg_n_0_[10] ,\U_FFT1/m6i_reg_n_0_[14] ,\U_FFT1/m6i_reg_n_0_[9] ,\U_FFT1/m6i_reg_n_0_[8] ,\U_FFT1/sji_reg_n_0_[12] ,\U_FFT1/s2d3i_reg_n_0_[9] ,\U_FFT1/s2d3i_reg_n_0_[4] ,\U_FFT1/s2d3i_reg_n_0_[3] ,\U_FFT1/s2d3i_reg_n_0_[12] ,\U_FFT1/s2d3i_reg_n_0_[10] ,\U_FFT1/sji_reg_n_0_[4] ,\U_FFT1/s2d3i_reg_n_0_[8] ,\U_FFT1/s2d3i_reg_n_0_[2] ,\U_FFT1/sji_reg_n_0_[14] ,\U_FFT1/sji_reg_n_0_[8] ,\U_FFT1/sji_reg_n_0_[15] ,\U_FFT1/m6i_reg_n_0_[5] ,\U_FFT1/s2d3i_reg_n_0_[1] ,\U_FFT1/m4m7i(0) ,\U_FFT1/sji_reg_n_0_[6] ,\U_FFT1/m4m7i(2) ,\U_FFT1/m6i_reg_n_0_[3] ,\U_FFT1/sji_reg_n_0_[5] ,\U_FFT1/sji_reg_n_0_[7] ,\U_FFT1/sji_reg_n_0_[1] ,\U_FFT1/m4m7i(6) ,\U_FFT1/m6i_reg_n_0_[16] ,\U_FFT1/s2d3i_reg_n_0_[16] ,\U_FFT1/m4m7i(11) ,\U_FFT1/sji_reg_n_0_[13] ,\U_FFT1/sji_reg_n_0_[11] ,\U_FFT1/m4m7i(7) ,\U_FFT1/m4m7i(10) ,\U_FFT1/m6i_reg_n_0_[7] ,\U_FFT1/sji_reg_n_0_[9] ,\U_FFT1/s2d3i_reg_n_0_[7] ,\U_FFT1/m6i_reg_n_0_[6] ,\U_FFT1/s5r1 ,\U_FFT1/s2d3i_reg_n_0_[0] ,\U_FFT1/sji_reg_n_0_[3] ,\U_FFT1/m6i_reg_n_0_[11] ,\U_FFT1/m6i_reg_n_0_[0] ,\U_FFT1/m4m7i(13) ,\U_FFT1/s2d3i_reg_n_0_[15] ,\U_FFT1/m4m7i(14) ,\U_FFT1/s2d3i_reg_n_0_[6] ,\U_FFT1/sji_reg_n_0_[10] ,\U_FFT1/m6i_reg_n_0_[13] ,\U_FFT1/m6i_reg_n_0_[15] ,\U_FFT1/s2d3i_reg_n_0_[13] ,\U_FFT1/s5i(13) ,\U_FFT1/s5i(3) ,\U_FFT1/s5i(10) ,\U_FFT1/s5i(9) ,\U_FFT1/s5i(7) ,\U_FFT1/s5i(0) ,\U_FFT1/s5i(16) ,\U_FFT1/s5i(17) ,\U_FFT1/s5i(4) ,\U_FFT1/s5i(6) ,\U_FFT1/s5i(12) ,\U_FFT1/s5i(2) ,\U_FFT1/s5i(8) ,\U_FFT1/s5i(11) ,\U_FFT1/s5i(14) ,\U_FFT1/s5i(18) ,\U_FFT1/s5i(1) ,\U_FFT1/s5i(5) ,\U_FFT1/s5i(15) );
    input \U_FFT1/s3r1 ;
    input \U_FFT1/sji_reg_n_0_[16] ;
    input \U_FFT1/m4m7i(4) ;
    input \U_FFT1/m4m7i(16) ;
    input \U_FFT1/m4m7i(8) ;
    input \U_FFT1/m4m7i(12) ;
    input \U_FFT1/m6i_reg_n_0_[17] ;
    input \U_FFT1/m4m7i(15) ;
    input \U_FFT1/sji_reg_n_0_[0] ;
    input \U_FFT1/m4m7i(9) ;
    input \U_FFT1/m6i_reg_n_0_[12] ;
    input \U_FFT1/m4m7i(17) ;
    input \U_FFT1/m4m7i(1) ;
    input \U_FFT1/sji_reg_n_0_[17] ;
    input \U_FFT1/s2d3i_reg_n_0_[14] ;
    input \U_FFT1/m4m7i(3) ;
    input \U_FFT1/s2d3i_reg_n_0_[11] ;
    input \U_FFT1/m6i_reg_n_0_[1] ;
    input \U_FFT1/s2d3i_reg_n_0_[5] ;
    input \U_FFT1/m6i_reg_n_0_[4] ;
    input \U_FFT1/sji_reg_n_0_[2] ;
    input \U_FFT1/m6i_reg_n_0_[2] ;
    input \U_FFT1/m4m7i(5) ;
    input \U_FFT1/m6i_reg_n_0_[10] ;
    input \U_FFT1/m6i_reg_n_0_[14] ;
    input \U_FFT1/m6i_reg_n_0_[9] ;
    input \U_FFT1/m6i_reg_n_0_[8] ;
    input \U_FFT1/sji_reg_n_0_[12] ;
    input \U_FFT1/s2d3i_reg_n_0_[9] ;
    input \U_FFT1/s2d3i_reg_n_0_[4] ;
    input \U_FFT1/s2d3i_reg_n_0_[3] ;
    input \U_FFT1/s2d3i_reg_n_0_[12] ;
    input \U_FFT1/s2d3i_reg_n_0_[10] ;
    input \U_FFT1/sji_reg_n_0_[4] ;
    input \U_FFT1/s2d3i_reg_n_0_[8] ;
    input \U_FFT1/s2d3i_reg_n_0_[2] ;
    input \U_FFT1/sji_reg_n_0_[14] ;
    input \U_FFT1/sji_reg_n_0_[8] ;
    input \U_FFT1/sji_reg_n_0_[15] ;
    input \U_FFT1/m6i_reg_n_0_[5] ;
    input \U_FFT1/s2d3i_reg_n_0_[1] ;
    input \U_FFT1/m4m7i(0) ;
    input \U_FFT1/sji_reg_n_0_[6] ;
    input \U_FFT1/m4m7i(2) ;
    input \U_FFT1/m6i_reg_n_0_[3] ;
    input \U_FFT1/sji_reg_n_0_[5] ;
    input \U_FFT1/sji_reg_n_0_[7] ;
    input \U_FFT1/sji_reg_n_0_[1] ;
    input \U_FFT1/m4m7i(6) ;
    input \U_FFT1/m6i_reg_n_0_[16] ;
    input \U_FFT1/s2d3i_reg_n_0_[16] ;
    input \U_FFT1/m4m7i(11) ;
    input \U_FFT1/sji_reg_n_0_[13] ;
    input \U_FFT1/sji_reg_n_0_[11] ;
    input \U_FFT1/m4m7i(7) ;
    input \U_FFT1/m4m7i(10) ;
    input \U_FFT1/m6i_reg_n_0_[7] ;
    input \U_FFT1/sji_reg_n_0_[9] ;
    input \U_FFT1/s2d3i_reg_n_0_[7] ;
    input \U_FFT1/m6i_reg_n_0_[6] ;
    input \U_FFT1/s5r1 ;
    input \U_FFT1/s2d3i_reg_n_0_[0] ;
    input \U_FFT1/sji_reg_n_0_[3] ;
    input \U_FFT1/m6i_reg_n_0_[11] ;
    input \U_FFT1/m6i_reg_n_0_[0] ;
    input \U_FFT1/m4m7i(13) ;
    input \U_FFT1/s2d3i_reg_n_0_[15] ;
    input \U_FFT1/m4m7i(14) ;
    input \U_FFT1/s2d3i_reg_n_0_[6] ;
    input \U_FFT1/sji_reg_n_0_[10] ;
    input \U_FFT1/m6i_reg_n_0_[13] ;
    input \U_FFT1/m6i_reg_n_0_[15] ;
    input \U_FFT1/s2d3i_reg_n_0_[13] ;
    output \U_FFT1/s5i(13) ;
    output \U_FFT1/s5i(3) ;
    output \U_FFT1/s5i(10) ;
    output \U_FFT1/s5i(9) ;
    output \U_FFT1/s5i(7) ;
    output \U_FFT1/s5i(0) ;
    output \U_FFT1/s5i(16) ;
    output \U_FFT1/s5i(17) ;
    output \U_FFT1/s5i(4) ;
    output \U_FFT1/s5i(6) ;
    output \U_FFT1/s5i(12) ;
    output \U_FFT1/s5i(2) ;
    output \U_FFT1/s5i(8) ;
    output \U_FFT1/s5i(11) ;
    output \U_FFT1/s5i(14) ;
    output \U_FFT1/s5i(18) ;
    output \U_FFT1/s5i(1) ;
    output \U_FFT1/s5i(5) ;
    output \U_FFT1/s5i(15) ;
    wire \s5i_reg[3]_i_1_n_3 ;
    wire \s5i_reg[18]_i_1_n_3 ;
    wire \s5i[15]_i_4_n_0 ;
    wire \s5i[7]_i_7_n_0 ;
    wire \s5i[7]_i_9_n_0 ;
    wire \s5i[3]_i_3_n_0 ;
    wire \s5i[15]_i_9_n_0 ;
    wire \s5i_reg[18]_i_1_n_2 ;
    wire \s5i[7]_i_8_n_0 ;
    wire \s5i[18]_i_6_n_0 ;
    wire \s5i[7]_i_2_n_0 ;
    wire \s5i[11]_i_6_n_0 ;
    wire \s5i[11]_i_7_n_0 ;
    wire \s5i[11]_i_8_n_0 ;
    wire \s5i_reg[11]_i_1_n_2 ;
    wire \s5i[18]_i_4_n_0 ;
    wire \s5i[11]_i_9_n_0 ;
    wire \s5i[3]_i_4_n_0 ;
    wire \s5i_reg[3]_i_1_n_1 ;
    wire \s5i_reg[7]_i_1_n_2 ;
    wire \s5i[15]_i_8_n_0 ;
    wire \s5i[11]_i_5_n_0 ;
    wire \s5i_reg[11]_i_1_n_0 ;
    wire \s5i_reg[7]_i_1_n_0 ;
    wire \s5i_reg[11]_i_1_n_1 ;
    wire \s5i_reg[11]_i_1_n_3 ;
    wire \s5i[11]_i_2_n_0 ;
    wire \s5i[11]_i_3_n_0 ;
    wire \s5i[15]_i_2_n_0 ;
    wire \s5i[3]_i_2_n_0 ;
    wire \s5i[18]_i_3_n_0 ;
    wire \s5i[15]_i_3_n_0 ;
    wire \s5i[15]_i_5_n_0 ;
    wire \s5i[15]_i_6_n_0 ;
    wire \s5i[15]_i_7_n_0 ;
    wire \s5i_reg[15]_i_1_n_1 ;
    wire \s5i[7]_i_5_n_0 ;
    wire \s5i[11]_i_4_n_0 ;
    wire \s5i_reg[15]_i_1_n_2 ;
    wire \s5i_reg[15]_i_1_n_3 ;
    wire \s5i[3]_i_6_n_0 ;
    wire \s5i[7]_i_6_n_0 ;
    wire \s5i_reg[7]_i_1_n_3 ;
    wire \s5i[3]_i_5_n_0 ;
    wire \s5i[3]_i_7_n_0 ;
    wire \s5i_reg[3]_i_1_n_0 ;
    wire \s5i_reg[3]_i_1_n_2 ;
    wire \s5i_reg[7]_i_1_n_1 ;
    wire \s5i[7]_i_3_n_0 ;
    wire \s5i[7]_i_4_n_0 ;
    wire \s5i[3]_i_8_n_0 ;
    wire \s5i[3]_i_9_n_0 ;
    wire \s5i[18]_i_2_n_0 ;
    wire \s5i_reg[15]_i_1_n_0 ;

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[18]_i_4  (
        .I0(\U_FFT1/sji_reg_n_0_[17] ),
        .I1(\U_FFT1/m4m7i(17) ),
        .I2(\U_FFT1/m6i_reg_n_0_[17] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[16] ),
        .O(\s5i[18]_i_4_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[18]_i_3  (
        .I0(\U_FFT1/m4m7i(16) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[16] ),
        .O(\s5i[18]_i_3_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[18]_i_6  (
        .I0(\U_FFT1/sji_reg_n_0_[16] ),
        .I1(\U_FFT1/m4m7i(16) ),
        .I2(\U_FFT1/m6i_reg_n_0_[16] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[16] ),
        .O(\s5i[18]_i_6_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[7]_i_8  (
        .I0(\U_FFT1/sji_reg_n_0_[5] ),
        .I1(\U_FFT1/m4m7i(5) ),
        .I2(\U_FFT1/m6i_reg_n_0_[5] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[5] ),
        .O(\s5i[7]_i_8_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[7]_i_9  (
        .I0(\U_FFT1/sji_reg_n_0_[4] ),
        .I1(\U_FFT1/m4m7i(4) ),
        .I2(\U_FFT1/m6i_reg_n_0_[4] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[4] ),
        .O(\s5i[7]_i_9_n_0 )
    );

    CARRY4 \s5i_reg[11]_i_1  (
        .CI(\s5i_reg[7]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({\s5i[11]_i_2_n_0 ,
\s5i[11]_i_3_n_0 ,
\s5i[11]_i_4_n_0 ,
\s5i[11]_i_5_n_0 }),
        .S({\s5i[11]_i_6_n_0 ,
\s5i[11]_i_7_n_0 ,
\s5i[11]_i_8_n_0 ,
\s5i[11]_i_9_n_0 }),
        .CO({\s5i_reg[11]_i_1_n_0 ,
\s5i_reg[11]_i_1_n_1 ,
\s5i_reg[11]_i_1_n_2 ,
\s5i_reg[11]_i_1_n_3 }),
        .O({\U_FFT1/s5i(11) ,
\U_FFT1/s5i(10) ,
\U_FFT1/s5i(9) ,
\U_FFT1/s5i(8) })
    );

    CARRY4 \s5i_reg[15]_i_1  (
        .CI(\s5i_reg[11]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({\s5i[15]_i_2_n_0 ,
\s5i[15]_i_3_n_0 ,
\s5i[15]_i_4_n_0 ,
\s5i[15]_i_5_n_0 }),
        .S({\s5i[15]_i_6_n_0 ,
\s5i[15]_i_7_n_0 ,
\s5i[15]_i_8_n_0 ,
\s5i[15]_i_9_n_0 }),
        .CO({\s5i_reg[15]_i_1_n_0 ,
\s5i_reg[15]_i_1_n_1 ,
\s5i_reg[15]_i_1_n_2 ,
\s5i_reg[15]_i_1_n_3 }),
        .O({\U_FFT1/s5i(15) ,
\U_FFT1/s5i(14) ,
\U_FFT1/s5i(13) ,
\U_FFT1/s5i(12) })
    );

    CARRY4 \s5i_reg[18]_i_1  (
        .CI(\s5i_reg[15]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({1'b0 ,
1'b0 ,
\s5i[18]_i_2_n_0 ,
\s5i[18]_i_3_n_0 }),
        .S({1'b0 ,
\s5i[18]_i_4_n_0 ,
\s5i[18]_i_4_n_0 ,
\s5i[18]_i_6_n_0 }),
        .CO({1'bz,
1'bz,
\s5i_reg[18]_i_1_n_2 ,
\s5i_reg[18]_i_1_n_3 }),
        .O({1'bz,
\U_FFT1/s5i(18) ,
\U_FFT1/s5i(17) ,
\U_FFT1/s5i(16) })
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[11]_i_7  (
        .I0(\U_FFT1/sji_reg_n_0_[10] ),
        .I1(\U_FFT1/m4m7i(10) ),
        .I2(\U_FFT1/m6i_reg_n_0_[10] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[10] ),
        .O(\s5i[11]_i_7_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[11]_i_8  (
        .I0(\U_FFT1/sji_reg_n_0_[9] ),
        .I1(\U_FFT1/m4m7i(9) ),
        .I2(\U_FFT1/m6i_reg_n_0_[9] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[9] ),
        .O(\s5i[11]_i_8_n_0 )
    );

    CARRY4 \s5i_reg[3]_i_1  (
        .CI(1'b0 ),
        .CYINIT(\s5i[3]_i_2_n_0 ),
        .DI({\s5i[3]_i_3_n_0 ,
\s5i[3]_i_4_n_0 ,
\s5i[3]_i_5_n_0 ,
\U_FFT1/s5r1 }),
        .S({\s5i[3]_i_6_n_0 ,
\s5i[3]_i_7_n_0 ,
\s5i[3]_i_8_n_0 ,
\s5i[3]_i_9_n_0 }),
        .CO({\s5i_reg[3]_i_1_n_0 ,
\s5i_reg[3]_i_1_n_1 ,
\s5i_reg[3]_i_1_n_2 ,
\s5i_reg[3]_i_1_n_3 }),
        .O({\U_FFT1/s5i(3) ,
\U_FFT1/s5i(2) ,
\U_FFT1/s5i(1) ,
\U_FFT1/s5i(0) })
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[11]_i_9  (
        .I0(\U_FFT1/sji_reg_n_0_[8] ),
        .I1(\U_FFT1/m4m7i(8) ),
        .I2(\U_FFT1/m6i_reg_n_0_[8] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[8] ),
        .O(\s5i[11]_i_9_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[15]_i_2  (
        .I0(\U_FFT1/m4m7i(15) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[15] ),
        .O(\s5i[15]_i_2_n_0 )
    );

    CARRY4 \s5i_reg[7]_i_1  (
        .CI(\s5i_reg[3]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({\s5i[7]_i_2_n_0 ,
\s5i[7]_i_3_n_0 ,
\s5i[7]_i_4_n_0 ,
\s5i[7]_i_5_n_0 }),
        .S({\s5i[7]_i_6_n_0 ,
\s5i[7]_i_7_n_0 ,
\s5i[7]_i_8_n_0 ,
\s5i[7]_i_9_n_0 }),
        .CO({\s5i_reg[7]_i_1_n_0 ,
\s5i_reg[7]_i_1_n_1 ,
\s5i_reg[7]_i_1_n_2 ,
\s5i_reg[7]_i_1_n_3 }),
        .O({\U_FFT1/s5i(7) ,
\U_FFT1/s5i(6) ,
\U_FFT1/s5i(5) ,
\U_FFT1/s5i(4) })
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[3]_i_8  (
        .I0(\U_FFT1/sji_reg_n_0_[1] ),
        .I1(\U_FFT1/m4m7i(1) ),
        .I2(\U_FFT1/m6i_reg_n_0_[1] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[1] ),
        .O(\s5i[3]_i_8_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[15]_i_3  (
        .I0(\U_FFT1/m4m7i(14) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[14] ),
        .O(\s5i[15]_i_3_n_0 )
    );

    LUT4 #(
        .INIT(16'hEA2A)
    ) \s5i[3]_i_9  (
        .I0(\U_FFT1/sji_reg_n_0_[0] ),
        .I1(\U_FFT1/s5r1 ),
        .I2(\U_FFT1/s3r1 ),
        .I3(\U_FFT1/m4m7i(0) ),
        .O(\s5i[3]_i_9_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[15]_i_4  (
        .I0(\U_FFT1/m4m7i(13) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[13] ),
        .O(\s5i[15]_i_4_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[7]_i_2  (
        .I0(\U_FFT1/m4m7i(7) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[7] ),
        .O(\s5i[7]_i_2_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[15]_i_5  (
        .I0(\U_FFT1/m4m7i(12) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[12] ),
        .O(\s5i[15]_i_5_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[7]_i_3  (
        .I0(\U_FFT1/m4m7i(6) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[6] ),
        .O(\s5i[7]_i_3_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[15]_i_6  (
        .I0(\U_FFT1/sji_reg_n_0_[15] ),
        .I1(\U_FFT1/m4m7i(15) ),
        .I2(\U_FFT1/m6i_reg_n_0_[15] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[15] ),
        .O(\s5i[15]_i_6_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[7]_i_4  (
        .I0(\U_FFT1/m4m7i(5) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[5] ),
        .O(\s5i[7]_i_4_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[7]_i_5  (
        .I0(\U_FFT1/m4m7i(4) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[4] ),
        .O(\s5i[7]_i_5_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[7]_i_6  (
        .I0(\U_FFT1/sji_reg_n_0_[7] ),
        .I1(\U_FFT1/m4m7i(7) ),
        .I2(\U_FFT1/m6i_reg_n_0_[7] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[7] ),
        .O(\s5i[7]_i_6_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[7]_i_7  (
        .I0(\U_FFT1/sji_reg_n_0_[6] ),
        .I1(\U_FFT1/m4m7i(6) ),
        .I2(\U_FFT1/m6i_reg_n_0_[6] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[6] ),
        .O(\s5i[7]_i_7_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[11]_i_2  (
        .I0(\U_FFT1/m4m7i(11) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[11] ),
        .O(\s5i[11]_i_2_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[11]_i_3  (
        .I0(\U_FFT1/m4m7i(10) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[10] ),
        .O(\s5i[11]_i_3_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[11]_i_4  (
        .I0(\U_FFT1/m4m7i(9) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[9] ),
        .O(\s5i[11]_i_4_n_0 )
    );

    LUT4 #(
        .INIT(16'hBF80)
    ) \s5i[3]_i_2  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[0] ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/m6i_reg_n_0_[0] ),
        .O(\s5i[3]_i_2_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[11]_i_5  (
        .I0(\U_FFT1/m4m7i(8) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[8] ),
        .O(\s5i[11]_i_5_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[3]_i_3  (
        .I0(\U_FFT1/m4m7i(3) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[3] ),
        .O(\s5i[3]_i_3_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[11]_i_6  (
        .I0(\U_FFT1/sji_reg_n_0_[11] ),
        .I1(\U_FFT1/m4m7i(11) ),
        .I2(\U_FFT1/m6i_reg_n_0_[11] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[11] ),
        .O(\s5i[11]_i_6_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[3]_i_4  (
        .I0(\U_FFT1/m4m7i(2) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[2] ),
        .O(\s5i[3]_i_4_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[3]_i_5  (
        .I0(\U_FFT1/m4m7i(1) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[1] ),
        .O(\s5i[3]_i_5_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[3]_i_6  (
        .I0(\U_FFT1/sji_reg_n_0_[3] ),
        .I1(\U_FFT1/m4m7i(3) ),
        .I2(\U_FFT1/m6i_reg_n_0_[3] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[3] ),
        .O(\s5i[3]_i_6_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[3]_i_7  (
        .I0(\U_FFT1/sji_reg_n_0_[2] ),
        .I1(\U_FFT1/m4m7i(2) ),
        .I2(\U_FFT1/m6i_reg_n_0_[2] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[2] ),
        .O(\s5i[3]_i_7_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[15]_i_7  (
        .I0(\U_FFT1/sji_reg_n_0_[14] ),
        .I1(\U_FFT1/m4m7i(14) ),
        .I2(\U_FFT1/m6i_reg_n_0_[14] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[14] ),
        .O(\s5i[15]_i_7_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[15]_i_8  (
        .I0(\U_FFT1/sji_reg_n_0_[13] ),
        .I1(\U_FFT1/m4m7i(13) ),
        .I2(\U_FFT1/m6i_reg_n_0_[13] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[13] ),
        .O(\s5i[15]_i_8_n_0 )
    );

    LUT6 #(
        .INIT(64'hCC5AA55A335AA55A)
    ) \s5i[15]_i_9  (
        .I0(\U_FFT1/sji_reg_n_0_[12] ),
        .I1(\U_FFT1/m4m7i(12) ),
        .I2(\U_FFT1/m6i_reg_n_0_[12] ),
        .I3(\U_FFT1/s5r1 ),
        .I4(\U_FFT1/s3r1 ),
        .I5(\U_FFT1/s2d3i_reg_n_0_[12] ),
        .O(\s5i[15]_i_9_n_0 )
    );

    LUT4 #(
        .INIT(16'h4F70)
    ) \s5i[18]_i_2  (
        .I0(\U_FFT1/m4m7i(17) ),
        .I1(\U_FFT1/s3r1 ),
        .I2(\U_FFT1/s5r1 ),
        .I3(\U_FFT1/sji_reg_n_0_[17] ),
        .O(\s5i[18]_i_2_n_0 )
    );
endmodule

