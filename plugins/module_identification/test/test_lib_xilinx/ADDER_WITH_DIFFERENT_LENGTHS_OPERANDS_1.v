`timescale 1 ps/1 ps
module USFFT64_2B(\U_FFT1/s2d3i_reg_n_0_[16] ,\U_FFT1/m4m7i(14) ,\U_FFT1/s2d3i_reg_n_0_[13] ,\U_FFT1/s2d3i_reg_n_0_[1] ,\U_FFT1/m4m7i(15) ,\U_FFT1/s2d3i_reg_n_0_[4] ,\U_FFT1/s2d3i_reg_n_0_[3] ,\U_FFT1/s2d3i_reg_n_0_[6] ,\U_FFT1/m4m7i(11) ,\U_FFT1/m4m7i(0) ,\U_FFT1/s2d3i_reg_n_0_[8] ,\U_FFT1/m4m7i(9) ,\U_FFT1/s2d3i_reg_n_0_[14] ,\U_FFT1/s2d3i_reg_n_0_[11] ,\U_FFT1/m4m7i(17) ,\U_FFT1/s2d3i_reg_n_0_[15] ,\U_FFT1/m4m7i(12) ,\U_FFT1/m4m7i(6) ,\U_FFT1/s2d3i_reg_n_0_[7] ,\U_FFT1/s2d3i_reg_n_0_[5] ,\U_FFT1/m4m7i(1) ,\U_FFT1/s2d3i_reg_n_0_[10] ,\U_FFT1/s2d3i_reg_n_0_[9] ,\U_FFT1/s2d3i_reg_n_0_[0] ,\U_FFT1/m4m7i(16) ,\U_FFT1/m4m7i(13) ,\U_FFT1/m4m7i(5) ,\U_FFT1/s2d3i_reg_n_0_[12] ,\U_FFT1/m4m7i(2) ,\U_FFT1/m4m7i(4) ,\U_FFT1/s2d3i_reg_n_0_[2] ,\U_FFT1/m4m7i(10) ,\U_FFT1/m4m7i(7) ,\U_FFT1/m4m7i(8) ,\U_FFT1/m4m7i(3) ,\q1i_reg[7]_i_1_n_4 ,\q1i_reg[7]_i_1_n_6 ,\q1i_reg[3]_i_1_n_6 ,\q1i_reg[7]_i_1_n_7 ,\q1i_reg[7]_i_1_n_5 ,\q1i_reg[11]_i_1_n_6 ,\q1i_reg[3]_i_1_n_4 ,\q1i_reg[15]_i_1_n_6 ,\q1i_reg[11]_i_1_n_5 ,\q1i_reg[15]_i_1_n_4 ,\q1i_reg[18]_i_1_n_6 ,\q1i_reg[3]_i_1_n_7 ,\q1i_reg[18]_i_1_n_7 ,\q1i_reg[11]_i_1_n_4 ,\q1i_reg[18]_i_1_n_5 ,\q1i_reg[15]_i_1_n_7 ,\q1i_reg[3]_i_1_n_5 ,\q1i_reg[15]_i_1_n_5 ,\q1i_reg[11]_i_1_n_7 );
    input \U_FFT1/s2d3i_reg_n_0_[16] ;
    input \U_FFT1/m4m7i(14) ;
    input \U_FFT1/s2d3i_reg_n_0_[13] ;
    input \U_FFT1/s2d3i_reg_n_0_[1] ;
    input \U_FFT1/m4m7i(15) ;
    input \U_FFT1/s2d3i_reg_n_0_[4] ;
    input \U_FFT1/s2d3i_reg_n_0_[3] ;
    input \U_FFT1/s2d3i_reg_n_0_[6] ;
    input \U_FFT1/m4m7i(11) ;
    input \U_FFT1/m4m7i(0) ;
    input \U_FFT1/s2d3i_reg_n_0_[8] ;
    input \U_FFT1/m4m7i(9) ;
    input \U_FFT1/s2d3i_reg_n_0_[14] ;
    input \U_FFT1/s2d3i_reg_n_0_[11] ;
    input \U_FFT1/m4m7i(17) ;
    input \U_FFT1/s2d3i_reg_n_0_[15] ;
    input \U_FFT1/m4m7i(12) ;
    input \U_FFT1/m4m7i(6) ;
    input \U_FFT1/s2d3i_reg_n_0_[7] ;
    input \U_FFT1/s2d3i_reg_n_0_[5] ;
    input \U_FFT1/m4m7i(1) ;
    input \U_FFT1/s2d3i_reg_n_0_[10] ;
    input \U_FFT1/s2d3i_reg_n_0_[9] ;
    input \U_FFT1/s2d3i_reg_n_0_[0] ;
    input \U_FFT1/m4m7i(16) ;
    input \U_FFT1/m4m7i(13) ;
    input \U_FFT1/m4m7i(5) ;
    input \U_FFT1/s2d3i_reg_n_0_[12] ;
    input \U_FFT1/m4m7i(2) ;
    input \U_FFT1/m4m7i(4) ;
    input \U_FFT1/s2d3i_reg_n_0_[2] ;
    input \U_FFT1/m4m7i(10) ;
    input \U_FFT1/m4m7i(7) ;
    input \U_FFT1/m4m7i(8) ;
    input \U_FFT1/m4m7i(3) ;
    output \q1i_reg[7]_i_1_n_4 ;
    output \q1i_reg[7]_i_1_n_6 ;
    output \q1i_reg[3]_i_1_n_6 ;
    output \q1i_reg[7]_i_1_n_7 ;
    output \q1i_reg[7]_i_1_n_5 ;
    output \q1i_reg[11]_i_1_n_6 ;
    output \q1i_reg[3]_i_1_n_4 ;
    output \q1i_reg[15]_i_1_n_6 ;
    output \q1i_reg[11]_i_1_n_5 ;
    output \q1i_reg[15]_i_1_n_4 ;
    output \q1i_reg[18]_i_1_n_6 ;
    output \q1i_reg[3]_i_1_n_7 ;
    output \q1i_reg[18]_i_1_n_7 ;
    output \q1i_reg[11]_i_1_n_4 ;
    output \q1i_reg[18]_i_1_n_5 ;
    output \q1i_reg[15]_i_1_n_7 ;
    output \q1i_reg[3]_i_1_n_5 ;
    output \q1i_reg[15]_i_1_n_5 ;
    output \q1i_reg[11]_i_1_n_7 ;
    wire \q1i[7]_i_2_n_0 ;
    wire \q1i[7]_i_5_n_0 ;
    wire \q1i[7]_i_3_n_0 ;
    wire \q1i_reg[3]_i_1_n_0 ;
    wire \q1i_reg[7]_i_1_n_3 ;
    wire \q1i[11]_i_5_n_0 ;
    wire \q1i_reg[7]_i_1_n_2 ;
    wire \q1i_reg[7]_i_1_n_1 ;
    wire \q1i[18]_i_4_n_0 ;
    wire \q1i[3]_i_3_n_0 ;
    wire \q1i_reg[3]_i_1_n_1 ;
    wire \q1i[3]_i_2_n_0 ;
    wire \q1i[7]_i_4_n_0 ;
    wire \q1i[3]_i_4_n_0 ;
    wire \q1i[15]_i_2_n_0 ;
    wire \q1i[3]_i_5_n_0 ;
    wire \q1i_reg[7]_i_1_n_0 ;
    wire \q1i_reg[3]_i_1_n_2 ;
    wire \q1i_reg[15]_i_1_n_0 ;
    wire \q1i_reg[3]_i_1_n_3 ;
    wire \q1i[18]_i_2_n_0 ;
    wire \q1i[18]_i_3_n_0 ;
    wire \q1i[15]_i_3_n_0 ;
    wire \q1i[15]_i_4_n_0 ;
    wire \q1i[11]_i_3_n_0 ;
    wire \q1i[15]_i_5_n_0 ;
    wire \q1i_reg[11]_i_1_n_0 ;
    wire \q1i_reg[15]_i_1_n_1 ;
    wire \q1i_reg[15]_i_1_n_3 ;
    wire \q1i[11]_i_4_n_0 ;
    wire \q1i_reg[11]_i_1_n_2 ;
    wire \q1i_reg[18]_i_1_n_3 ;
    wire \q1i_reg[18]_i_1_n_2 ;
    wire \q1i_reg[11]_i_1_n_1 ;
    wire \q1i[11]_i_2_n_0 ;
    wire \q1i_reg[15]_i_1_n_2 ;
    wire \q1i_reg[11]_i_1_n_3 ;

    CARRY4 \q1i_reg[7]_i_1  (
        .CI(\q1i_reg[3]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s2d3i_reg_n_0_[7] ,
            \U_FFT1/s2d3i_reg_n_0_[6] ,
            \U_FFT1/s2d3i_reg_n_0_[5] ,
            \U_FFT1/s2d3i_reg_n_0_[4] 
        }),
        .S({
            \q1i[7]_i_2_n_0 ,
            \q1i[7]_i_3_n_0 ,
            \q1i[7]_i_4_n_0 ,
            \q1i[7]_i_5_n_0 
        }),
        .CO({
            \q1i_reg[7]_i_1_n_0 ,
            \q1i_reg[7]_i_1_n_1 ,
            \q1i_reg[7]_i_1_n_2 ,
            \q1i_reg[7]_i_1_n_3 
        }),
        .O({
            \q1i_reg[7]_i_1_n_4 ,
            \q1i_reg[7]_i_1_n_5 ,
            \q1i_reg[7]_i_1_n_6 ,
            \q1i_reg[7]_i_1_n_7 
        })
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[18]_i_4  (
        .I0(\U_FFT1/m4m7i(16) ),
        .I1(\U_FFT1/s2d3i_reg_n_0_[16] ),
        .O(\q1i[18]_i_4_n_0 )
    );

    CARRY4 \q1i_reg[3]_i_1  (
        .CI(1'b0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s2d3i_reg_n_0_[3] ,
            \U_FFT1/s2d3i_reg_n_0_[2] ,
            \U_FFT1/s2d3i_reg_n_0_[1] ,
            \U_FFT1/s2d3i_reg_n_0_[0] 
        }),
        .S({
            \q1i[3]_i_2_n_0 ,
            \q1i[3]_i_3_n_0 ,
            \q1i[3]_i_4_n_0 ,
            \q1i[3]_i_5_n_0 
        }),
        .CO({
            \q1i_reg[3]_i_1_n_0 ,
            \q1i_reg[3]_i_1_n_1 ,
            \q1i_reg[3]_i_1_n_2 ,
            \q1i_reg[3]_i_1_n_3 
        }),
        .O({
            \q1i_reg[3]_i_1_n_4 ,
            \q1i_reg[3]_i_1_n_5 ,
            \q1i_reg[3]_i_1_n_6 ,
            \q1i_reg[3]_i_1_n_7 
        })
    );

    CARRY4 \q1i_reg[18]_i_1  (
        .CI(\q1i_reg[15]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            \U_FFT1/m4m7i(16) ,
            \q1i[18]_i_2_n_0 
        }),
        .S({
            1'b0 ,
            1'b1 ,
            \q1i[18]_i_3_n_0 ,
            \q1i[18]_i_4_n_0 
        }),
        .CO({
            1'bz,
            1'bz,
            \q1i_reg[18]_i_1_n_2 ,
            \q1i_reg[18]_i_1_n_3 
        }),
        .O({
            1'bz,
            \q1i_reg[18]_i_1_n_5 ,
            \q1i_reg[18]_i_1_n_6 ,
            \q1i_reg[18]_i_1_n_7 
        })
    );

    LUT1 #(
        .INIT(1'b1)
    ) \q1i[18]_i_2  (
        .I0(\U_FFT1/m4m7i(16) ),
        .O(\q1i[18]_i_2_n_0 )
    );

    CARRY4 \q1i_reg[15]_i_1  (
        .CI(\q1i_reg[11]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s2d3i_reg_n_0_[15] ,
            \U_FFT1/s2d3i_reg_n_0_[14] ,
            \U_FFT1/s2d3i_reg_n_0_[13] ,
            \U_FFT1/s2d3i_reg_n_0_[12] 
        }),
        .S({
            \q1i[15]_i_2_n_0 ,
            \q1i[15]_i_3_n_0 ,
            \q1i[15]_i_4_n_0 ,
            \q1i[15]_i_5_n_0 
        }),
        .CO({
            \q1i_reg[15]_i_1_n_0 ,
            \q1i_reg[15]_i_1_n_1 ,
            \q1i_reg[15]_i_1_n_2 ,
            \q1i_reg[15]_i_1_n_3 
        }),
        .O({
            \q1i_reg[15]_i_1_n_4 ,
            \q1i_reg[15]_i_1_n_5 ,
            \q1i_reg[15]_i_1_n_6 ,
            \q1i_reg[15]_i_1_n_7 
        })
    );

    LUT2 #(
        .INIT(4'h9)
    ) \q1i[18]_i_3  (
        .I0(\U_FFT1/m4m7i(16) ),
        .I1(\U_FFT1/m4m7i(17) ),
        .O(\q1i[18]_i_3_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[15]_i_5  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[12] ),
        .I1(\U_FFT1/m4m7i(12) ),
        .O(\q1i[15]_i_5_n_0 )
    );

    CARRY4 \q1i_reg[11]_i_1  (
        .CI(\q1i_reg[7]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s2d3i_reg_n_0_[11] ,
            \U_FFT1/s2d3i_reg_n_0_[10] ,
            \U_FFT1/s2d3i_reg_n_0_[9] ,
            \U_FFT1/s2d3i_reg_n_0_[8] 
        }),
        .S({
            \q1i[11]_i_2_n_0 ,
            \q1i[11]_i_3_n_0 ,
            \q1i[11]_i_4_n_0 ,
            \q1i[11]_i_5_n_0 
        }),
        .CO({
            \q1i_reg[11]_i_1_n_0 ,
            \q1i_reg[11]_i_1_n_1 ,
            \q1i_reg[11]_i_1_n_2 ,
            \q1i_reg[11]_i_1_n_3 
        }),
        .O({
            \q1i_reg[11]_i_1_n_4 ,
            \q1i_reg[11]_i_1_n_5 ,
            \q1i_reg[11]_i_1_n_6 ,
            \q1i_reg[11]_i_1_n_7 
        })
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[15]_i_4  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[13] ),
        .I1(\U_FFT1/m4m7i(13) ),
        .O(\q1i[15]_i_4_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[7]_i_5  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[4] ),
        .I1(\U_FFT1/m4m7i(4) ),
        .O(\q1i[7]_i_5_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[15]_i_3  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[14] ),
        .I1(\U_FFT1/m4m7i(14) ),
        .O(\q1i[15]_i_3_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[15]_i_2  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[15] ),
        .I1(\U_FFT1/m4m7i(15) ),
        .O(\q1i[15]_i_2_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[11]_i_5  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[8] ),
        .I1(\U_FFT1/m4m7i(8) ),
        .O(\q1i[11]_i_5_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[7]_i_4  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[5] ),
        .I1(\U_FFT1/m4m7i(5) ),
        .O(\q1i[7]_i_4_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[7]_i_3  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[6] ),
        .I1(\U_FFT1/m4m7i(6) ),
        .O(\q1i[7]_i_3_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[7]_i_2  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[7] ),
        .I1(\U_FFT1/m4m7i(7) ),
        .O(\q1i[7]_i_2_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[11]_i_4  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[9] ),
        .I1(\U_FFT1/m4m7i(9) ),
        .O(\q1i[11]_i_4_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[3]_i_5  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[0] ),
        .I1(\U_FFT1/m4m7i(0) ),
        .O(\q1i[3]_i_5_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[11]_i_3  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[10] ),
        .I1(\U_FFT1/m4m7i(10) ),
        .O(\q1i[11]_i_3_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[3]_i_4  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[1] ),
        .I1(\U_FFT1/m4m7i(1) ),
        .O(\q1i[3]_i_4_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[11]_i_2  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[11] ),
        .I1(\U_FFT1/m4m7i(11) ),
        .O(\q1i[11]_i_2_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[3]_i_3  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[2] ),
        .I1(\U_FFT1/m4m7i(2) ),
        .O(\q1i[3]_i_3_n_0 )
    );

    LUT2 #(
        .INIT(4'h6)
    ) \q1i[3]_i_2  (
        .I0(\U_FFT1/s2d3i_reg_n_0_[3] ),
        .I1(\U_FFT1/m4m7i(3) ),
        .O(\q1i[3]_i_2_n_0 )
    );
endmodule

