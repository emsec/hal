`timescale 1ps/1ps

module dsp_netlist(CI,ADDSUBBOT,OLOADBOT,OHOLDBOT,DHOLD,ADDSUBTOP,BHOLD,AHOLD,D_5 ,IRSTBOT,D_7 ,IRSTTOP,CE,ORSTTOP,CLK,OHOLDTOP,D_0 ,D_1 ,A_2 ,D_2 ,D_3 ,D_4 ,D_6 ,D_8 ,D_9 ,D_10 ,D_11 ,D_12 ,D_13 ,D_14 ,D_15 ,C_13 ,C_14 ,C_15 ,ORSTBOT,B_0 ,A_0 ,A_1 ,A_3 ,A_4 ,A_5 ,A_6 ,A_7 ,A_8 ,A_9 ,A_10 ,A_11 ,A_12 ,A_13 ,A_14 ,A_15 ,CHOLD,OLOADTOP,C_0 ,C_1 ,C_2 ,C_3 ,C_4 ,C_5 ,C_6 ,C_7 ,C_8 ,C_9 ,C_10 ,C_11 ,C_12 ,B_1 ,B_2 ,B_3 ,B_4 ,B_5 ,B_6 ,B_7 ,B_8 ,B_9 ,B_10 ,B_11 ,B_12 ,B_13 ,B_14 ,B_15 ,CO,O_13 ,O_14 ,O_31 ,O_30 ,O_0 ,O_1 ,O_2 ,O_3 ,O_4 ,O_5 ,O_6 ,O_7 ,O_8 ,O_9 ,O_10 ,O_11 ,O_12 ,O_15 ,O_16 ,O_17 ,O_18 ,O_19 ,O_20 ,O_21 ,O_22 ,O_23 ,O_24 ,O_25 ,O_26 ,O_27 ,O_28 ,O_29) ;
    input CI;
    input ADDSUBBOT;
    input OLOADBOT;
    input OHOLDBOT;
    input DHOLD;
    input ADDSUBTOP;
    input BHOLD;
    input AHOLD;
    input D_5 ;
    input IRSTBOT;
    input D_7 ;
    input IRSTTOP;
    input CE;
    input ORSTTOP;
    input CLK;
    input OHOLDTOP;
    input D_0 ;
    input D_1 ;
    input A_2 ;
    input D_2 ;
    input D_3 ;
    input D_4 ;
    input D_6 ;
    input D_8 ;
    input D_9 ;
    input D_10 ;
    input D_11 ;
    input D_12 ;
    input D_13 ;
    input D_14 ;
    input D_15 ;
    input C_13 ;
    input C_14 ;
    input C_15 ;
    input ORSTBOT;
    input B_0 ;
    input A_0 ;
    input A_1 ;
    input A_3 ;
    input A_4 ;
    input A_5 ;
    input A_6 ;
    input A_7 ;
    input A_8 ;
    input A_9 ;
    input A_10 ;
    input A_11 ;
    input A_12 ;
    input A_13 ;
    input A_14 ;
    input A_15 ;
    input CHOLD;
    input OLOADTOP;
    input C_0 ;
    input C_1 ;
    input C_2 ;
    input C_3 ;
    input C_4 ;
    input C_5 ;
    input C_6 ;
    input C_7 ;
    input C_8 ;
    input C_9 ;
    input C_10 ;
    input C_11 ;
    input C_12 ;
    input B_1 ;
    input B_2 ;
    input B_3 ;
    input B_4 ;
    input B_5 ;
    input B_6 ;
    input B_7 ;
    input B_8 ;
    input B_9 ;
    input B_10 ;
    input B_11 ;
    input B_12 ;
    input B_13 ;
    input B_14 ;
    input B_15 ;
    output CO;
    output O_13 ;
    output O_14 ;
    output O_31 ;
    output O_30 ;
    output O_0 ;
    output O_1 ;
    output O_2 ;
    output O_3 ;
    output O_4 ;
    output O_5 ;
    output O_6 ;
    output O_7 ;
    output O_8 ;
    output O_9 ;
    output O_10 ;
    output O_11 ;
    output O_12 ;
    output O_15 ;
    output O_16 ;
    output O_17 ;
    output O_18 ;
    output O_19 ;
    output O_20 ;
    output O_21 ;
    output O_22 ;
    output O_23 ;
    output O_24 ;
    output O_25 ;
    output O_26 ;
    output O_27 ;
    output O_28 ;
    output O_29 ;

    SB_MAC16 #(
        .A_REG(1'b1),
        .A_SIGNED(1'b1),
        .BOTADDSUB_CARRYSELECT(1'b0),
        .BOTADDSUB_LOWERINPUT(1'b1),
        .BOTADDSUB_UPPERINPUT(1'b0),
        .BOTOUTPUT_SELECT(2'h2),
        .BOT_8x8_MULT_REG(1'b0),
        .B_REG(1'b1),
        .B_SIGNED(1'b1),
        .C_REG(1'b1),
        .D_REG(1'b1),
        .MODE_8x8(1'b0),
        .PIPELINE_16x16_MULT_REG1(1'b0),
        .PIPELINE_16x16_MULT_REG2(1'b1),
        .TOPADDSUB_CARRYSELECT(1'b1),
        .TOPADDSUB_LOWERINPUT(1'b1),
        .TOPADDSUB_UPPERINPUT(1'b0),
        .TOPOUTPUT_SELECT(2'h2),
        .TOP_8x8_MULT_REG(1'b0)
    ) i_sbmac16_0_40 (
        .CLK(CLK),
        .CE(CE),
        .IRSTTOP(IRSTTOP),
        .IRSTBOT(IRSTBOT),
        .ORSTTOP(ORSTTOP),
        .ORSTBOT(ORSTBOT),
        .AHOLD(AHOLD),
        .BHOLD(BHOLD),
        .CHOLD(CHOLD),
        .DHOLD(DHOLD),
        .OHOLDTOP(OHOLDTOP),
        .OHOLDBOT(OHOLDBOT),
        .OLOADTOP(OLOADTOP),
        .OLOADBOT(OLOADBOT),
        .ADDSUBTOP(ADDSUBTOP),
        .ADDSUBBOT(ADDSUBBOT),
        .CO(CO),
        .CI(CI),
        .A({A_15 ,A_14 ,A_13 ,A_12 ,A_11 ,A_10 ,A_9 ,A_8 ,A_7 ,A_6 ,A_5 ,A_4 ,A_3 ,A_2 ,A_1 ,A_0 }),
        .B({B_15 ,B_14 ,B_13 ,B_12 ,B_11 ,B_10 ,B_9 ,B_8 ,B_7 ,B_6 ,B_5 ,B_4 ,B_3 ,B_2 ,B_1 ,B_0 }),
        .C({C_15 ,C_14 ,C_13 ,C_12 ,C_11 ,C_10 ,C_9 ,C_8 ,C_7 ,C_6 ,C_5 ,C_4 ,C_3 ,C_2 ,C_1 ,C_0 }),
        .D({D_15 ,D_14 ,D_13 ,D_12 ,D_11 ,D_10 ,D_9 ,D_8 ,D_7 ,D_6 ,D_5 ,D_4 ,D_3 ,D_2 ,D_1 ,D_0 }),
        .O({O_31 ,O_30 ,O_29 ,O_28 ,O_27 ,O_26 ,O_25 ,O_24 ,O_23 ,O_22 ,O_21 ,O_20 ,O_19 ,O_18 ,O_17 ,O_16 ,O_15 ,O_14 ,O_13 ,O_12 ,O_11 ,O_10 ,O_9 ,O_8 ,O_7 ,O_6 ,O_5 ,O_4 ,O_3 ,O_2 ,O_1 ,O_0 }),
        .ACCUMCI(), 	// Carry input from MAC CO below
		.ACCUMCO(), 	// Carry output to above MAC block.    
		.SIGNEXTIN(),
		.SIGNEXTOUT()
    );
endmodule

