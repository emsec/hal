module FSM (
  CLK,
  INPUT_0,
  INPUT_1,
  INPUT_2,
  INPUT_3,
  INPUT_4,
  OUTPUT_0,
  OUTPUT_1
 ) ;
  input CLK ;
  input INPUT_0 ;
  input INPUT_1 ;
  input INPUT_2 ;
  input INPUT_3 ;
  input INPUT_4 ;
  output OUTPUT_0 ;
  output OUTPUT_1 ;
  wire CLK_BUF ;
  wire CLK_BUF_BUF ;
  wire FSM_sequential_STATE_REG_0_i_2_n_0 ;
  wire FSM_sequential_STATE_REG_0_i_3_n_0 ;
  wire FSM_sequential_STATE_REG_1_i_2_n_0 ;
  wire FSM_sequential_STATE_REG_1_i_3_n_0 ;
  wire INPUT_BUF_0 ;
  wire INPUT_BUF_1 ;
  wire INPUT_BUF_2 ;
  wire INPUT_BUF_3 ;
  wire INPUT_BUF_4 ;
  wire NEXT_STATE_S_0 ;
  wire NEXT_STATE_S_1 ;
  wire OUTPUT_BUF_0 ;
  wire OUTPUT_BUF_1 ;
  wire STATE_REG_0 ;
  wire STATE_REG_1 ;
  wire const0 ;
  wire const1 ;
BUF CLK_BUF_BUF_inst_inst (
  .\I (CLK_BUF ),
  .\O (CLK_BUF_BUF )
 ) ;
BUF CLK_BUF_inst_inst (
  .\I (CLK ),
  .\O (CLK_BUF )
 ) ;
LUT6 #(.INIT(64'hc8ccccccfccccccc))
FSM_sequential_STATE_REG_0_i_2_inst (
  .\I0 (INPUT_BUF_3 ),
  .\I1 (STATE_REG_0 ),
  .\I2 (INPUT_BUF_1 ),
  .\I3 (INPUT_BUF_4 ),
  .\I4 (INPUT_BUF_0 ),
  .\I5 (INPUT_BUF_2 ),
  .\O (FSM_sequential_STATE_REG_0_i_2_n_0 )
 ) ;
LUT6 #(.INIT(64'haaaaabaaaaaaaa8a))
FSM_sequential_STATE_REG_0_i_3_inst (
  .\I0 (STATE_REG_0 ),
  .\I1 (INPUT_BUF_1 ),
  .\I2 (INPUT_BUF_4 ),
  .\I3 (INPUT_BUF_3 ),
  .\I4 (INPUT_BUF_0 ),
  .\I5 (INPUT_BUF_2 ),
  .\O (FSM_sequential_STATE_REG_0_i_3_n_0 )
 ) ;
LUT6 #(.INIT(61'h0a00000004000000))
FSM_sequential_STATE_REG_1_i_2_inst (
  .\I0 (STATE_REG_0 ),
  .\I1 (INPUT_BUF_1 ),
  .\I2 (INPUT_BUF_3 ),
  .\I3 (INPUT_BUF_4 ),
  .\I4 (INPUT_BUF_0 ),
  .\I5 (INPUT_BUF_2 ),
  .\O (FSM_sequential_STATE_REG_1_i_2_n_0 )
 ) ;
LUT6 #(.INIT(64'hfdffffffffbfffdf))
FSM_sequential_STATE_REG_1_i_3_inst (
  .\I0 (STATE_REG_0 ),
  .\I1 (INPUT_BUF_1 ),
  .\I2 (INPUT_BUF_4 ),
  .\I3 (INPUT_BUF_3 ),
  .\I4 (INPUT_BUF_0 ),
  .\I5 (INPUT_BUF_2 ),
  .\O (FSM_sequential_STATE_REG_1_i_3_n_0 )
 ) ;
FFR #(.INIT(1'h0))
FSM_sequential_STATE_REG_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (NEXT_STATE_S_0 ),
  .\R (const0 ),
  .\Q (STATE_REG_0 )
 ) ;
MUX FSM_sequential_STATE_REG_reg_0_i_1_inst (
  .\I0 (FSM_sequential_STATE_REG_0_i_2_n_0 ),
  .\I1 (FSM_sequential_STATE_REG_0_i_3_n_0 ),
  .\S (STATE_REG_1 ),
  .\O (NEXT_STATE_S_0 )
 ) ;
FFR #(.INIT(1'h0))
FSM_sequential_STATE_REG_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (NEXT_STATE_S_1 ),
  .\R (const0 ),
  .\Q (STATE_REG_1 )
 ) ;
GND GND_inst (
  .\O (const0 )
 ) ;
MUX FSM_sequential_STATE_REG_reg_1_i_1_inst (
  .\I0 (FSM_sequential_STATE_REG_1_i_2_n_0 ),
  .\I1 (FSM_sequential_STATE_REG_1_i_3_n_0 ),
  .\S (STATE_REG_1 ),
  .\O (NEXT_STATE_S_1 )
 ) ;
BUF INPUT_BUF_0_inst_inst (
  .\I (INPUT_0 ),
  .\O (INPUT_BUF_0 )
 ) ;
BUF INPUT_BUF_1_inst_inst (
  .\I (INPUT_1 ),
  .\O (INPUT_BUF_1 )
 ) ;
BUF INPUT_BUF_2_inst_inst (
  .\I (INPUT_2 ),
  .\O (INPUT_BUF_2 )
 ) ;
BUF INPUT_BUF_3_inst_inst (
  .\I (INPUT_3 ),
  .\O (INPUT_BUF_3 )
 ) ;
BUF INPUT_BUF_4_inst_inst (
  .\I (INPUT_4 ),
  .\O (INPUT_BUF_4 )
 ) ;
BUF OUTPUT_BUF_0_inst_inst (
  .\I (OUTPUT_BUF_0 ),
  .\O (OUTPUT_0 )
 ) ;
LUT1 #(.INIT(1'h1))
OUTPUT_BUF_0_inst_i_1_inst (
  .\I0 (STATE_REG_0 ),
  .\O (OUTPUT_BUF_0 )
 ) ;
BUF OUTPUT_BUF_1_inst_inst (
  .\I (OUTPUT_BUF_1 ),
  .\O (OUTPUT_1 )
 ) ;
LUT2 #(.INIT(3'h6))
OUTPUT_BUF_1_inst_i_1_inst (
  .\I0 (STATE_REG_1 ),
  .\I1 (STATE_REG_0 ),
  .\O (OUTPUT_BUF_1 )
 ) ;
VCC VCC_inst (
  .\O (const1 )
 ) ;
endmodule
