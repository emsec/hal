module test_ext_uart (
  CLK,
  UART_RX,
  UART_TX
 ) ;
  input CLK ;
  input UART_RX ;
  output UART_TX ;
  wire BIT_CNT_0_i_1_0_n_0 ;
  wire BIT_CNT_3_i_3_n_0 ;
  wire BIT_CNT_3_i_4_n_0 ;
  wire CLK_CNT_10_i_2_n_0 ;
  wire CLK_CNT_13_i_1_n_0 ;
  wire CLK_CNT_13_i_4_n_0 ;
  wire CLK_BUF ;
  wire CLK_BUF_BUF ;
  wire FSM_onehot_STATE_0_i_1_n_0 ;
  wire FSM_onehot_STATE_0_i_2_n_0 ;
  wire FSM_onehot_STATE_1_i_1_0_n_0 ;
  wire FSM_onehot_STATE_1_i_1_n_0 ;
  wire FSM_onehot_STATE_1_i_2_n_0 ;
  wire FSM_onehot_STATE_2_i_1_n_0 ;
  wire FSM_onehot_STATE_3_i_1_n_0 ;
  wire FSM_onehot_STATE_3_i_2_n_0 ;
  wire FSM_sequential_STATE_0_i_1_0_n_0 ;
  wire FSM_sequential_STATE_0_i_1_n_0 ;
  wire FSM_sequential_STATE_1_i_1_0_n_0 ;
  wire FSM_sequential_STATE_1_i_1_n_0 ;
  wire FSM_sequential_STATE_1_i_2_0_n_0 ;
  wire NEXT_STATE ;
  wire REG_0 ;
  wire REG_1 ;
  wire REG_2 ;
  wire REG_3 ;
  wire REG_4 ;
  wire REG_5 ;
  wire REG_6 ;
  wire REG_63_i_1_n_0 ;
  wire REG_7 ;
  wire REG_7_i_1_0_n_0 ;
  wire REG_7_i_2_n_0 ;
  wire REG_7_i_3_n_0 ;
  wire REG_7_i_4_n_0 ;
  wire REG_reg_n_0_0 ;
  wire REG_reg_n_0_1 ;
  wire REG_reg_n_0_10 ;
  wire REG_reg_n_0_11 ;
  wire REG_reg_n_0_12 ;
  wire REG_reg_n_0_13 ;
  wire REG_reg_n_0_14 ;
  wire REG_reg_n_0_15 ;
  wire REG_reg_n_0_16 ;
  wire REG_reg_n_0_17 ;
  wire REG_reg_n_0_18 ;
  wire REG_reg_n_0_19 ;
  wire REG_reg_n_0_2 ;
  wire REG_reg_n_0_20 ;
  wire REG_reg_n_0_21 ;
  wire REG_reg_n_0_22 ;
  wire REG_reg_n_0_23 ;
  wire REG_reg_n_0_24 ;
  wire REG_reg_n_0_25 ;
  wire REG_reg_n_0_26 ;
  wire REG_reg_n_0_27 ;
  wire REG_reg_n_0_28 ;
  wire REG_reg_n_0_29 ;
  wire REG_reg_n_0_3 ;
  wire REG_reg_n_0_30 ;
  wire REG_reg_n_0_31 ;
  wire REG_reg_n_0_32 ;
  wire REG_reg_n_0_33 ;
  wire REG_reg_n_0_34 ;
  wire REG_reg_n_0_35 ;
  wire REG_reg_n_0_36 ;
  wire REG_reg_n_0_37 ;
  wire REG_reg_n_0_38 ;
  wire REG_reg_n_0_39 ;
  wire REG_reg_n_0_4 ;
  wire REG_reg_n_0_40 ;
  wire REG_reg_n_0_41 ;
  wire REG_reg_n_0_42 ;
  wire REG_reg_n_0_43 ;
  wire REG_reg_n_0_44 ;
  wire REG_reg_n_0_45 ;
  wire REG_reg_n_0_46 ;
  wire REG_reg_n_0_47 ;
  wire REG_reg_n_0_48 ;
  wire REG_reg_n_0_49 ;
  wire REG_reg_n_0_5 ;
  wire REG_reg_n_0_50 ;
  wire REG_reg_n_0_51 ;
  wire REG_reg_n_0_52 ;
  wire REG_reg_n_0_53 ;
  wire REG_reg_n_0_54 ;
  wire REG_reg_n_0_55 ;
  wire REG_reg_n_0_56 ;
  wire REG_reg_n_0_57 ;
  wire REG_reg_n_0_58 ;
  wire REG_reg_n_0_59 ;
  wire REG_reg_n_0_6 ;
  wire REG_reg_n_0_60 ;
  wire REG_reg_n_0_61 ;
  wire REG_reg_n_0_62 ;
  wire REG_reg_n_0_63 ;
  wire REG_reg_n_0_7 ;
  wire REG_reg_n_0_8 ;
  wire REG_reg_n_0_9 ;
  wire SEND_CNT_10_i_2_n_0 ;
  wire SEND_CNT_13_i_4_n_0 ;
  wire SEND_CNT_13_i_6_n_0 ;
  wire SEND_CNT_13_i_7_n_0 ;
  wire STATE ;
  wire TX_IDLE_O ;
  wire UART_RX_BUF ;
  wire UART_TX_BUF ;
  wire UART_TX_BUF_inst_i_2_n_0 ;
  wire UART_TX_BUF_inst_i_3_n_0 ;
  wire UART_TX_BUF_inst_i_4_n_0 ;
  wire UART_TX_BUF_inst_i_5_n_0 ;
  wire const0 ;
  wire const1 ;
  wire ext_uartinst_receiverCNT_EN ;
  wire ext_uartinst_receiverCNT_RST ;
  wire ext_uartinst_receiverCOUNT_reg_0_0 ;
  wire ext_uartinst_receiverCOUNT_reg_0_1 ;
  wire ext_uartinst_receiverCOUNT_reg_0_2 ;
  wire ext_uartinst_receiverCOUNT_reg_0_3 ;
  wire ext_uartinst_receiverNEXT_STATE_1 ;
  wire ext_uartinst_receiverREG_EN ;
  wire ext_uartinst_receiverREG_reg_n_0_0 ;
  wire ext_uartinst_receiverREG_reg_n_0_1 ;
  wire ext_uartinst_receiverREG_reg_n_0_10 ;
  wire ext_uartinst_receiverREG_reg_n_0_11 ;
  wire ext_uartinst_receiverREG_reg_n_0_12 ;
  wire ext_uartinst_receiverREG_reg_n_0_13 ;
  wire ext_uartinst_receiverREG_reg_n_0_14 ;
  wire ext_uartinst_receiverREG_reg_n_0_15 ;
  wire ext_uartinst_receiverREG_reg_n_0_16 ;
  wire ext_uartinst_receiverREG_reg_n_0_17 ;
  wire ext_uartinst_receiverREG_reg_n_0_18 ;
  wire ext_uartinst_receiverREG_reg_n_0_19 ;
  wire ext_uartinst_receiverREG_reg_n_0_2 ;
  wire ext_uartinst_receiverREG_reg_n_0_20 ;
  wire ext_uartinst_receiverREG_reg_n_0_21 ;
  wire ext_uartinst_receiverREG_reg_n_0_22 ;
  wire ext_uartinst_receiverREG_reg_n_0_23 ;
  wire ext_uartinst_receiverREG_reg_n_0_24 ;
  wire ext_uartinst_receiverREG_reg_n_0_25 ;
  wire ext_uartinst_receiverREG_reg_n_0_26 ;
  wire ext_uartinst_receiverREG_reg_n_0_27 ;
  wire ext_uartinst_receiverREG_reg_n_0_28 ;
  wire ext_uartinst_receiverREG_reg_n_0_29 ;
  wire ext_uartinst_receiverREG_reg_n_0_3 ;
  wire ext_uartinst_receiverREG_reg_n_0_30 ;
  wire ext_uartinst_receiverREG_reg_n_0_31 ;
  wire ext_uartinst_receiverREG_reg_n_0_32 ;
  wire ext_uartinst_receiverREG_reg_n_0_33 ;
  wire ext_uartinst_receiverREG_reg_n_0_34 ;
  wire ext_uartinst_receiverREG_reg_n_0_35 ;
  wire ext_uartinst_receiverREG_reg_n_0_36 ;
  wire ext_uartinst_receiverREG_reg_n_0_37 ;
  wire ext_uartinst_receiverREG_reg_n_0_38 ;
  wire ext_uartinst_receiverREG_reg_n_0_39 ;
  wire ext_uartinst_receiverREG_reg_n_0_4 ;
  wire ext_uartinst_receiverREG_reg_n_0_40 ;
  wire ext_uartinst_receiverREG_reg_n_0_41 ;
  wire ext_uartinst_receiverREG_reg_n_0_42 ;
  wire ext_uartinst_receiverREG_reg_n_0_43 ;
  wire ext_uartinst_receiverREG_reg_n_0_44 ;
  wire ext_uartinst_receiverREG_reg_n_0_45 ;
  wire ext_uartinst_receiverREG_reg_n_0_46 ;
  wire ext_uartinst_receiverREG_reg_n_0_47 ;
  wire ext_uartinst_receiverREG_reg_n_0_48 ;
  wire ext_uartinst_receiverREG_reg_n_0_49 ;
  wire ext_uartinst_receiverREG_reg_n_0_5 ;
  wire ext_uartinst_receiverREG_reg_n_0_50 ;
  wire ext_uartinst_receiverREG_reg_n_0_51 ;
  wire ext_uartinst_receiverREG_reg_n_0_52 ;
  wire ext_uartinst_receiverREG_reg_n_0_53 ;
  wire ext_uartinst_receiverREG_reg_n_0_54 ;
  wire ext_uartinst_receiverREG_reg_n_0_55 ;
  wire ext_uartinst_receiverREG_reg_n_0_56 ;
  wire ext_uartinst_receiverREG_reg_n_0_57 ;
  wire ext_uartinst_receiverREG_reg_n_0_58 ;
  wire ext_uartinst_receiverREG_reg_n_0_59 ;
  wire ext_uartinst_receiverREG_reg_n_0_6 ;
  wire ext_uartinst_receiverREG_reg_n_0_60 ;
  wire ext_uartinst_receiverREG_reg_n_0_61 ;
  wire ext_uartinst_receiverREG_reg_n_0_62 ;
  wire ext_uartinst_receiverREG_reg_n_0_63 ;
  wire ext_uartinst_receiverREG_reg_n_0_7 ;
  wire ext_uartinst_receiverREG_reg_n_0_8 ;
  wire ext_uartinst_receiverREG_reg_n_0_9 ;
  wire ext_uartinst_receiverSTATE_0 ;
  wire ext_uartinst_receiverSTATE_1 ;
  wire ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 ;
  wire ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_1 ;
  wire ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_2 ;
  wire ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_3 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_10 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_12 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_13 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_3 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_4 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_5 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 ;
  wire ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_9 ;
  wire ext_uartinst_receiverinst_receiverCNT_EN ;
  wire ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_0 ;
  wire ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_1 ;
  wire ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_2 ;
  wire ext_uartinst_receiverinst_receiverREG_EN ;
  wire ext_uartinst_receiverinst_receiverRS232_BIT_LAST ;
  wire ext_uartinst_receiverinst_receiverRS232_BIT_SYN ;
  wire ext_uartinst_receiverinst_receiver_DATA_VALID ;
  wire ext_uartinst_transmitterCOUNT_reg_0_0 ;
  wire ext_uartinst_transmitterCOUNT_reg_0_1 ;
  wire ext_uartinst_transmitterCOUNT_reg_0_2 ;
  wire ext_uartinst_transmitterCOUNT_reg_0_3 ;
  wire ext_uartinst_transmitterFSM_onehot_STATE_reg_n_0_2 ;
  wire ext_uartinst_transmitterREG_reg_n_0_0 ;
  wire ext_uartinst_transmitterREG_reg_n_0_1 ;
  wire ext_uartinst_transmitterREG_reg_n_0_10 ;
  wire ext_uartinst_transmitterREG_reg_n_0_11 ;
  wire ext_uartinst_transmitterREG_reg_n_0_12 ;
  wire ext_uartinst_transmitterREG_reg_n_0_13 ;
  wire ext_uartinst_transmitterREG_reg_n_0_14 ;
  wire ext_uartinst_transmitterREG_reg_n_0_15 ;
  wire ext_uartinst_transmitterREG_reg_n_0_16 ;
  wire ext_uartinst_transmitterREG_reg_n_0_17 ;
  wire ext_uartinst_transmitterREG_reg_n_0_18 ;
  wire ext_uartinst_transmitterREG_reg_n_0_19 ;
  wire ext_uartinst_transmitterREG_reg_n_0_2 ;
  wire ext_uartinst_transmitterREG_reg_n_0_20 ;
  wire ext_uartinst_transmitterREG_reg_n_0_21 ;
  wire ext_uartinst_transmitterREG_reg_n_0_22 ;
  wire ext_uartinst_transmitterREG_reg_n_0_23 ;
  wire ext_uartinst_transmitterREG_reg_n_0_24 ;
  wire ext_uartinst_transmitterREG_reg_n_0_25 ;
  wire ext_uartinst_transmitterREG_reg_n_0_26 ;
  wire ext_uartinst_transmitterREG_reg_n_0_27 ;
  wire ext_uartinst_transmitterREG_reg_n_0_28 ;
  wire ext_uartinst_transmitterREG_reg_n_0_29 ;
  wire ext_uartinst_transmitterREG_reg_n_0_3 ;
  wire ext_uartinst_transmitterREG_reg_n_0_30 ;
  wire ext_uartinst_transmitterREG_reg_n_0_31 ;
  wire ext_uartinst_transmitterREG_reg_n_0_32 ;
  wire ext_uartinst_transmitterREG_reg_n_0_33 ;
  wire ext_uartinst_transmitterREG_reg_n_0_34 ;
  wire ext_uartinst_transmitterREG_reg_n_0_35 ;
  wire ext_uartinst_transmitterREG_reg_n_0_36 ;
  wire ext_uartinst_transmitterREG_reg_n_0_37 ;
  wire ext_uartinst_transmitterREG_reg_n_0_38 ;
  wire ext_uartinst_transmitterREG_reg_n_0_39 ;
  wire ext_uartinst_transmitterREG_reg_n_0_4 ;
  wire ext_uartinst_transmitterREG_reg_n_0_40 ;
  wire ext_uartinst_transmitterREG_reg_n_0_41 ;
  wire ext_uartinst_transmitterREG_reg_n_0_42 ;
  wire ext_uartinst_transmitterREG_reg_n_0_43 ;
  wire ext_uartinst_transmitterREG_reg_n_0_44 ;
  wire ext_uartinst_transmitterREG_reg_n_0_45 ;
  wire ext_uartinst_transmitterREG_reg_n_0_46 ;
  wire ext_uartinst_transmitterREG_reg_n_0_47 ;
  wire ext_uartinst_transmitterREG_reg_n_0_48 ;
  wire ext_uartinst_transmitterREG_reg_n_0_49 ;
  wire ext_uartinst_transmitterREG_reg_n_0_5 ;
  wire ext_uartinst_transmitterREG_reg_n_0_50 ;
  wire ext_uartinst_transmitterREG_reg_n_0_51 ;
  wire ext_uartinst_transmitterREG_reg_n_0_52 ;
  wire ext_uartinst_transmitterREG_reg_n_0_53 ;
  wire ext_uartinst_transmitterREG_reg_n_0_54 ;
  wire ext_uartinst_transmitterREG_reg_n_0_55 ;
  wire ext_uartinst_transmitterREG_reg_n_0_6 ;
  wire ext_uartinst_transmitterREG_reg_n_0_7 ;
  wire ext_uartinst_transmitterREG_reg_n_0_8 ;
  wire ext_uartinst_transmitterREG_reg_n_0_9 ;
  wire ext_uartinst_transmitterTX_DATA_0 ;
  wire ext_uartinst_transmitterTX_DATA_1 ;
  wire ext_uartinst_transmitterTX_DATA_2 ;
  wire ext_uartinst_transmitterTX_DATA_3 ;
  wire ext_uartinst_transmitterTX_DATA_4 ;
  wire ext_uartinst_transmitterTX_DATA_5 ;
  wire ext_uartinst_transmitterTX_DATA_6 ;
  wire ext_uartinst_transmitterTX_DATA_7 ;
  wire ext_uartinst_transmitterinst_transmitterBIT_CNT ;
  wire ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ;
  wire ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ;
  wire ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_2 ;
  wire ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_3 ;
  wire ext_uartinst_transmitterinst_transmitterCNT_EN ;
  wire ext_uartinst_transmitterinst_transmitterCNT_RST ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_10 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_11 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_12 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_13 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_4 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_5 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_8 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_9 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_2 ;
  wire ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_3 ;
  wire ext_uartinst_transmitterinst_transmitterSTATE_0 ;
  wire ext_uartinst_transmitterinst_transmitterSTATE_1 ;
  wire ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ;
  wire p_0_in_0 ;
  wire p_0_in_0_1 ;
  wire p_0_in_0_2 ;
  wire p_0_in_0_3 ;
  wire p_0_in_1 ;
  wire p_0_in_10 ;
  wire p_0_in_11 ;
  wire p_0_in_12 ;
  wire p_0_in_13 ;
  wire p_0_in_1_0 ;
  wire p_0_in_1_1 ;
  wire p_0_in_1_2 ;
  wire p_0_in_1_3 ;
  wire p_0_in_2 ;
  wire p_0_in_2_0 ;
  wire p_0_in_2_1 ;
  wire p_0_in_2_10 ;
  wire p_0_in_2_11 ;
  wire p_0_in_2_12 ;
  wire p_0_in_2_13 ;
  wire p_0_in_2_2 ;
  wire p_0_in_2_3 ;
  wire p_0_in_2_4 ;
  wire p_0_in_2_5 ;
  wire p_0_in_2_6 ;
  wire p_0_in_2_7 ;
  wire p_0_in_2_8 ;
  wire p_0_in_2_9 ;
  wire p_0_in_3 ;
  wire p_0_in_3_0 ;
  wire p_0_in_3_1 ;
  wire p_0_in_3_2 ;
  wire p_0_in_3_3 ;
  wire p_0_in_4 ;
  wire p_0_in_4_0 ;
  wire p_0_in_4_1 ;
  wire p_0_in_4_2 ;
  wire p_0_in_4_3 ;
  wire p_0_in_5 ;
  wire p_0_in_6 ;
  wire p_0_in_7 ;
  wire p_0_in_8 ;
  wire p_0_in_9 ;
  wire p_1_in_10 ;
  wire p_1_in_11 ;
  wire p_1_in_12 ;
  wire p_1_in_13 ;
  wire p_1_in_14 ;
  wire p_1_in_15 ;
  wire p_1_in_16 ;
  wire p_1_in_17 ;
  wire p_1_in_18 ;
  wire p_1_in_19 ;
  wire p_1_in_20 ;
  wire p_1_in_21 ;
  wire p_1_in_22 ;
  wire p_1_in_23 ;
  wire p_1_in_24 ;
  wire p_1_in_25 ;
  wire p_1_in_26 ;
  wire p_1_in_27 ;
  wire p_1_in_28 ;
  wire p_1_in_29 ;
  wire p_1_in_30 ;
  wire p_1_in_31 ;
  wire p_1_in_32 ;
  wire p_1_in_33 ;
  wire p_1_in_34 ;
  wire p_1_in_35 ;
  wire p_1_in_36 ;
  wire p_1_in_37 ;
  wire p_1_in_38 ;
  wire p_1_in_39 ;
  wire p_1_in_40 ;
  wire p_1_in_41 ;
  wire p_1_in_42 ;
  wire p_1_in_43 ;
  wire p_1_in_44 ;
  wire p_1_in_45 ;
  wire p_1_in_46 ;
  wire p_1_in_47 ;
  wire p_1_in_48 ;
  wire p_1_in_49 ;
  wire p_1_in_50 ;
  wire p_1_in_51 ;
  wire p_1_in_52 ;
  wire p_1_in_53 ;
  wire p_1_in_54 ;
  wire p_1_in_55 ;
  wire p_1_in_56 ;
  wire p_1_in_57 ;
  wire p_1_in_58 ;
  wire p_1_in_59 ;
  wire p_1_in_60 ;
  wire p_1_in_61 ;
  wire p_1_in_62 ;
  wire p_1_in_63 ;
  wire p_1_in_8 ;
  wire p_1_in_9 ;
LUT1 #(.INIT(1'h1))
BIT_CNT_0_i_1_0_inst (
  .\I0 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 ),
  .\O (BIT_CNT_0_i_1_0_n_0 )
 ) ;
LUT2 #(.INIT(3'h6))
BIT_CNT_1_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_1 ),
  .\O (p_0_in_0_1 )
 ) ;
LUT3 #(.INIT(5'h06))
BIT_CNT_1_i_1_0_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterCNT_RST ),
  .\O (p_0_in_3_1 )
 ) ;
LUT4 #(.INIT(13'h1540))
BIT_CNT_2_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterCNT_RST ),
  .\I1 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_2 ),
  .\O (p_0_in_3_2 )
 ) ;
LUT3 #(.INIT(7'h78))
BIT_CNT_2_i_1_0_inst (
  .\I0 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_1 ),
  .\I2 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_2 ),
  .\O (p_0_in_0_2 )
 ) ;
LUT5 #(.INIT(32'hff00d500))
BIT_CNT_3_i_1_inst (
  .\I0 (BIT_CNT_3_i_3_n_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_13 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_12 ),
  .\I3 (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\I4 (BIT_CNT_3_i_4_n_0 ),
  .\O (ext_uartinst_receiverinst_receiverCLK_CNT )
 ) ;
LUT5 #(.INIT(29'h15554000))
BIT_CNT_3_i_1_0_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterCNT_RST ),
  .\I1 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_2 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_3 ),
  .\O (p_0_in_3_3 )
 ) ;
LUT4 #(.INIT(15'h7f80))
BIT_CNT_3_i_2_inst (
  .\I0 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_1 ),
  .\I1 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 ),
  .\I2 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_2 ),
  .\I3 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_3 ),
  .\O (p_0_in_0_3 )
 ) ;
LUT5 #(.INIT(31'h7777777f))
BIT_CNT_3_i_3_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_13 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_9 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_10 ),
  .\O (BIT_CNT_3_i_3_n_0 )
 ) ;
LUT6 #(.INIT(63'h7fffffff80000000))
CLK_CNT_10_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\I2 (CLK_CNT_10_i_2_n_0 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_9 ),
  .\I5 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_10 ),
  .\O (p_0_in_10 )
 ) ;
LUT2 #(.INIT(3'h6))
CLK_CNT_11_i_1_inst (
  .\I0 (CLK_CNT_13_i_4_n_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 ),
  .\O (p_0_in_11 )
 ) ;
LUT6 #(.INIT(64'h8000000000000000))
CLK_CNT_10_i_2_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_5 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_3 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 ),
  .\I5 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_4 ),
  .\O (CLK_CNT_10_i_2_n_0 )
 ) ;
LUT6 #(.INIT(64'hea00000000000000))
BIT_CNT_3_i_4_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_5 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_4 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_13 ),
  .\I5 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ),
  .\O (BIT_CNT_3_i_4_n_0 )
 ) ;
LUT1 #(.INIT(1'h1))
CLK_CNT_0_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\O (p_0_in_0 )
 ) ;
LUT3 #(.INIT(7'h78))
CLK_CNT_12_i_1_inst (
  .\I0 (CLK_CNT_13_i_4_n_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_12 ),
  .\O (p_0_in_12 )
 ) ;
LUT3 #(.INIT(8'hfe))
CLK_CNT_13_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiver_DATA_VALID ),
  .\I1 (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_0 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT ),
  .\O (CLK_CNT_13_i_1_n_0 )
 ) ;
LUT5 #(.INIT(32'haa8aaaaa))
CLK_CNT_13_i_2_inst (
  .\I0 (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_2 ),
  .\I1 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_2 ),
  .\I2 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 ),
  .\I3 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_1 ),
  .\I4 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_3 ),
  .\O (ext_uartinst_receiverinst_receiverCNT_EN )
 ) ;
LUT4 #(.INIT(15'h7f80))
CLK_CNT_13_i_3_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 ),
  .\I1 (CLK_CNT_13_i_4_n_0 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_12 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_13 ),
  .\O (p_0_in_13 )
 ) ;
LUT6 #(.INIT(64'h8000000000000000))
CLK_CNT_13_i_4_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_10 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\I3 (CLK_CNT_10_i_2_n_0 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ),
  .\I5 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_9 ),
  .\O (CLK_CNT_13_i_4_n_0 )
 ) ;
LUT2 #(.INIT(3'h6))
CLK_CNT_1_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ),
  .\O (p_0_in_1 )
 ) ;
LUT3 #(.INIT(7'h78))
CLK_CNT_2_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 ),
  .\O (p_0_in_2 )
 ) ;
LUT4 #(.INIT(15'h7f80))
CLK_CNT_3_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_3 ),
  .\O (p_0_in_3 )
 ) ;
LUT5 #(.INIT(31'h7fff8000))
CLK_CNT_4_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_3 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_4 ),
  .\O (p_0_in_4 )
 ) ;
LUT2 #(.INIT(3'h6))
CLK_CNT_6_i_1_inst (
  .\I0 (CLK_CNT_10_i_2_n_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\O (p_0_in_6 )
 ) ;
LUT6 #(.INIT(63'h7fffffff80000000))
CLK_CNT_5_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_3 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_4 ),
  .\I5 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_5 ),
  .\O (p_0_in_5 )
 ) ;
LUT3 #(.INIT(7'h78))
CLK_CNT_7_i_1_inst (
  .\I0 (CLK_CNT_10_i_2_n_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ),
  .\O (p_0_in_7 )
 ) ;
LUT4 #(.INIT(15'h7f80))
CLK_CNT_8_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\I1 (CLK_CNT_10_i_2_n_0 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 ),
  .\O (p_0_in_8 )
 ) ;
BUF CLK_BUF_BUF_inst_inst (
  .\I (CLK_BUF ),
  .\O (CLK_BUF_BUF )
 ) ;
LUT5 #(.INIT(31'h7fff8000))
CLK_CNT_9_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ),
  .\I1 (CLK_CNT_10_i_2_n_0 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 ),
  .\I4 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_9 ),
  .\O (p_0_in_9 )
 ) ;
BUF CLK_BUF_inst_inst (
  .\I (CLK ),
  .\O (CLK_BUF )
 ) ;
LUT1 #(.INIT(1'h1))
COUNT_0_i_1_inst (
  .\I0 (ext_uartinst_receiverCOUNT_reg_0_0 ),
  .\O (p_0_in_1_0 )
 ) ;
LUT1 #(.INIT(1'h1))
COUNT_0_i_1_0_inst (
  .\I0 (ext_uartinst_transmitterCOUNT_reg_0_0 ),
  .\O (p_0_in_4_0 )
 ) ;
LUT2 #(.INIT(3'h6))
COUNT_1_i_1_inst (
  .\I0 (ext_uartinst_receiverCOUNT_reg_0_0 ),
  .\I1 (ext_uartinst_receiverCOUNT_reg_0_1 ),
  .\O (p_0_in_1_1 )
 ) ;
LUT2 #(.INIT(3'h6))
COUNT_1_i_1_0_inst (
  .\I0 (ext_uartinst_transmitterCOUNT_reg_0_0 ),
  .\I1 (ext_uartinst_transmitterCOUNT_reg_0_1 ),
  .\O (p_0_in_4_1 )
 ) ;
LUT3 #(.INIT(7'h78))
COUNT_2_i_1_inst (
  .\I0 (ext_uartinst_receiverCOUNT_reg_0_0 ),
  .\I1 (ext_uartinst_receiverCOUNT_reg_0_1 ),
  .\I2 (ext_uartinst_receiverCOUNT_reg_0_2 ),
  .\O (p_0_in_1_2 )
 ) ;
LUT3 #(.INIT(7'h78))
COUNT_2_i_1_0_inst (
  .\I0 (ext_uartinst_transmitterCOUNT_reg_0_0 ),
  .\I1 (ext_uartinst_transmitterCOUNT_reg_0_1 ),
  .\I2 (ext_uartinst_transmitterCOUNT_reg_0_2 ),
  .\O (p_0_in_4_2 )
 ) ;
LUT4 #(.INIT(15'h7f80))
COUNT_3_i_1_inst (
  .\I0 (ext_uartinst_transmitterCOUNT_reg_0_1 ),
  .\I1 (ext_uartinst_transmitterCOUNT_reg_0_0 ),
  .\I2 (ext_uartinst_transmitterCOUNT_reg_0_2 ),
  .\I3 (ext_uartinst_transmitterCOUNT_reg_0_3 ),
  .\O (p_0_in_4_3 )
 ) ;
LUT2 #(.INIT(4'h9))
COUNT_3_i_1_0_inst (
  .\I0 (ext_uartinst_receiverSTATE_0 ),
  .\I1 (ext_uartinst_receiverSTATE_1 ),
  .\O (ext_uartinst_receiverCNT_RST )
 ) ;
LUT4 #(.INIT(16'haba8))
FSM_onehot_STATE_0_i_1_inst (
  .\I0 (FSM_onehot_STATE_0_i_2_n_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\I2 (REG_63_i_1_n_0 ),
  .\I3 (TX_IDLE_O ),
  .\O (FSM_onehot_STATE_0_i_1_n_0 )
 ) ;
LUT3 #(.INIT(5'h08))
COUNT_3_i_2_inst (
  .\I0 (ext_uartinst_receiverinst_receiver_DATA_VALID ),
  .\I1 (ext_uartinst_receiverSTATE_0 ),
  .\I2 (ext_uartinst_receiverSTATE_1 ),
  .\O (ext_uartinst_receiverCNT_EN )
 ) ;
LUT4 #(.INIT(15'h7f80))
COUNT_3_i_3_inst (
  .\I0 (ext_uartinst_receiverCOUNT_reg_0_1 ),
  .\I1 (ext_uartinst_receiverCOUNT_reg_0_0 ),
  .\I2 (ext_uartinst_receiverCOUNT_reg_0_2 ),
  .\I3 (ext_uartinst_receiverCOUNT_reg_0_3 ),
  .\O (p_0_in_1_3 )
 ) ;
LUT5 #(.INIT(29'h01000000))
FSM_onehot_STATE_0_i_2_inst (
  .\I0 (ext_uartinst_transmitterCOUNT_reg_0_0 ),
  .\I1 (ext_uartinst_transmitterCOUNT_reg_0_1 ),
  .\I2 (ext_uartinst_transmitterCOUNT_reg_0_2 ),
  .\I3 (ext_uartinst_transmitterFSM_onehot_STATE_reg_n_0_2 ),
  .\I4 (ext_uartinst_transmitterCOUNT_reg_0_3 ),
  .\O (FSM_onehot_STATE_0_i_2_n_0 )
 ) ;
LUT4 #(.INIT(16'haba8))
FSM_onehot_STATE_1_i_1_inst (
  .\I0 (FSM_onehot_STATE_1_i_2_n_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\I2 (REG_63_i_1_n_0 ),
  .\I3 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\O (FSM_onehot_STATE_1_i_1_n_0 )
 ) ;
LUT2 #(.INIT(4'he))
FSM_onehot_STATE_1_i_1_0_inst (
  .\I0 (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_0 ),
  .\I1 (ext_uartinst_receiverinst_receiver_DATA_VALID ),
  .\O (FSM_onehot_STATE_1_i_1_0_n_0 )
 ) ;
LUT6 #(.INIT(64'hffffffffffef0000))
FSM_onehot_STATE_1_i_2_inst (
  .\I0 (ext_uartinst_transmitterCOUNT_reg_0_0 ),
  .\I1 (ext_uartinst_transmitterCOUNT_reg_0_1 ),
  .\I2 (ext_uartinst_transmitterCOUNT_reg_0_3 ),
  .\I3 (ext_uartinst_transmitterCOUNT_reg_0_2 ),
  .\I4 (ext_uartinst_transmitterFSM_onehot_STATE_reg_n_0_2 ),
  .\I5 (TX_IDLE_O ),
  .\O (FSM_onehot_STATE_1_i_2_n_0 )
 ) ;
LUT3 #(.INIT(8'hba))
FSM_onehot_STATE_2_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\I1 (REG_63_i_1_n_0 ),
  .\I2 (ext_uartinst_transmitterFSM_onehot_STATE_reg_n_0_2 ),
  .\O (FSM_onehot_STATE_2_i_1_n_0 )
 ) ;
LUT4 #(.INIT(16'hdfdd))
FSM_sequential_STATE_0_i_1_inst (
  .\I0 (ext_uartinst_receiverSTATE_0 ),
  .\I1 (ext_uartinst_receiverSTATE_1 ),
  .\I2 (ext_uartinst_receiverinst_receiver_DATA_VALID ),
  .\I3 (ext_uartinst_receiverSTATE_0 ),
  .\O (FSM_sequential_STATE_0_i_1_n_0 )
 ) ;
LUT5 #(.INIT(32'haaaeaaa2))
FSM_sequential_STATE_1_i_1_inst (
  .\I0 (ext_uartinst_receiverNEXT_STATE_1 ),
  .\I1 (ext_uartinst_receiverSTATE_0 ),
  .\I2 (ext_uartinst_receiverSTATE_1 ),
  .\I3 (ext_uartinst_receiverinst_receiver_DATA_VALID ),
  .\I4 (ext_uartinst_receiverSTATE_1 ),
  .\O (FSM_sequential_STATE_1_i_1_n_0 )
 ) ;
LUT6 #(.INIT(64'hffffffffefeeeeee))
FSM_onehot_STATE_3_i_1_inst (
  .\I0 (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_0 ),
  .\I1 (ext_uartinst_receiverinst_receiver_DATA_VALID ),
  .\I2 (ext_uartinst_receiverinst_receiverRS232_BIT_SYN ),
  .\I3 (ext_uartinst_receiverinst_receiverRS232_BIT_LAST ),
  .\I4 (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_1 ),
  .\I5 (FSM_onehot_STATE_3_i_2_n_0 ),
  .\O (FSM_onehot_STATE_3_i_1_n_0 )
 ) ;
LUT5 #(.INIT(29'h00004000))
FSM_onehot_STATE_3_i_2_inst (
  .\I0 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_2 ),
  .\I1 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_3 ),
  .\I2 (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_2 ),
  .\I3 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 ),
  .\I4 (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_1 ),
  .\O (FSM_onehot_STATE_3_i_2_n_0 )
 ) ;
LUT5 #(.INIT(31'h55df5510))
FSM_sequential_STATE_0_i_1_0_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSTATE_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\I2 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\I3 (FSM_sequential_STATE_1_i_2_0_n_0 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSTATE_0 ),
  .\O (FSM_sequential_STATE_0_i_1_0_n_0 )
 ) ;
LUT6 #(.INIT(61'h00000100ffff0000))
FSM_sequential_STATE_1_i_2_inst (
  .\I0 (ext_uartinst_receiverCOUNT_reg_0_1 ),
  .\I1 (ext_uartinst_receiverCOUNT_reg_0_0 ),
  .\I2 (ext_uartinst_receiverCOUNT_reg_0_2 ),
  .\I3 (ext_uartinst_receiverCOUNT_reg_0_3 ),
  .\I4 (ext_uartinst_receiverSTATE_0 ),
  .\I5 (ext_uartinst_receiverSTATE_1 ),
  .\O (ext_uartinst_receiverNEXT_STATE_1 )
 ) ;
LUT5 #(.INIT(31'h7777777f))
SEND_CNT_13_i_6_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_11 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_13 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_8 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_9 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_10 ),
  .\O (SEND_CNT_13_i_6_n_0 )
 ) ;
LUT5 #(.INIT(30'h22ef2220))
FSM_sequential_STATE_1_i_1_0_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSTATE_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\I2 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\I3 (FSM_sequential_STATE_1_i_2_0_n_0 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\O (FSM_sequential_STATE_1_i_1_0_n_0 )
 ) ;
LUT5 #(.INIT(29'h0000a8ff))
FSM_sequential_STATE_1_i_2_0_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_3 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_2 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSTATE_0 ),
  .\O (FSM_sequential_STATE_1_i_2_0_n_0 )
 ) ;
GND GND_inst (
  .\O (const0 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_10_i_1_inst (
  .\I0 (REG_reg_n_0_10 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_2 ),
  .\O (p_1_in_10 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_11_i_1_inst (
  .\I0 (REG_reg_n_0_11 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_3 ),
  .\O (p_1_in_11 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_12_i_1_inst (
  .\I0 (REG_reg_n_0_12 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_4 ),
  .\O (p_1_in_12 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_13_i_1_inst (
  .\I0 (REG_reg_n_0_13 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_5 ),
  .\O (p_1_in_13 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_14_i_1_inst (
  .\I0 (REG_reg_n_0_14 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_6 ),
  .\O (p_1_in_14 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_15_i_1_inst (
  .\I0 (REG_reg_n_0_15 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_7 ),
  .\O (p_1_in_15 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_16_i_1_inst (
  .\I0 (REG_reg_n_0_16 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_8 ),
  .\O (p_1_in_16 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_17_i_1_inst (
  .\I0 (REG_reg_n_0_17 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_9 ),
  .\O (p_1_in_17 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_25_i_1_inst (
  .\I0 (REG_reg_n_0_25 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_17 ),
  .\O (p_1_in_25 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_18_i_1_inst (
  .\I0 (REG_reg_n_0_18 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_10 ),
  .\O (p_1_in_18 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_19_i_1_inst (
  .\I0 (REG_reg_n_0_19 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_11 ),
  .\O (p_1_in_19 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_27_i_1_inst (
  .\I0 (REG_reg_n_0_27 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_19 ),
  .\O (p_1_in_27 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_20_i_1_inst (
  .\I0 (REG_reg_n_0_20 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_12 ),
  .\O (p_1_in_20 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_21_i_1_inst (
  .\I0 (REG_reg_n_0_21 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_13 ),
  .\O (p_1_in_21 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_22_i_1_inst (
  .\I0 (REG_reg_n_0_22 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_14 ),
  .\O (p_1_in_22 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_23_i_1_inst (
  .\I0 (REG_reg_n_0_23 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_15 ),
  .\O (p_1_in_23 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_24_i_1_inst (
  .\I0 (REG_reg_n_0_24 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_16 ),
  .\O (p_1_in_24 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_41_i_1_inst (
  .\I0 (REG_reg_n_0_41 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_33 ),
  .\O (p_1_in_41 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_42_i_1_inst (
  .\I0 (REG_reg_n_0_42 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_34 ),
  .\O (p_1_in_42 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_26_i_1_inst (
  .\I0 (REG_reg_n_0_26 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_18 ),
  .\O (p_1_in_26 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_43_i_1_inst (
  .\I0 (REG_reg_n_0_43 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_35 ),
  .\O (p_1_in_43 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_44_i_1_inst (
  .\I0 (REG_reg_n_0_44 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_36 ),
  .\O (p_1_in_44 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_28_i_1_inst (
  .\I0 (REG_reg_n_0_28 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_20 ),
  .\O (p_1_in_28 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_29_i_1_inst (
  .\I0 (REG_reg_n_0_29 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_21 ),
  .\O (p_1_in_29 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_30_i_1_inst (
  .\I0 (REG_reg_n_0_30 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_22 ),
  .\O (p_1_in_30 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_31_i_1_inst (
  .\I0 (REG_reg_n_0_31 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_23 ),
  .\O (p_1_in_31 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_32_i_1_inst (
  .\I0 (REG_reg_n_0_32 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_24 ),
  .\O (p_1_in_32 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_33_i_1_inst (
  .\I0 (REG_reg_n_0_33 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_25 ),
  .\O (p_1_in_33 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_34_i_1_inst (
  .\I0 (REG_reg_n_0_34 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_26 ),
  .\O (p_1_in_34 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_35_i_1_inst (
  .\I0 (REG_reg_n_0_35 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_27 ),
  .\O (p_1_in_35 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_36_i_1_inst (
  .\I0 (REG_reg_n_0_36 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_28 ),
  .\O (p_1_in_36 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_37_i_1_inst (
  .\I0 (REG_reg_n_0_37 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_29 ),
  .\O (p_1_in_37 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_38_i_1_inst (
  .\I0 (REG_reg_n_0_38 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_30 ),
  .\O (p_1_in_38 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_39_i_1_inst (
  .\I0 (REG_reg_n_0_39 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_31 ),
  .\O (p_1_in_39 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_40_i_1_inst (
  .\I0 (REG_reg_n_0_40 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_32 ),
  .\O (p_1_in_40 )
 ) ;
FFR #(.INIT(1'h1))
ext_uartinst_transmitterFSM_onehot_STATE_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (FSM_onehot_STATE_0_i_1_n_0 ),
  .\R (const0 ),
  .\Q (TX_IDLE_O )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterFSM_onehot_STATE_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (FSM_onehot_STATE_1_i_1_n_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterFSM_onehot_STATE_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (FSM_onehot_STATE_2_i_1_n_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterFSM_onehot_STATE_reg_n_0_2 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_45_i_1_inst (
  .\I0 (REG_reg_n_0_45 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_37 ),
  .\O (p_1_in_45 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_46_i_1_inst (
  .\I0 (REG_reg_n_0_46 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_38 ),
  .\O (p_1_in_46 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_47_i_1_inst (
  .\I0 (REG_reg_n_0_47 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_39 ),
  .\O (p_1_in_47 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_48_i_1_inst (
  .\I0 (REG_reg_n_0_48 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_40 ),
  .\O (p_1_in_48 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_49_i_1_inst (
  .\I0 (REG_reg_n_0_49 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_41 ),
  .\O (p_1_in_49 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_50_i_1_inst (
  .\I0 (REG_reg_n_0_50 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_42 ),
  .\O (p_1_in_50 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_51_i_1_inst (
  .\I0 (REG_reg_n_0_51 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_43 ),
  .\O (p_1_in_51 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_52_i_1_inst (
  .\I0 (REG_reg_n_0_52 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_44 ),
  .\O (p_1_in_52 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_53_i_1_inst (
  .\I0 (REG_reg_n_0_53 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_45 ),
  .\O (p_1_in_53 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_54_i_1_inst (
  .\I0 (REG_reg_n_0_54 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_46 ),
  .\O (p_1_in_54 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_55_i_1_inst (
  .\I0 (REG_reg_n_0_55 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_47 ),
  .\O (p_1_in_55 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_56_i_1_inst (
  .\I0 (REG_reg_n_0_56 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_48 ),
  .\O (p_1_in_56 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_57_i_1_inst (
  .\I0 (REG_reg_n_0_57 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_49 ),
  .\O (p_1_in_57 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_58_i_1_inst (
  .\I0 (REG_reg_n_0_58 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_50 ),
  .\O (p_1_in_58 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_59_i_1_inst (
  .\I0 (REG_reg_n_0_59 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_51 ),
  .\O (p_1_in_59 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_60_i_1_inst (
  .\I0 (REG_reg_n_0_60 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_52 ),
  .\O (p_1_in_60 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_61_i_1_inst (
  .\I0 (REG_reg_n_0_61 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_53 ),
  .\O (p_1_in_61 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_62_i_1_inst (
  .\I0 (REG_reg_n_0_62 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_54 ),
  .\O (p_1_in_62 )
 ) ;
LUT6 #(.INIT(64'hffff888f88888888))
REG_63_i_1_inst (
  .\I0 (TX_IDLE_O ),
  .\I1 (STATE ),
  .\I2 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\I4 (FSM_sequential_STATE_1_i_2_0_n_0 ),
  .\I5 (ext_uartinst_transmitterFSM_onehot_STATE_reg_n_0_2 ),
  .\O (REG_63_i_1_n_0 )
 ) ;
LUT2 #(.INIT(1'h1))
BIT_CNT_0_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterCNT_RST ),
  .\I1 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\O (p_0_in_3_0 )
 ) ;
LUT2 #(.INIT(2'h2))
REG_63_i_1_0_inst (
  .\I0 (ext_uartinst_receiverSTATE_1 ),
  .\I1 (ext_uartinst_receiverSTATE_0 ),
  .\O (ext_uartinst_receiverREG_EN )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_63_i_2_inst (
  .\I0 (REG_reg_n_0_63 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_55 ),
  .\O (p_1_in_63 )
 ) ;
LUT6 #(.INIT(61'h0002000000000000))
REG_7_i_1_inst (
  .\I0 (REG_7_i_2_n_0 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 ),
  .\I4 (REG_7_i_3_n_0 ),
  .\I5 (REG_7_i_4_n_0 ),
  .\O (ext_uartinst_receiverinst_receiverREG_EN )
 ) ;
LUT3 #(.INIT(6'h2a))
REG_7_i_1_0_inst (
  .\I0 (REG_63_i_1_n_0 ),
  .\I1 (STATE ),
  .\I2 (TX_IDLE_O ),
  .\O (REG_7_i_1_0_n_0 )
 ) ;
LUT4 #(.INIT(15'h4000))
REG_7_i_2_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_5 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_4 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_3 ),
  .\O (REG_7_i_2_n_0 )
 ) ;
LUT4 #(.INIT(13'h0400))
REG_7_i_4_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_13 ),
  .\I1 (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_2 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_12 ),
  .\O (REG_7_i_4_n_0 )
 ) ;
LUT4 #(.INIT(13'h0004))
REG_7_i_3_inst (
  .\I0 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_9 ),
  .\I1 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_10 ),
  .\I2 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 ),
  .\I3 (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 ),
  .\O (REG_7_i_3_n_0 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_8_i_1_inst (
  .\I0 (REG_reg_n_0_8 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_0 ),
  .\O (p_1_in_8 )
 ) ;
LUT4 #(.INIT(16'hbf80))
REG_9_i_1_inst (
  .\I0 (REG_reg_n_0_9 ),
  .\I1 (TX_IDLE_O ),
  .\I2 (STATE ),
  .\I3 (ext_uartinst_transmitterREG_reg_n_0_1 ),
  .\O (p_1_in_9 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_10_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_10 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_10 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_0 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_0 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_11_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_11 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_11 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_12_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_12 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_12 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_15_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_15 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_15 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_13_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_13 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_13 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_14_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_14 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_14 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_18_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_18 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_18 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_16_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_16 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_16 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_17_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_17 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_17 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_1 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_1 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_19_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_19 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_19 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_25_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_25 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_25 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_26_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_26 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_26 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_28_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_28 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_28 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_20_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_20 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_20 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_21_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_21 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_21 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_22_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_22 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_22 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_23_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_23 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_23 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_24_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_24 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_24 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_3 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_3 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_40_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_40 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_40 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_41_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_41 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_41 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_42_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_42 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_42 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_27_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_27 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_27 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_29_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_29 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_29 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_2 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_2 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_30_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_30 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_30 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_31_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_31 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_31 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_33_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_33 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_33 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_32_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_32 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_32 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_34_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_34 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_34 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_35_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_35 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_35 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_36_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_36 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_36 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_37_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_37 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_37 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_38_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_38 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_38 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_39_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_39 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_39 )
 ) ;
BUF UART_TX_BUF_inst_inst (
  .\I (UART_TX_BUF ),
  .\O (UART_TX )
 ) ;
LUT6 #(.INIT(61'h0000aaa8ffffffff))
UART_TX_BUF_inst_i_1_inst (
  .\I0 (UART_TX_BUF_inst_i_2_n_0 ),
  .\I1 (UART_TX_BUF_inst_i_3_n_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_3 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSTATE_0 ),
  .\I5 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\O (UART_TX_BUF )
 ) ;
LUT6 #(.INIT(64'hfffeeeefaaaeeee0))
UART_TX_BUF_inst_i_2_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_3 ),
  .\I1 (UART_TX_BUF_inst_i_4_n_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_2 ),
  .\I5 (UART_TX_BUF_inst_i_5_n_0 ),
  .\O (UART_TX_BUF_inst_i_2_n_0 )
 ) ;
LUT2 #(.INIT(4'he))
UART_TX_BUF_inst_i_3_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_2 ),
  .\O (UART_TX_BUF_inst_i_3_n_0 )
 ) ;
LUT6 #(.INIT(64'haaccfff0aacc00f0))
UART_TX_BUF_inst_i_5_inst (
  .\I0 (ext_uartinst_transmitterTX_DATA_6 ),
  .\I1 (ext_uartinst_transmitterTX_DATA_4 ),
  .\I2 (ext_uartinst_transmitterTX_DATA_7 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\I5 (ext_uartinst_transmitterTX_DATA_5 ),
  .\O (UART_TX_BUF_inst_i_5_n_0 )
 ) ;
VCC VCC_inst (
  .\O (const1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverCOUNT_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverCNT_EN ),
  .\D (p_0_in_1_0 ),
  .\R (ext_uartinst_receiverCNT_RST ),
  .\Q (ext_uartinst_receiverCOUNT_reg_0_0 )
 ) ;
LUT6 #(.INIT(64'haaccfff0aacc00f0))
UART_TX_BUF_inst_i_4_inst (
  .\I0 (ext_uartinst_transmitterTX_DATA_2 ),
  .\I1 (ext_uartinst_transmitterTX_DATA_0 ),
  .\I2 (ext_uartinst_transmitterTX_DATA_3 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 ),
  .\I5 (ext_uartinst_transmitterTX_DATA_1 ),
  .\O (UART_TX_BUF_inst_i_4_n_0 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_43_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_43 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_43 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_44_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_44 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_44 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_45_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_45 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_45 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_46_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_46 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_46 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_47_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_47 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_47 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_48_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_48 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_48 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_49_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_49 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_49 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_50_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_50 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_50 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_4_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_4 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_4 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_51_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_51 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_51 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_52_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_52 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_52 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_53_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_53 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_53 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_54_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_54 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_54 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_55_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_55 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_55 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_57_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_57 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_57 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_56_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_56 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_56 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_58_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_58 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_58 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_59_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_59 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_59 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_60_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_60 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_60 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_5_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_5 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_5 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_61_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_61 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_61 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_62_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_62 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_62 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_63_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_63 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_63 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_6_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_6 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_6 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_7_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_7 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_7 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_8_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_8 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_8 )
 ) ;
FFR #(.INIT(1'h0))
REG_reg_9_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverREG_reg_n_0_9 ),
  .\R (const0 ),
  .\Q (REG_reg_n_0_9 )
 ) ;
LUT1 #(.INIT(1'h1))
SEND_CNT_0_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ),
  .\O (p_0_in_2_0 )
 ) ;
LUT6 #(.INIT(63'h7fffffff80000000))
SEND_CNT_10_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_8 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ),
  .\I2 (SEND_CNT_10_i_2_n_0 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_9 ),
  .\I5 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_10 ),
  .\O (p_0_in_2_10 )
 ) ;
LUT6 #(.INIT(64'h8000000000000000))
SEND_CNT_10_i_2_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_5 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_3 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_2 ),
  .\I5 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_4 ),
  .\O (SEND_CNT_10_i_2_n_0 )
 ) ;
LUT2 #(.INIT(3'h6))
SEND_CNT_11_i_1_inst (
  .\I0 (SEND_CNT_13_i_7_n_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_11 ),
  .\O (p_0_in_2_11 )
 ) ;
LUT3 #(.INIT(7'h78))
SEND_CNT_12_i_1_inst (
  .\I0 (SEND_CNT_13_i_7_n_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_11 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_12 ),
  .\O (p_0_in_2_12 )
 ) ;
LUT6 #(.INIT(64'hfff8ff00ffffff00))
SEND_CNT_13_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_13 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_12 ),
  .\I2 (SEND_CNT_13_i_4_n_0 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterCNT_RST ),
  .\I4 (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\I5 (SEND_CNT_13_i_6_n_0 ),
  .\O (ext_uartinst_transmitterinst_transmitterBIT_CNT )
 ) ;
LUT3 #(.INIT(7'h58))
SEND_CNT_13_i_2_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSTATE_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\O (ext_uartinst_transmitterinst_transmitterCNT_EN )
 ) ;
LUT4 #(.INIT(15'h7f80))
SEND_CNT_13_i_3_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_11 ),
  .\I1 (SEND_CNT_13_i_7_n_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_12 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_13 ),
  .\O (p_0_in_2_13 )
 ) ;
LUT6 #(.INIT(64'hea00000000000000))
SEND_CNT_13_i_4_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_5 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_4 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_11 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_13 ),
  .\I5 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 ),
  .\O (SEND_CNT_13_i_4_n_0 )
 ) ;
LUT5 #(.INIT(29'h080008f0))
SEND_CNT_13_i_5_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_3 ),
  .\I1 (UART_TX_BUF_inst_i_3_n_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSTATE_0 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSTATE_1 ),
  .\I4 (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\O (ext_uartinst_transmitterinst_transmitterCNT_RST )
 ) ;
LUT6 #(.INIT(64'h8000000000000000))
SEND_CNT_13_i_7_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_10 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_8 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ),
  .\I3 (SEND_CNT_10_i_2_n_0 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 ),
  .\I5 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_9 ),
  .\O (SEND_CNT_13_i_7_n_0 )
 ) ;
LUT2 #(.INIT(3'h6))
SEND_CNT_1_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 ),
  .\O (p_0_in_2_1 )
 ) ;
LUT3 #(.INIT(7'h78))
SEND_CNT_2_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_2 ),
  .\O (p_0_in_2_2 )
 ) ;
LUT4 #(.INIT(15'h7f80))
SEND_CNT_3_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_2 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_3 ),
  .\O (p_0_in_2_3 )
 ) ;
LUT5 #(.INIT(31'h7fff8000))
SEND_CNT_4_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_2 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_3 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_4 ),
  .\O (p_0_in_2_4 )
 ) ;
LUT6 #(.INIT(63'h7fffffff80000000))
SEND_CNT_5_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_3 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_2 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_4 ),
  .\I5 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_5 ),
  .\O (p_0_in_2_5 )
 ) ;
LUT2 #(.INIT(3'h6))
SEND_CNT_6_i_1_inst (
  .\I0 (SEND_CNT_10_i_2_n_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ),
  .\O (p_0_in_2_6 )
 ) ;
LUT3 #(.INIT(7'h78))
SEND_CNT_7_i_1_inst (
  .\I0 (SEND_CNT_10_i_2_n_0 ),
  .\I1 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 ),
  .\O (p_0_in_2_7 )
 ) ;
LUT4 #(.INIT(15'h7f80))
SEND_CNT_8_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ),
  .\I1 (SEND_CNT_10_i_2_n_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_8 ),
  .\O (p_0_in_2_8 )
 ) ;
LUT5 #(.INIT(31'h7fff8000))
SEND_CNT_9_i_1_inst (
  .\I0 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 ),
  .\I1 (SEND_CNT_10_i_2_n_0 ),
  .\I2 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 ),
  .\I3 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_8 ),
  .\I4 (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_9 ),
  .\O (p_0_in_2_9 )
 ) ;
LUT4 #(.INIT(15'h7444))
STATE_i_1_inst (
  .\I0 (TX_IDLE_O ),
  .\I1 (STATE ),
  .\I2 (ext_uartinst_receiverSTATE_0 ),
  .\I3 (ext_uartinst_receiverSTATE_1 ),
  .\O (NEXT_STATE )
 ) ;
BUF UART_RX_BUF_inst_inst (
  .\I (UART_RX ),
  .\O (UART_RX_BUF )
 ) ;
FFR #(.INIT(1'h0))
STATE_reg_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (NEXT_STATE ),
  .\R (const0 ),
  .\Q (STATE )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_63_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_55 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_63 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_6_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_6 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_6 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_7_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_7 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_7 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_8_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_8 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_9_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_1 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_9 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCLK_CNT ),
  .\D (BIT_CNT_0_i_1_0_n_0 ),
  .\R (FSM_onehot_STATE_1_i_1_0_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverBIT_CNT_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCLK_CNT ),
  .\D (p_0_in_0_1 ),
  .\R (FSM_onehot_STATE_1_i_1_0_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverBIT_CNT_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCLK_CNT ),
  .\D (p_0_in_0_2 ),
  .\R (FSM_onehot_STATE_1_i_1_0_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverBIT_CNT_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCLK_CNT ),
  .\D (p_0_in_0_3 ),
  .\R (FSM_onehot_STATE_1_i_1_0_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverBIT_CNT_reg_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverCOUNT_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverCNT_EN ),
  .\D (p_0_in_1_1 ),
  .\R (ext_uartinst_receiverCNT_RST ),
  .\Q (ext_uartinst_receiverCOUNT_reg_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverCOUNT_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverCNT_EN ),
  .\D (p_0_in_1_2 ),
  .\R (ext_uartinst_receiverCNT_RST ),
  .\Q (ext_uartinst_receiverCOUNT_reg_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverCOUNT_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverCNT_EN ),
  .\D (p_0_in_1_3 ),
  .\R (ext_uartinst_receiverCNT_RST ),
  .\Q (ext_uartinst_receiverCOUNT_reg_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverFSM_sequential_STATE_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (FSM_sequential_STATE_0_i_1_n_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverSTATE_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverFSM_sequential_STATE_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (FSM_sequential_STATE_1_i_1_n_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverSTATE_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_10_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_2 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_10 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_11_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_3 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_11 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_12_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_4 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_12 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_13_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_5 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_13 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_14_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_6 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_14 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_15_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_7 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_15 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_16_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_8 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_16 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_17_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_9 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_17 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_18_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_10 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_18 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_19_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_11 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_19 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_1 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_20_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_12 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_20 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_21_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_13 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_21 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_22_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_14 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_22 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_23_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_15 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_23 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_24_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_16 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_24 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_25_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_17 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_25 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_3 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_40_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_32 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_40 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_26_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_18 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_26 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_27_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_19 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_27 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_28_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_20 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_28 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_29_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_21 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_29 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_2 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_30_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_22 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_30 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_31_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_23 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_31 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_32_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_24 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_32 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_33_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_25 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_33 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_34_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_26 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_34 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_35_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_27 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_35 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_36_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_28 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_36 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_37_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_29 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_37 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_38_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_30 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_38 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_39_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_31 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_39 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_41_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_33 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_41 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_42_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_34 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_42 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_43_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_35 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_43 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_44_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_36 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_44 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_45_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_37 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_45 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_46_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_38 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_46 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_47_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_39 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_47 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_48_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_40 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_48 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_49_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_41 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_49 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_4_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_4 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_4 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_50_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_42 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_50 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_51_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_43 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_51 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_52_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_44 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_52 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_53_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_45 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_53 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_54_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_46 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_54 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_55_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_47 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_55 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_56_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_48 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_56 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_57_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_49 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_57 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_58_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_50 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_58 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_59_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_51 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_59 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_5_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (REG_5 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_5 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_60_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_52 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_60 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_61_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_53 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_61 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverREG_reg_62_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverREG_EN ),
  .\D (ext_uartinst_receiverREG_reg_n_0_54 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverREG_reg_n_0_62 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_1 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_0 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_10_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_10 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_10 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_11_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_11 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_11 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_12_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_12 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_12 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_13_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_13 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_13 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_8_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_8 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_8 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_5_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_5 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_5 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_2 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_3 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_4_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_4 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_4 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_6_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_6 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_6 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_7_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_7 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_7 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverCLK_CNT_reg_9_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverCNT_EN ),
  .\D (p_0_in_9 ),
  .\R (CLK_CNT_13_i_1_n_0 ),
  .\Q (ext_uartinst_receiverinst_receiverCLK_CNT_reg_0_9 )
 ) ;
FFR #(.INIT(1'h1))
ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (FSM_onehot_STATE_3_i_1_n_0 ),
  .\D (const0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (FSM_onehot_STATE_3_i_1_n_0 ),
  .\D (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_2 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverinst_receiver_DATA_VALID )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (FSM_onehot_STATE_3_i_1_n_0 ),
  .\D (FSM_onehot_STATE_1_i_1_0_n_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (FSM_onehot_STATE_3_i_1_n_0 ),
  .\D (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_1 ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverinst_receiverFSM_onehot_STATE_reg_n_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (REG_1 ),
  .\R (const0 ),
  .\Q (REG_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (REG_2 ),
  .\R (const0 ),
  .\Q (REG_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (REG_3 ),
  .\R (const0 ),
  .\Q (REG_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (REG_4 ),
  .\R (const0 ),
  .\Q (REG_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_4_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (REG_5 ),
  .\R (const0 ),
  .\Q (REG_4 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_5_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (REG_6 ),
  .\R (const0 ),
  .\Q (REG_5 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_6_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (REG_7 ),
  .\R (const0 ),
  .\Q (REG_6 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverREG_reg_7_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_receiverinst_receiverREG_EN ),
  .\D (UART_RX_BUF ),
  .\R (const0 ),
  .\Q (REG_7 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverRS232_BIT_LAST_reg_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (ext_uartinst_receiverinst_receiverRS232_BIT_SYN ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverinst_receiverRS232_BIT_LAST )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_receiverinst_receiverRS232_BIT_SYN_reg_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (UART_RX_BUF ),
  .\R (const0 ),
  .\Q (ext_uartinst_receiverinst_receiverRS232_BIT_SYN )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterCOUNT_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\D (p_0_in_4_0 ),
  .\R (TX_IDLE_O ),
  .\Q (ext_uartinst_transmitterCOUNT_reg_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterCOUNT_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\D (p_0_in_4_1 ),
  .\R (TX_IDLE_O ),
  .\Q (ext_uartinst_transmitterCOUNT_reg_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterCOUNT_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\D (p_0_in_4_2 ),
  .\R (TX_IDLE_O ),
  .\Q (ext_uartinst_transmitterCOUNT_reg_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterCOUNT_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitter_SEND_ENABLE ),
  .\D (p_0_in_4_3 ),
  .\R (TX_IDLE_O ),
  .\Q (ext_uartinst_transmitterCOUNT_reg_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_0 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_10_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_10 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_10 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_11_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_11 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_11 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_12_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_12 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_12 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_13_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_13 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_13 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_14_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_14 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_14 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_15_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_15 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_15 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_16_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_16 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_16 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_17_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_17 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_17 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_18_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_18 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_18 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_19_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_19 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_19 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_1 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_20_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_20 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_20 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_21_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_21 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_21 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_37_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_37 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_37 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_22_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_22 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_22 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_23_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_23 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_23 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_24_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_24 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_24 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_25_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_25 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_25 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_26_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_26 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_26 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_27_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_27 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_27 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_28_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_28 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_28 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_29_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_29 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_29 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_2 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_30_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_30 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_30 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_31_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_31 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_31 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_32_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_32 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_32 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_33_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_33 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_33 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_34_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_34 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_34 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_35_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_35 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_35 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_36_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_36 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_36 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_8_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_8 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_8 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\D (p_0_in_3_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_9_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_9 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_9 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_38_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_38 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_38 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_39_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_39 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_39 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_3 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_40_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_40 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_40 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_41_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_41 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_41 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_42_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_42 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_42 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_43_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_43 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_43 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_44_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_44 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_44 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_45_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_45 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_45 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_46_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_46 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_46 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_47_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_47 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_47 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_48_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_48 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_48 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_49_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_49 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_49 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_4_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_4 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_4 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_50_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_50 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_50 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_51_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_51 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_51 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_52_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_52 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_52 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_53_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_53 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_53 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_54_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_54 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_54 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_55_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_55 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_55 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_56_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_56 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_57_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_57 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_58_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_58 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_59_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_59 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_5_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_5 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_5 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_60_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_60 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_4 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_61_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_61 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_5 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_62_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_62 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_6 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_63_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (p_1_in_63 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterTX_DATA_7 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_6_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_6 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_6 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterREG_reg_7_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (REG_63_i_1_n_0 ),
  .\D (REG_reg_n_0_7 ),
  .\R (REG_7_i_1_0_n_0 ),
  .\Q (ext_uartinst_transmitterREG_reg_n_0_7 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\D (p_0_in_3_1 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\D (p_0_in_3_2 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterFSM_sequential_STATE_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (FSM_sequential_STATE_0_i_1_0_n_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterinst_transmitterSTATE_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\D (p_0_in_3_3 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterinst_transmitterBIT_CNT_reg_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_0 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_0 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterFSM_sequential_STATE_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (const1 ),
  .\D (FSM_sequential_STATE_1_i_1_0_n_0 ),
  .\R (const0 ),
  .\Q (ext_uartinst_transmitterinst_transmitterSTATE_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_10_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_10 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_10 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_11_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_11 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_11 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_12_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_12 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_12 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_5_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_5 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_5 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_13_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_13 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_13 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_1_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_1 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_1 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_2_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_2 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_2 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_3_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_3 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_n_0_3 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_4_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_4 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_4 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_6_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_6 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_6 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_7_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_7 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_7 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_8_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_8 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_8 )
 ) ;
FFR #(.INIT(1'h0))
ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_9_inst (
  .\C (CLK_BUF_BUF ),
  .\CE (ext_uartinst_transmitterinst_transmitterCNT_EN ),
  .\D (p_0_in_2_9 ),
  .\R (ext_uartinst_transmitterinst_transmitterBIT_CNT ),
  .\Q (ext_uartinst_transmitterinst_transmitterSEND_CNT_reg_0_9 )
 ) ;
endmodule
