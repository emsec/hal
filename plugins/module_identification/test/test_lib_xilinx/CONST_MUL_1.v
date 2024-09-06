`timescale 1 ps/1 ps
module USFFT64_2B(\U_FFT1/s4i_reg_n_0_[7] ,\U_FFT1/s4i_reg_n_0_[5] ,\U_FFT1/s4i_reg_n_0_[2] ,\U_FFT1/s4i_reg_n_0_[1] ,\U_FFT1/s4i_reg_n_0_[13] ,\U_FFT1/s4i_reg_n_0_[9] ,\U_FFT1/s4i_reg_n_0_[17] ,\U_FFT1/s4i_reg_n_0_[6] ,\U_FFT1/s4i_reg_n_0_[8] ,\U_FFT1/s4i_reg_n_0_[10] ,\U_FFT1/s4i_reg_n_0_[0] ,\U_FFT1/s4i_reg_n_0_[12] ,\U_FFT1/s4i_reg_n_0_[4] ,\U_FFT1/s4i_reg_n_0_[3] ,\U_FFT1/s4i_reg_n_0_[16] ,\U_FFT1/s4i_reg_n_0_[15] ,\U_FFT1/s4i_reg_n_0_[14] ,\U_FFT1/s4i_reg_n_0_[11] ,\U_FFT1/UMI/dx5_reg[4]_i_1_n_4 ,\U_FFT1/UMI/dx5_reg[4]_i_1_n_5 ,\U_FFT1/UMI/dx5_reg[21]_i_1_n_6 ,\U_FFT1/UMI/dx5_reg[4]_i_1_n_6 ,\U_FFT1/UMI/dx5_reg[8]_i_1_n_6 ,\U_FFT1/UMI/dx5_reg[12]_i_1_n_7 ,\U_FFT1/UMI/dx5_reg[21]_i_1_n_5 ,\U_FFT1/UMI/dx5_reg[12]_i_1_n_6 ,\U_FFT1/UMI/dx5_reg[21]_i_1_n_7 ,\U_FFT1/UMI/dx5_reg[21]_i_1_n_4 ,\U_FFT1/UMI/dx5_reg[8]_i_1_n_7 ,\U_FFT1/UMI/dx5_reg[4]_i_1_n_7 ,\U_FFT1/UMI/dx5_reg[12]_i_1_n_5 ,\U_FFT1/UMI/dx5_reg[12]_i_1_n_4 ,\U_FFT1/UMI/dx5_reg[8]_i_1_n_5 ,\U_FFT1/UMI/dx5_reg[16]_i_1_n_4 ,\U_FFT1/UMI/dx5_reg[16]_i_1_n_7 ,\U_FFT1/UMI/dx5_reg[16]_i_1_n_6 ,\U_FFT1/UMI/dx5_reg[16]_i_1_n_5 ,\U_FFT1/UMI/dx5_reg[8]_i_1_n_4 );
    input \U_FFT1/s4i_reg_n_0_[7] ;
    input \U_FFT1/s4i_reg_n_0_[5] ;
    input \U_FFT1/s4i_reg_n_0_[2] ;
    input \U_FFT1/s4i_reg_n_0_[1] ;
    input \U_FFT1/s4i_reg_n_0_[13] ;
    input \U_FFT1/s4i_reg_n_0_[9] ;
    input \U_FFT1/s4i_reg_n_0_[17] ;
    input \U_FFT1/s4i_reg_n_0_[6] ;
    input \U_FFT1/s4i_reg_n_0_[8] ;
    input \U_FFT1/s4i_reg_n_0_[10] ;
    input \U_FFT1/s4i_reg_n_0_[0] ;
    input \U_FFT1/s4i_reg_n_0_[12] ;
    input \U_FFT1/s4i_reg_n_0_[4] ;
    input \U_FFT1/s4i_reg_n_0_[3] ;
    input \U_FFT1/s4i_reg_n_0_[16] ;
    input \U_FFT1/s4i_reg_n_0_[15] ;
    input \U_FFT1/s4i_reg_n_0_[14] ;
    input \U_FFT1/s4i_reg_n_0_[11] ;
    output \U_FFT1/UMI/dx5_reg[4]_i_1_n_4 ;
    output \U_FFT1/UMI/dx5_reg[4]_i_1_n_5 ;
    output \U_FFT1/UMI/dx5_reg[21]_i_1_n_6 ;
    output \U_FFT1/UMI/dx5_reg[4]_i_1_n_6 ;
    output \U_FFT1/UMI/dx5_reg[8]_i_1_n_6 ;
    output \U_FFT1/UMI/dx5_reg[12]_i_1_n_7 ;
    output \U_FFT1/UMI/dx5_reg[21]_i_1_n_5 ;
    output \U_FFT1/UMI/dx5_reg[12]_i_1_n_6 ;
    output \U_FFT1/UMI/dx5_reg[21]_i_1_n_7 ;
    output \U_FFT1/UMI/dx5_reg[21]_i_1_n_4 ;
    output \U_FFT1/UMI/dx5_reg[8]_i_1_n_7 ;
    output \U_FFT1/UMI/dx5_reg[4]_i_1_n_7 ;
    output \U_FFT1/UMI/dx5_reg[12]_i_1_n_5 ;
    output \U_FFT1/UMI/dx5_reg[12]_i_1_n_4 ;
    output \U_FFT1/UMI/dx5_reg[8]_i_1_n_5 ;
    output \U_FFT1/UMI/dx5_reg[16]_i_1_n_4 ;
    output \U_FFT1/UMI/dx5_reg[16]_i_1_n_7 ;
    output \U_FFT1/UMI/dx5_reg[16]_i_1_n_6 ;
    output \U_FFT1/UMI/dx5_reg[16]_i_1_n_5 ;
    output \U_FFT1/UMI/dx5_reg[8]_i_1_n_4 ;
    wire \U_FFT1/UMI/dx5[8]_i_4_n_0 ;
    wire \U_FFT1/UMI/dx5[8]_i_5_n_0 ;
    wire \U_FFT1/UMI/dx5[8]_i_2_n_0 ;
    wire \U_FFT1/UMI/dx5[12]_i_2_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[4]_i_1_n_2 ;
    wire \U_FFT1/UMI/dx5_reg[4]_i_1_n_1 ;
    wire \U_FFT1/UMI/dx5_reg[4]_i_1_n_0 ;
    wire \U_FFT1/UMI/dx5[4]_i_4_n_0 ;
    wire \U_FFT1/UMI/dx5[4]_i_3_n_0 ;
    wire \U_FFT1/UMI/dx5[12]_i_4_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[12]_i_1_n_0 ;
    wire \U_FFT1/UMI/dx5[12]_i_3_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[16]_i_1_n_1 ;
    wire \U_FFT1/UMI/dx5[21]_i_2_n_0 ;
    wire \U_FFT1/UMI/dx5[4]_i_2_n_0 ;
    wire \dx5[21]_i_3_n_0 ;
    wire \dx5[21]_i_4_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[21]_i_1_n_1 ;
    wire \U_FFT1/UMI/dx5_reg[12]_i_1_n_3 ;
    wire \U_FFT1/UMI/dx5_reg[12]_i_1_n_2 ;
    wire \U_FFT1/UMI/dx5_reg[16]_i_1_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[8]_i_1_n_2 ;
    wire \U_FFT1/UMI/dx5[16]_i_4_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[16]_i_1_n_2 ;
    wire \U_FFT1/UMI/dx5[12]_i_5_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[16]_i_1_n_3 ;
    wire \U_FFT1/UMI/dx5[8]_i_3_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[12]_i_1_n_1 ;
    wire \U_FFT1/UMI/dx5_reg[8]_i_1_n_0 ;
    wire \U_FFT1/UMI/dx5[16]_i_2_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[8]_i_1_n_1 ;
    wire \U_FFT1/UMI/dx5[16]_i_3_n_0 ;
    wire \dx5[21]_i_5_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[8]_i_1_n_3 ;
    wire \U_FFT1/UMI/dx5[16]_i_5_n_0 ;
    wire \U_FFT1/UMI/dx5_reg[21]_i_1_n_2 ;
    wire \U_FFT1/UMI/dx5_reg[21]_i_1_n_3 ;

    HAL_INV _2__71_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[15] ),
        .O(\U_FFT1/UMI/dx5[21]_i_2_n_0 )
    );

    HAL_XOR2 _7__171_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[12] ),
        .B(\U_FFT1/s4i_reg_n_0_[10] ),
        .O(\U_FFT1/UMI/dx5[12]_i_2_n_0 )
    );

    CARRY4 \U_FFT1/UMI/dx5_reg[16]_i_1  (
        .CI(\U_FFT1/UMI/dx5_reg[12]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s4i_reg_n_0_[16] ,
            \U_FFT1/s4i_reg_n_0_[15] ,
            \U_FFT1/s4i_reg_n_0_[14] ,
            \U_FFT1/s4i_reg_n_0_[13] 
        }),
        .S({
            \U_FFT1/UMI/dx5[16]_i_2_n_0 ,
            \U_FFT1/UMI/dx5[16]_i_3_n_0 ,
            \U_FFT1/UMI/dx5[16]_i_4_n_0 ,
            \U_FFT1/UMI/dx5[16]_i_5_n_0 
        }),
        .CO({
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_0 ,
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_1 ,
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_2 ,
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_3 
        }),
        .O({
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_4 ,
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_5 ,
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_6 ,
            \U_FFT1/UMI/dx5_reg[16]_i_1_n_7 
        })
    );

    HAL_XOR2 _7__174_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[9] ),
        .B(\U_FFT1/s4i_reg_n_0_[7] ),
        .O(\U_FFT1/UMI/dx5[12]_i_5_n_0 )
    );

    CARRY4 \U_FFT1/UMI/dx5_reg[21]_i_1  (
        .CI(\U_FFT1/UMI/dx5_reg[16]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            \U_FFT1/s4i_reg_n_0_[16] ,
            \U_FFT1/s4i_reg_n_0_[15] ,
            \U_FFT1/UMI/dx5[21]_i_2_n_0 
        }),
        .S({
            1'b1 ,
            \dx5[21]_i_3_n_0 ,
            \dx5[21]_i_4_n_0 ,
            \dx5[21]_i_5_n_0 
        }),
        .CO({
            1'bz,
            \U_FFT1/UMI/dx5_reg[21]_i_1_n_1 ,
            \U_FFT1/UMI/dx5_reg[21]_i_1_n_2 ,
            \U_FFT1/UMI/dx5_reg[21]_i_1_n_3 
        }),
        .O({
            \U_FFT1/UMI/dx5_reg[21]_i_1_n_4 ,
            \U_FFT1/UMI/dx5_reg[21]_i_1_n_5 ,
            \U_FFT1/UMI/dx5_reg[21]_i_1_n_6 ,
            \U_FFT1/UMI/dx5_reg[21]_i_1_n_7 
        })
    );

    HAL_XOR2 _7__175_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[16] ),
        .B(\U_FFT1/s4i_reg_n_0_[14] ),
        .O(\U_FFT1/UMI/dx5[16]_i_2_n_0 )
    );

    HAL_XOR2 _7__172_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[11] ),
        .B(\U_FFT1/s4i_reg_n_0_[9] ),
        .O(\U_FFT1/UMI/dx5[12]_i_3_n_0 )
    );

    HAL_XOR2 _7__173_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[10] ),
        .B(\U_FFT1/s4i_reg_n_0_[8] ),
        .O(\U_FFT1/UMI/dx5[12]_i_4_n_0 )
    );

    CARRY4 \U_FFT1/UMI/dx5_reg[4]_i_1  (
        .CI(1'b0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s4i_reg_n_0_[4] ,
            \U_FFT1/s4i_reg_n_0_[3] ,
            \U_FFT1/s4i_reg_n_0_[2] ,
            1'b0 
        }),
        .S({
            \U_FFT1/UMI/dx5[4]_i_2_n_0 ,
            \U_FFT1/UMI/dx5[4]_i_3_n_0 ,
            \U_FFT1/UMI/dx5[4]_i_4_n_0 ,
            \U_FFT1/s4i_reg_n_0_[1] 
        }),
        .CO({
            \U_FFT1/UMI/dx5_reg[4]_i_1_n_0 ,
            \U_FFT1/UMI/dx5_reg[4]_i_1_n_1 ,
            \U_FFT1/UMI/dx5_reg[4]_i_1_n_2 ,
            1'bz
        }),
        .O({
            \U_FFT1/UMI/dx5_reg[4]_i_1_n_4 ,
            \U_FFT1/UMI/dx5_reg[4]_i_1_n_5 ,
            \U_FFT1/UMI/dx5_reg[4]_i_1_n_6 ,
            \U_FFT1/UMI/dx5_reg[4]_i_1_n_7 
        })
    );

    CARRY4 \U_FFT1/UMI/dx5_reg[8]_i_1  (
        .CI(\U_FFT1/UMI/dx5_reg[4]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s4i_reg_n_0_[8] ,
            \U_FFT1/s4i_reg_n_0_[7] ,
            \U_FFT1/s4i_reg_n_0_[6] ,
            \U_FFT1/s4i_reg_n_0_[5] 
        }),
        .S({
            \U_FFT1/UMI/dx5[8]_i_2_n_0 ,
            \U_FFT1/UMI/dx5[8]_i_3_n_0 ,
            \U_FFT1/UMI/dx5[8]_i_4_n_0 ,
            \U_FFT1/UMI/dx5[8]_i_5_n_0 
        }),
        .CO({
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_0 ,
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_1 ,
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_2 ,
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_3 
        }),
        .O({
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_4 ,
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_5 ,
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_6 ,
            \U_FFT1/UMI/dx5_reg[8]_i_1_n_7 
        })
    );

    HAL_XOR2 _7__178_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[13] ),
        .B(\U_FFT1/s4i_reg_n_0_[11] ),
        .O(\U_FFT1/UMI/dx5[16]_i_5_n_0 )
    );

    HAL_XOR2 _7__179_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[4] ),
        .B(\U_FFT1/s4i_reg_n_0_[2] ),
        .O(\U_FFT1/UMI/dx5[4]_i_2_n_0 )
    );

    HAL_XOR2 _7__176_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[15] ),
        .B(\U_FFT1/s4i_reg_n_0_[13] ),
        .O(\U_FFT1/UMI/dx5[16]_i_3_n_0 )
    );

    CARRY4 \U_FFT1/UMI/dx5_reg[12]_i_1  (
        .CI(\U_FFT1/UMI/dx5_reg[8]_i_1_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            \U_FFT1/s4i_reg_n_0_[12] ,
            \U_FFT1/s4i_reg_n_0_[11] ,
            \U_FFT1/s4i_reg_n_0_[10] ,
            \U_FFT1/s4i_reg_n_0_[9] 
        }),
        .S({
            \U_FFT1/UMI/dx5[12]_i_2_n_0 ,
            \U_FFT1/UMI/dx5[12]_i_3_n_0 ,
            \U_FFT1/UMI/dx5[12]_i_4_n_0 ,
            \U_FFT1/UMI/dx5[12]_i_5_n_0 
        }),
        .CO({
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_0 ,
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_1 ,
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_2 ,
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_3 
        }),
        .O({
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_4 ,
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_5 ,
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_6 ,
            \U_FFT1/UMI/dx5_reg[12]_i_1_n_7 
        })
    );

    HAL_XOR2 _7__177_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[14] ),
        .B(\U_FFT1/s4i_reg_n_0_[12] ),
        .O(\U_FFT1/UMI/dx5[16]_i_4_n_0 )
    );

    HAL_XOR2 _7__182_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[8] ),
        .B(\U_FFT1/s4i_reg_n_0_[6] ),
        .O(\U_FFT1/UMI/dx5[8]_i_2_n_0 )
    );

    HAL_XNOR2 _7__3350_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[16] ),
        .B(\U_FFT1/s4i_reg_n_0_[15] ),
        .O(\dx5[21]_i_4_n_0 )
    );

    HAL_XOR2 _7__3351_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[15] ),
        .B(\U_FFT1/s4i_reg_n_0_[17] ),
        .O(\dx5[21]_i_5_n_0 )
    );

    HAL_XOR2 _7__183_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[7] ),
        .B(\U_FFT1/s4i_reg_n_0_[5] ),
        .O(\U_FFT1/UMI/dx5[8]_i_3_n_0 )
    );

    HAL_XOR2 _7__180_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[3] ),
        .B(\U_FFT1/s4i_reg_n_0_[1] ),
        .O(\U_FFT1/UMI/dx5[4]_i_3_n_0 )
    );

    HAL_XOR2 _7__181_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[2] ),
        .B(\U_FFT1/s4i_reg_n_0_[0] ),
        .O(\U_FFT1/UMI/dx5[4]_i_4_n_0 )
    );

    HAL_XNOR2 _7__3349_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[17] ),
        .B(\U_FFT1/s4i_reg_n_0_[16] ),
        .O(\dx5[21]_i_3_n_0 )
    );

    HAL_XOR2 _7__184_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[6] ),
        .B(\U_FFT1/s4i_reg_n_0_[4] ),
        .O(\U_FFT1/UMI/dx5[8]_i_4_n_0 )
    );

    HAL_XOR2 _7__185_RESYNTH_GATE (
        .A(\U_FFT1/s4i_reg_n_0_[5] ),
        .B(\U_FFT1/s4i_reg_n_0_[3] ),
        .O(\U_FFT1/UMI/dx5[8]_i_5_n_0 )
    );
endmodule

