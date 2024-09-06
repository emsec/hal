`timescale 1 ps/1 ps
module top(\simple_cpu_inst/pc_reg_n_0_[11] ,\simple_cpu_inst/pc_reg_n_0_[7] ,\simple_cpu_inst/pc_reg_n_0_[14] ,\simple_cpu_inst/pc_reg_n_0_[6] ,\simple_cpu_inst/pc_reg_n_0_[1] ,\simple_cpu_inst/pc_reg_n_0_[3] ,\simple_cpu_inst/pc_reg_n_0_[31] ,\simple_cpu_inst/pc_reg_n_0_[15] ,\simple_cpu_inst/pc_reg_n_0_[9] ,\simple_cpu_inst/pc_reg_n_0_[8] ,\simple_cpu_inst/pc_reg_n_0_[13] ,\simple_cpu_inst/pc_reg_n_0_[4] ,\simple_cpu_inst/pc_reg_n_0_[21] ,\simple_cpu_inst/pc_reg_n_0_[17] ,\simple_cpu_inst/pc_reg_n_0_[10] ,\simple_cpu_inst/pc_reg_n_0_[20] ,\simple_cpu_inst/pc_reg_n_0_[18] ,\simple_cpu_inst/pc_reg_n_0_[28] ,\simple_cpu_inst/pc_reg_n_0_[29] ,\simple_cpu_inst/pc_reg_n_0_[12] ,\simple_cpu_inst/pc_reg_n_0_[27] ,\simple_cpu_inst/pc_reg_n_0_[16] ,\simple_cpu_inst/pc_reg_n_0_[5] ,\simple_cpu_inst/pc_reg_n_0_[22] ,\simple_cpu_inst/pc_reg_n_0_[25] ,\simple_cpu_inst/pc_reg_n_0_[23] ,\simple_cpu_inst/pc_reg_n_0_[19] ,\simple_cpu_inst/pc_reg_n_0_[26] ,\simple_cpu_inst/pc_reg_n_0_[24] ,\simple_cpu_inst/pc_reg_n_0_[2] ,\simple_cpu_inst/pc_reg_n_0_[30] ,\pc_reg[8]_i_4_n_7 ,\pc_reg[12]_i_3_n_4 ,\pc_reg[4]_i_3_n_4 ,\pc_reg[24]_i_3_n_7 ,\pc_reg[8]_i_4_n_4 ,\pc_reg[20]_i_3_n_6 ,\pc_reg[8]_i_4_n_5 ,\pc_reg[4]_i_3_n_7 ,\pc_reg[28]_i_3_n_5 ,\pc_reg[8]_i_4_n_6 ,\pc_reg[31]_i_10_n_7 ,\pc_reg[4]_i_3_n_6 ,\pc_reg[16]_i_4_n_6 ,\pc_reg[16]_i_4_n_4 ,\pc_reg[12]_i_3_n_5 ,\pc_reg[12]_i_3_n_6 ,\pc_reg[4]_i_3_n_5 ,\pc_reg[28]_i_3_n_4 ,\pc_reg[16]_i_4_n_5 ,\pc_reg[24]_i_3_n_5 ,\pc_reg[16]_i_4_n_7 ,\pc_reg[20]_i_3_n_5 ,\pc_reg[20]_i_3_n_4 ,\pc_reg[12]_i_3_n_7 ,\pc_reg[20]_i_3_n_7 ,\pc_reg[24]_i_3_n_4 ,\pc_reg[24]_i_3_n_6 ,\pc_reg[28]_i_3_n_6 ,\pc_reg[31]_i_10_n_5 ,\pc_reg[31]_i_10_n_6 ,\pc_reg[28]_i_3_n_7 );
    input \simple_cpu_inst/pc_reg_n_0_[11] ;
    input \simple_cpu_inst/pc_reg_n_0_[7] ;
    input \simple_cpu_inst/pc_reg_n_0_[14] ;
    input \simple_cpu_inst/pc_reg_n_0_[6] ;
    input \simple_cpu_inst/pc_reg_n_0_[1] ;
    input \simple_cpu_inst/pc_reg_n_0_[3] ;
    input \simple_cpu_inst/pc_reg_n_0_[31] ;
    input \simple_cpu_inst/pc_reg_n_0_[15] ;
    input \simple_cpu_inst/pc_reg_n_0_[9] ;
    input \simple_cpu_inst/pc_reg_n_0_[8] ;
    input \simple_cpu_inst/pc_reg_n_0_[13] ;
    input \simple_cpu_inst/pc_reg_n_0_[4] ;
    input \simple_cpu_inst/pc_reg_n_0_[21] ;
    input \simple_cpu_inst/pc_reg_n_0_[17] ;
    input \simple_cpu_inst/pc_reg_n_0_[10] ;
    input \simple_cpu_inst/pc_reg_n_0_[20] ;
    input \simple_cpu_inst/pc_reg_n_0_[18] ;
    input \simple_cpu_inst/pc_reg_n_0_[28] ;
    input \simple_cpu_inst/pc_reg_n_0_[29] ;
    input \simple_cpu_inst/pc_reg_n_0_[12] ;
    input \simple_cpu_inst/pc_reg_n_0_[27] ;
    input \simple_cpu_inst/pc_reg_n_0_[16] ;
    input \simple_cpu_inst/pc_reg_n_0_[5] ;
    input \simple_cpu_inst/pc_reg_n_0_[22] ;
    input \simple_cpu_inst/pc_reg_n_0_[25] ;
    input \simple_cpu_inst/pc_reg_n_0_[23] ;
    input \simple_cpu_inst/pc_reg_n_0_[19] ;
    input \simple_cpu_inst/pc_reg_n_0_[26] ;
    input \simple_cpu_inst/pc_reg_n_0_[24] ;
    input \simple_cpu_inst/pc_reg_n_0_[2] ;
    input \simple_cpu_inst/pc_reg_n_0_[30] ;
    output \pc_reg[8]_i_4_n_7 ;
    output \pc_reg[12]_i_3_n_4 ;
    output \pc_reg[4]_i_3_n_4 ;
    output \pc_reg[24]_i_3_n_7 ;
    output \pc_reg[8]_i_4_n_4 ;
    output \pc_reg[20]_i_3_n_6 ;
    output \pc_reg[8]_i_4_n_5 ;
    output \pc_reg[4]_i_3_n_7 ;
    output \pc_reg[28]_i_3_n_5 ;
    output \pc_reg[8]_i_4_n_6 ;
    output \pc_reg[31]_i_10_n_7 ;
    output \pc_reg[4]_i_3_n_6 ;
    output \pc_reg[16]_i_4_n_6 ;
    output \pc_reg[16]_i_4_n_4 ;
    output \pc_reg[12]_i_3_n_5 ;
    output \pc_reg[12]_i_3_n_6 ;
    output \pc_reg[4]_i_3_n_5 ;
    output \pc_reg[28]_i_3_n_4 ;
    output \pc_reg[16]_i_4_n_5 ;
    output \pc_reg[24]_i_3_n_5 ;
    output \pc_reg[16]_i_4_n_7 ;
    output \pc_reg[20]_i_3_n_5 ;
    output \pc_reg[20]_i_3_n_4 ;
    output \pc_reg[12]_i_3_n_7 ;
    output \pc_reg[20]_i_3_n_7 ;
    output \pc_reg[24]_i_3_n_4 ;
    output \pc_reg[24]_i_3_n_6 ;
    output \pc_reg[28]_i_3_n_6 ;
    output \pc_reg[31]_i_10_n_5 ;
    output \pc_reg[31]_i_10_n_6 ;
    output \pc_reg[28]_i_3_n_7 ;
    wire \pc_reg[4]_i_3_n_0 ;
    wire \pc_reg[24]_i_3_n_2 ;
    wire \pc_reg[4]_i_3_n_2 ;
    wire \pc_reg[8]_i_4_n_2 ;
    wire \pc_reg[16]_i_4_n_0 ;
    wire i__carry_i_3__1_n_0;
    wire \pc_reg[12]_i_3_n_0 ;
    wire \pc_reg[24]_i_3_n_0 ;
    wire \pc_reg[8]_i_4_n_1 ;
    wire \pc_reg[28]_i_3_n_1 ;
    wire \pc_reg[4]_i_3_n_1 ;
    wire \pc_reg[8]_i_4_n_0 ;
    wire \pc_reg[12]_i_3_n_3 ;
    wire \pc_reg[12]_i_3_n_1 ;
    wire \pc_reg[16]_i_4_n_1 ;
    wire \pc_reg[16]_i_4_n_2 ;
    wire \pc_reg[16]_i_4_n_3 ;
    wire \pc_reg[28]_i_3_n_2 ;
    wire \pc_reg[8]_i_4_n_3 ;
    wire \pc_reg[12]_i_3_n_2 ;
    wire \pc_reg[31]_i_10_n_2 ;
    wire \pc_reg[28]_i_3_n_0 ;
    wire \pc_reg[31]_i_10_n_3 ;
    wire \pc_reg[20]_i_3_n_2 ;
    wire \pc_reg[20]_i_3_n_0 ;
    wire \pc_reg[24]_i_3_n_1 ;
    wire \pc_reg[20]_i_3_n_3 ;
    wire \pc_reg[24]_i_3_n_3 ;
    wire \pc_reg[28]_i_3_n_3 ;
    wire \pc_reg[20]_i_3_n_1 ;

    CARRY4 \pc_reg[8]_i_4  (
        .CI(\pc_reg[4]_i_3_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            1'b0 ,
            1'b0 
        }),
        .S({
            \simple_cpu_inst/pc_reg_n_0_[8] ,
            \simple_cpu_inst/pc_reg_n_0_[7] ,
            \simple_cpu_inst/pc_reg_n_0_[6] ,
            \simple_cpu_inst/pc_reg_n_0_[5] 
        }),
        .CO({
            \pc_reg[8]_i_4_n_0 ,
            \pc_reg[8]_i_4_n_1 ,
            \pc_reg[8]_i_4_n_2 ,
            \pc_reg[8]_i_4_n_3 
        }),
        .O({
            \pc_reg[8]_i_4_n_4 ,
            \pc_reg[8]_i_4_n_5 ,
            \pc_reg[8]_i_4_n_6 ,
            \pc_reg[8]_i_4_n_7 
        })
    );

    CARRY4 \pc_reg[4]_i_3  (
        .CI(1'b0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            \simple_cpu_inst/pc_reg_n_0_[2] ,
            1'b0 
        }),
        .S({
            \simple_cpu_inst/pc_reg_n_0_[4] ,
            \simple_cpu_inst/pc_reg_n_0_[3] ,
            i__carry_i_3__1_n_0,
            \simple_cpu_inst/pc_reg_n_0_[1] 
        }),
        .CO({
            \pc_reg[4]_i_3_n_0 ,
            \pc_reg[4]_i_3_n_1 ,
            \pc_reg[4]_i_3_n_2 ,
            1'bz
        }),
        .O({
            \pc_reg[4]_i_3_n_4 ,
            \pc_reg[4]_i_3_n_5 ,
            \pc_reg[4]_i_3_n_6 ,
            \pc_reg[4]_i_3_n_7 
        })
    );

    LUT1 #(
        .INIT(1'b1)
    ) i__carry_i_3__1 (
        .I0(\simple_cpu_inst/pc_reg_n_0_[2] ),
        .O(i__carry_i_3__1_n_0)
    );

    CARRY4 \pc_reg[16]_i_4  (
        .CI(\pc_reg[12]_i_3_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            1'b0 ,
            1'b0 
        }),
        .S({
            \simple_cpu_inst/pc_reg_n_0_[16] ,
            \simple_cpu_inst/pc_reg_n_0_[15] ,
            \simple_cpu_inst/pc_reg_n_0_[14] ,
            \simple_cpu_inst/pc_reg_n_0_[13] 
        }),
        .CO({
            \pc_reg[16]_i_4_n_0 ,
            \pc_reg[16]_i_4_n_1 ,
            \pc_reg[16]_i_4_n_2 ,
            \pc_reg[16]_i_4_n_3 
        }),
        .O({
            \pc_reg[16]_i_4_n_4 ,
            \pc_reg[16]_i_4_n_5 ,
            \pc_reg[16]_i_4_n_6 ,
            \pc_reg[16]_i_4_n_7 
        })
    );

    CARRY4 \pc_reg[12]_i_3  (
        .CI(\pc_reg[8]_i_4_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            1'b0 ,
            1'b0 
        }),
        .S({
            \simple_cpu_inst/pc_reg_n_0_[12] ,
            \simple_cpu_inst/pc_reg_n_0_[11] ,
            \simple_cpu_inst/pc_reg_n_0_[10] ,
            \simple_cpu_inst/pc_reg_n_0_[9] 
        }),
        .CO({
            \pc_reg[12]_i_3_n_0 ,
            \pc_reg[12]_i_3_n_1 ,
            \pc_reg[12]_i_3_n_2 ,
            \pc_reg[12]_i_3_n_3 
        }),
        .O({
            \pc_reg[12]_i_3_n_4 ,
            \pc_reg[12]_i_3_n_5 ,
            \pc_reg[12]_i_3_n_6 ,
            \pc_reg[12]_i_3_n_7 
        })
    );

    CARRY4 \pc_reg[31]_i_10  (
        .CI(\pc_reg[28]_i_3_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            1'b0 ,
            1'b0 
        }),
        .S({
            1'b0 ,
            \simple_cpu_inst/pc_reg_n_0_[31] ,
            \simple_cpu_inst/pc_reg_n_0_[30] ,
            \simple_cpu_inst/pc_reg_n_0_[29] 
        }),
        .CO({
            1'bz,
            1'bz,
            \pc_reg[31]_i_10_n_2 ,
            \pc_reg[31]_i_10_n_3 
        }),
        .O({
            1'bz,
            \pc_reg[31]_i_10_n_5 ,
            \pc_reg[31]_i_10_n_6 ,
            \pc_reg[31]_i_10_n_7 
        })
    );

    CARRY4 \pc_reg[28]_i_3  (
        .CI(\pc_reg[24]_i_3_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            1'b0 ,
            1'b0 
        }),
        .S({
            \simple_cpu_inst/pc_reg_n_0_[28] ,
            \simple_cpu_inst/pc_reg_n_0_[27] ,
            \simple_cpu_inst/pc_reg_n_0_[26] ,
            \simple_cpu_inst/pc_reg_n_0_[25] 
        }),
        .CO({
            \pc_reg[28]_i_3_n_0 ,
            \pc_reg[28]_i_3_n_1 ,
            \pc_reg[28]_i_3_n_2 ,
            \pc_reg[28]_i_3_n_3 
        }),
        .O({
            \pc_reg[28]_i_3_n_4 ,
            \pc_reg[28]_i_3_n_5 ,
            \pc_reg[28]_i_3_n_6 ,
            \pc_reg[28]_i_3_n_7 
        })
    );

    CARRY4 \pc_reg[24]_i_3  (
        .CI(\pc_reg[20]_i_3_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            1'b0 ,
            1'b0 
        }),
        .S({
            \simple_cpu_inst/pc_reg_n_0_[24] ,
            \simple_cpu_inst/pc_reg_n_0_[23] ,
            \simple_cpu_inst/pc_reg_n_0_[22] ,
            \simple_cpu_inst/pc_reg_n_0_[21] 
        }),
        .CO({
            \pc_reg[24]_i_3_n_0 ,
            \pc_reg[24]_i_3_n_1 ,
            \pc_reg[24]_i_3_n_2 ,
            \pc_reg[24]_i_3_n_3 
        }),
        .O({
            \pc_reg[24]_i_3_n_4 ,
            \pc_reg[24]_i_3_n_5 ,
            \pc_reg[24]_i_3_n_6 ,
            \pc_reg[24]_i_3_n_7 
        })
    );

    CARRY4 \pc_reg[20]_i_3  (
        .CI(\pc_reg[16]_i_4_n_0 ),
        .CYINIT(1'b0 ),
        .DI({
            1'b0 ,
            1'b0 ,
            1'b0 ,
            1'b0 
        }),
        .S({
            \simple_cpu_inst/pc_reg_n_0_[20] ,
            \simple_cpu_inst/pc_reg_n_0_[19] ,
            \simple_cpu_inst/pc_reg_n_0_[18] ,
            \simple_cpu_inst/pc_reg_n_0_[17] 
        }),
        .CO({
            \pc_reg[20]_i_3_n_0 ,
            \pc_reg[20]_i_3_n_1 ,
            \pc_reg[20]_i_3_n_2 ,
            \pc_reg[20]_i_3_n_3 
        }),
        .O({
            \pc_reg[20]_i_3_n_4 ,
            \pc_reg[20]_i_3_n_5 ,
            \pc_reg[20]_i_3_n_6 ,
            \pc_reg[20]_i_3_n_7 
        })
    );
endmodule

