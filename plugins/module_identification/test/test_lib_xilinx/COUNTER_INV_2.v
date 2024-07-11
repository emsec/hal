`timescale 1 ps/1 ps
module top(\icicle/uart/rx_clks_reg_n_0_[10] ,\icicle/uart/rx_clks_reg_n_0_[0] ,\icicle/uart/rx_clks_reg_n_0_[12] ,\icicle/uart/rx_clks_reg_n_0_[9] ,\icicle/uart/rx_clks_reg_n_0_[7] ,\icicle/uart/rx_clks_reg_n_0_[1] ,\icicle/uart/rx_clks_reg_n_0_[2] ,\icicle/uart/rx_clks_reg_n_0_[3] ,\icicle/uart/rx_clks_reg_n_0_[15] ,\icicle/uart/rx_clks_reg_n_0_[5] ,\icicle/uart/rx_clks_reg_n_0_[4] ,\icicle/uart/rx_clks_reg_n_0_[8] ,\icicle/uart/rx_clks_reg_n_0_[14] ,\icicle/uart/rx_clks_reg_n_0_[6] ,\icicle/uart/rx_clks_reg_n_0_[11] ,\icicle/uart/rx_clks_reg_n_0_[13] ,\icicle/uart/rx_clks0__28(3) ,\icicle/uart/rx_clks0__28(2) ,\icicle/uart/rx_clks0__28(1) ,\icicle/uart/rx_clks0__28(8) ,\icicle/uart/rx_clks0__28(15) ,\icicle/uart/rx_clks0__28(6) ,\icicle/uart/rx_clks0__28(7) ,\icicle/uart/rx_clks0__28(4) ,\icicle/uart/rx_clks0__28(10) ,\icicle/uart/rx_clks0__28(5) ,\icicle/uart/rx_clks0__28(13) ,\icicle/uart/rx_clks0__28(11) ,\icicle/uart/rx_clks0__28(9) ,\icicle/uart/rx_clks0__28(12) ,\icicle/uart/rx_clks0__28(14) );
    input \icicle/uart/rx_clks_reg_n_0_[10] ;
    input \icicle/uart/rx_clks_reg_n_0_[0] ;
    input \icicle/uart/rx_clks_reg_n_0_[12] ;
    input \icicle/uart/rx_clks_reg_n_0_[9] ;
    input \icicle/uart/rx_clks_reg_n_0_[7] ;
    input \icicle/uart/rx_clks_reg_n_0_[1] ;
    input \icicle/uart/rx_clks_reg_n_0_[2] ;
    input \icicle/uart/rx_clks_reg_n_0_[3] ;
    input \icicle/uart/rx_clks_reg_n_0_[15] ;
    input \icicle/uart/rx_clks_reg_n_0_[5] ;
    input \icicle/uart/rx_clks_reg_n_0_[4] ;
    input \icicle/uart/rx_clks_reg_n_0_[8] ;
    input \icicle/uart/rx_clks_reg_n_0_[14] ;
    input \icicle/uart/rx_clks_reg_n_0_[6] ;
    input \icicle/uart/rx_clks_reg_n_0_[11] ;
    input \icicle/uart/rx_clks_reg_n_0_[13] ;
    output \icicle/uart/rx_clks0__28(3) ;
    output \icicle/uart/rx_clks0__28(2) ;
    output \icicle/uart/rx_clks0__28(1) ;
    output \icicle/uart/rx_clks0__28(8) ;
    output \icicle/uart/rx_clks0__28(15) ;
    output \icicle/uart/rx_clks0__28(6) ;
    output \icicle/uart/rx_clks0__28(7) ;
    output \icicle/uart/rx_clks0__28(4) ;
    output \icicle/uart/rx_clks0__28(10) ;
    output \icicle/uart/rx_clks0__28(5) ;
    output \icicle/uart/rx_clks0__28(13) ;
    output \icicle/uart/rx_clks0__28(11) ;
    output \icicle/uart/rx_clks0__28(9) ;
    output \icicle/uart/rx_clks0__28(12) ;
    output \icicle/uart/rx_clks0__28(14) ;
    wire rx_clks0_carry__2_i_2_n_0;
    wire \icicle/uart/rx_clks0_carry__2_n_2 ;
    wire rx_clks0_carry_i_3_n_0;
    wire rx_clks0_carry_i_1_n_0;
    wire rx_clks0_carry_i_4_n_0;
    wire rx_clks0_carry_i_2_n_0;
    wire rx_clks0_carry__1_i_3_n_0;
    wire \icicle/uart/rx_clks0_carry__2_n_3 ;
    wire rx_clks0_carry__1_i_1_n_0;
    wire \icicle/uart/rx_clks0_carry_n_1 ;
    wire rx_clks0_carry__0_i_4_n_0;
    wire \icicle/uart/rx_clks0_carry_n_2 ;
    wire rx_clks0_carry__0_i_1_n_0;
    wire rx_clks0_carry__0_i_2_n_0;
    wire rx_clks0_carry__0_i_3_n_0;
    wire \icicle/uart/rx_clks0_carry_n_0 ;
    wire \icicle/uart/rx_clks0_carry__0_n_1 ;
    wire \icicle/uart/rx_clks0_carry__0_n_3 ;
    wire \icicle/uart/rx_clks0_carry__1_n_0 ;
    wire rx_clks0_carry__1_i_2_n_0;
    wire \icicle/uart/rx_clks0_carry__0_n_0 ;
    wire rx_clks0_carry__1_i_4_n_0;
    wire \icicle/uart/rx_clks0_carry_n_3 ;
    wire \icicle/uart/rx_clks0_carry__1_n_3 ;
    wire rx_clks0_carry__2_i_1_n_0;
    wire \icicle/uart/rx_clks0_carry__1_n_1 ;
    wire \icicle/uart/rx_clks0_carry__1_n_2 ;
    wire \icicle/uart/rx_clks0_carry__0_n_2 ;
    wire rx_clks0_carry__2_i_3_n_0;

    CARRY4 \icicle/uart/rx_clks0_carry  (
        .CI(1'b0),
        .CYINIT(\icicle/uart/rx_clks_reg_n_0_[0] ),
        .DI({
            \icicle/uart/rx_clks_reg_n_0_[4] ,
            \icicle/uart/rx_clks_reg_n_0_[3] ,
            \icicle/uart/rx_clks_reg_n_0_[2] ,
            \icicle/uart/rx_clks_reg_n_0_[1] 
        }),
        .S({
            rx_clks0_carry_i_1_n_0,
            rx_clks0_carry_i_2_n_0,
            rx_clks0_carry_i_3_n_0,
            rx_clks0_carry_i_4_n_0
        }),
        .CO({
            \icicle/uart/rx_clks0_carry_n_0 ,
            \icicle/uart/rx_clks0_carry_n_1 ,
            \icicle/uart/rx_clks0_carry_n_2 ,
            \icicle/uart/rx_clks0_carry_n_3 
        }),
        .O({
            \icicle/uart/rx_clks0__28(4) ,
            \icicle/uart/rx_clks0__28(3) ,
            \icicle/uart/rx_clks0__28(2) ,
            \icicle/uart/rx_clks0__28(1) 
        })
    );

    HAL_INV g0_1616_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[14] ),
        .O(rx_clks0_carry__2_i_2_n_0)
    );

    CARRY4 \icicle/uart/rx_clks0_carry__0  (
        .CI(\icicle/uart/rx_clks0_carry_n_0 ),
        .CYINIT(1'b0),
        .DI({
            \icicle/uart/rx_clks_reg_n_0_[8] ,
            \icicle/uart/rx_clks_reg_n_0_[7] ,
            \icicle/uart/rx_clks_reg_n_0_[6] ,
            \icicle/uart/rx_clks_reg_n_0_[5] 
        }),
        .S({
            rx_clks0_carry__0_i_1_n_0,
            rx_clks0_carry__0_i_2_n_0,
            rx_clks0_carry__0_i_3_n_0,
            rx_clks0_carry__0_i_4_n_0
        }),
        .CO({
            \icicle/uart/rx_clks0_carry__0_n_0 ,
            \icicle/uart/rx_clks0_carry__0_n_1 ,
            \icicle/uart/rx_clks0_carry__0_n_2 ,
            \icicle/uart/rx_clks0_carry__0_n_3 
        }),
        .O({
            \icicle/uart/rx_clks0__28(8) ,
            \icicle/uart/rx_clks0__28(7) ,
            \icicle/uart/rx_clks0__28(6) ,
            \icicle/uart/rx_clks0__28(5) 
        })
    );

    CARRY4 \icicle/uart/rx_clks0_carry__1  (
        .CI(\icicle/uart/rx_clks0_carry__0_n_0 ),
        .CYINIT(1'b0),
        .DI({
            \icicle/uart/rx_clks_reg_n_0_[12] ,
            \icicle/uart/rx_clks_reg_n_0_[11] ,
            \icicle/uart/rx_clks_reg_n_0_[10] ,
            \icicle/uart/rx_clks_reg_n_0_[9] 
        }),
        .S({
            rx_clks0_carry__1_i_1_n_0,
            rx_clks0_carry__1_i_2_n_0,
            rx_clks0_carry__1_i_3_n_0,
            rx_clks0_carry__1_i_4_n_0
        }),
        .CO({
            \icicle/uart/rx_clks0_carry__1_n_0 ,
            \icicle/uart/rx_clks0_carry__1_n_1 ,
            \icicle/uart/rx_clks0_carry__1_n_2 ,
            \icicle/uart/rx_clks0_carry__1_n_3 
        }),
        .O({
            \icicle/uart/rx_clks0__28(12) ,
            \icicle/uart/rx_clks0__28(11) ,
            \icicle/uart/rx_clks0__28(10) ,
            \icicle/uart/rx_clks0__28(9) 
        })
    );

    CARRY4 \icicle/uart/rx_clks0_carry__2  (
        .CI(\icicle/uart/rx_clks0_carry__1_n_0 ),
        .CYINIT(1'b0),
        .DI({
            1'b0,
            1'b0,
            \icicle/uart/rx_clks_reg_n_0_[14] ,
            \icicle/uart/rx_clks_reg_n_0_[13] 
        }),
        .S({
            1'b0,
            rx_clks0_carry__2_i_1_n_0,
            rx_clks0_carry__2_i_2_n_0,
            rx_clks0_carry__2_i_3_n_0
        }),
        .CO({
            1'bz,
            1'bz,
            \icicle/uart/rx_clks0_carry__2_n_2 ,
            \icicle/uart/rx_clks0_carry__2_n_3 
        }),
        .O({
            1'bz,
            \icicle/uart/rx_clks0__28(15) ,
            \icicle/uart/rx_clks0__28(14) ,
            \icicle/uart/rx_clks0__28(13) 
        })
    );

    HAL_INV g0_1620_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[13] ),
        .O(rx_clks0_carry__2_i_3_n_0)
    );

    HAL_INV g0_1658_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[4] ),
        .O(rx_clks0_carry_i_1_n_0)
    );

    HAL_INV g0_1659_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[3] ),
        .O(rx_clks0_carry_i_2_n_0)
    );

    HAL_INV g0_1662_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[2] ),
        .O(rx_clks0_carry_i_3_n_0)
    );

    HAL_INV g0_1579_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[5] ),
        .O(rx_clks0_carry__0_i_4_n_0)
    );

    HAL_INV g0_1581_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[12] ),
        .O(rx_clks0_carry__1_i_1_n_0)
    );

    HAL_INV g0_1586_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[11] ),
        .O(rx_clks0_carry__1_i_2_n_0)
    );

    HAL_INV g0_1592_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[10] ),
        .O(rx_clks0_carry__1_i_3_n_0)
    );

    HAL_INV g0_1611_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[15] ),
        .O(rx_clks0_carry__2_i_1_n_0)
    );

    HAL_INV g0_1558_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[8] ),
        .O(rx_clks0_carry__0_i_1_n_0)
    );

    HAL_INV g0_1669_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[1] ),
        .O(rx_clks0_carry_i_4_n_0)
    );

    HAL_INV g0_1559_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[7] ),
        .O(rx_clks0_carry__0_i_2_n_0)
    );

    HAL_INV g0_1560_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[6] ),
        .O(rx_clks0_carry__0_i_3_n_0)
    );

    HAL_INV g0_1597_RESYNTH (
        .A(\icicle/uart/rx_clks_reg_n_0_[9] ),
        .O(rx_clks0_carry__1_i_4_n_0)
    );
endmodule

