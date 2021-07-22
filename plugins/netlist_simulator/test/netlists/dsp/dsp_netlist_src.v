module top (
        A,
		B,
		C,
		D,
		O,
		CLK,
		CE,
		IRSTTOP,
	    IRSTBOT,
		ORSTTOP,
		ORSTBOT,
		AHOLD,
		BHOLD,
		CHOLD,
		DHOLD,
		OHOLDTOP,
		OHOLDBOT,
		OLOADTOP,
		OLOADBOT,
		ADDSUBTOP,
		ADDSUBBOT,
		CO,
		CI,  		//from bottom tile
		ACCUMCI, 	// Carry input from MAC CO below
		ACCUMCO, 	// Carry output to above MAC block.    
		SIGNEXTIN,
		SIGNEXTOUT
); 

output 	[31:0] O;	     // Output [31:0]
input	[15:0] A;        // data  to upper mult block / upper accum block.
input	[15:0] B;        // data  to lower mult block / lower accum block.   
input	[15:0] C;        // direct data  to upper accum block. 
input	[15:0] D;        // direct data  to lower accum block.
input	CLK;	         // Clock for MAC16 elements 
input	CE;              // Clock enable . global control 
input	IRSTTOP;         // Active High  reset for  A,C registers,upper half multplier pipeline regs(16). 
input	IRSTBOT;         // Active High reset for  B,D registers, lower half multiplier pipeline regs(16), 32 bit result pipelines regs   
input	ORSTTOP;	     // Active High reset for top accum registers O[31:16]
input	ORSTBOT;         // Active High reset for bottom accum registers O[15:0]   
input   AHOLD;           // Active High hold data signal for A register
input   BHOLD;           // Active High hold data signal for B register   
input   CHOLD;           // Active High hold data signal for C register
input   DHOLD;           // Active High hold data signal for D register 
input   OHOLDTOP;        // Active High hold data signal for top accum registers O[31:16]
input   OHOLDBOT;        // Active High hold data signal for bottom  accum registers O[15:0]     
input 	OLOADTOP;        // Load top accum regiser with  direct input C or Registered data C.  
input 	OLOADBOT;        // Load bottom accum regisers with direct input D or Registered data D
input 	ADDSUBTOP;       // Control for Add/Sub operation for top accum . 0-addition , 1-subtraction.  
input 	ADDSUBBOT;       // Control for Add/Sub operation for bottom accum . 0-addition , 1-subtraction.
output  CO;              // top accumulator carry out to next LUT
input 	CI;              // bottom accumaltor carry in signal from lower LUT block. 
input   ACCUMCI;         // Carry in from  MAC16 below
output  ACCUMCO;         // Carry out to MAC16 above
input   SIGNEXTIN;	     // Single bit Sign extenstion from MAC16 below         
output  SIGNEXTOUT;      // Single bit Sign extenstion to MAC16 above

SB_MAC16 # (
    .X_COORDINATE(0),
    .Y_COORDINATE(40),
    .B_SIGNED(1'b1),
    .A_SIGNED(1'b1),
    .MODE_8x8(1'b0),
    .BOTADDSUB_CARRYSELECT(2'b00),
    .BOTADDSUB_UPPERINPUT(1'b0),
    .BOTADDSUB_LOWERINPUT(2'b01),
    .BOTOUTPUT_SELECT(2'b10),
    .TOPADDSUB_CARRYSELECT(2'b01),
    .TOPADDSUB_UPPERINPUT(1'b0),
    .TOPADDSUB_LOWERINPUT(2'b01),
    .TOPOUTPUT_SELECT(2'b10),
    .PIPELINE_16x16_MULT_REG2(1'b1),
    .PIPELINE_16x16_MULT_REG1(1'b0),
    .BOT_8x8_MULT_REG(1'b0),
    .TOP_8x8_MULT_REG(1'b0),
    .D_REG(1'b1),
    .B_REG(1'b1),
    .A_REG(1'b1),
    .C_REG(1'b1)
) i_sbmac16_0_40 (
    .A(A),
    .B(B),
    .C(C),
    .D(D),
    .O(O),
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
    //MAC cascading ports.  
    .ACCUMCI(),
    .ACCUMCO(),
    .SIGNEXTIN(),
    .SIGNEXTOUT()
);
endmodule