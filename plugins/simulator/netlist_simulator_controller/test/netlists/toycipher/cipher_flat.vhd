Library UNISIM;
use UNISIM.vcomponents.all;
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity CIPHER is
  port (
CLK : in STD_LOGIC := 'X'; 
  KEY_0 : in STD_LOGIC := 'X'; 
  KEY_1 : in STD_LOGIC := 'X'; 
  KEY_10 : in STD_LOGIC := 'X'; 
  KEY_11 : in STD_LOGIC := 'X'; 
  KEY_12 : in STD_LOGIC := 'X'; 
  KEY_13 : in STD_LOGIC := 'X'; 
  KEY_14 : in STD_LOGIC := 'X'; 
  KEY_15 : in STD_LOGIC := 'X'; 
  KEY_2 : in STD_LOGIC := 'X'; 
  KEY_3 : in STD_LOGIC := 'X'; 
  KEY_4 : in STD_LOGIC := 'X'; 
  KEY_5 : in STD_LOGIC := 'X'; 
  KEY_6 : in STD_LOGIC := 'X'; 
  KEY_7 : in STD_LOGIC := 'X'; 
  KEY_8 : in STD_LOGIC := 'X'; 
  KEY_9 : in STD_LOGIC := 'X'; 
  PLAINTEXT_0 : in STD_LOGIC := 'X'; 
  PLAINTEXT_1 : in STD_LOGIC := 'X'; 
  PLAINTEXT_10 : in STD_LOGIC := 'X'; 
  PLAINTEXT_11 : in STD_LOGIC := 'X'; 
  PLAINTEXT_12 : in STD_LOGIC := 'X'; 
  PLAINTEXT_13 : in STD_LOGIC := 'X'; 
  PLAINTEXT_14 : in STD_LOGIC := 'X'; 
  PLAINTEXT_15 : in STD_LOGIC := 'X'; 
  PLAINTEXT_2 : in STD_LOGIC := 'X'; 
  PLAINTEXT_3 : in STD_LOGIC := 'X'; 
  PLAINTEXT_4 : in STD_LOGIC := 'X'; 
  PLAINTEXT_5 : in STD_LOGIC := 'X'; 
  PLAINTEXT_6 : in STD_LOGIC := 'X'; 
  PLAINTEXT_7 : in STD_LOGIC := 'X'; 
  PLAINTEXT_8 : in STD_LOGIC := 'X'; 
  PLAINTEXT_9 : in STD_LOGIC := 'X'; 
  START : in STD_LOGIC := 'X'; 
  DONE : out STD_LOGIC; 
  OUTPUT_0 : out STD_LOGIC; 
  OUTPUT_1 : out STD_LOGIC; 
  OUTPUT_10 : out STD_LOGIC; 
  OUTPUT_11 : out STD_LOGIC; 
  OUTPUT_12 : out STD_LOGIC; 
  OUTPUT_13 : out STD_LOGIC; 
  OUTPUT_14 : out STD_LOGIC; 
  OUTPUT_15 : out STD_LOGIC; 
  OUTPUT_2 : out STD_LOGIC; 
  OUTPUT_3 : out STD_LOGIC; 
  OUTPUT_4 : out STD_LOGIC; 
  OUTPUT_5 : out STD_LOGIC; 
  OUTPUT_6 : out STD_LOGIC; 
  OUTPUT_7 : out STD_LOGIC; 
  OUTPUT_8 : out STD_LOGIC; 
  OUTPUT_9 : out STD_LOGIC
);
end CIPHER;

architecture STRUCTURE of CIPHER is
  signal CLK_IBUF : STD_LOGIC;
  signal CLK_IBUF_BUFG : STD_LOGIC;
  signal COUNTER_DONE : STD_LOGIC;
  signal COUNTER_REG : STD_LOGIC;
  signal COUNTER_REG_reg_0_0 : STD_LOGIC;
  signal COUNTER_REG_reg_0_1 : STD_LOGIC;
  signal COUNTER_REG_reg_0_2 : STD_LOGIC;
  signal COUNTER_REG_reg_0_3 : STD_LOGIC;
  signal FSM_onehot_STATE_REG_0_i_1_n_0 : STD_LOGIC;
  signal FSM_onehot_STATE_REG_1_i_1_n_0 : STD_LOGIC;
  signal FSM_onehot_STATE_REG_2_i_1_n_0 : STD_LOGIC;
  signal FSM_onehot_STATE_REG_reg_n_0_1 : STD_LOGIC;
  signal FSM_onehot_STATE_REG_reg_n_0_2 : STD_LOGIC;
  signal FSM_onehot_STATE_REG_reg_n_0_3 : STD_LOGIC;
  signal KEY_IBUF_0 : STD_LOGIC;
  signal KEY_IBUF_1 : STD_LOGIC;
  signal KEY_IBUF_2 : STD_LOGIC;
  signal KEY_IBUF_3 : STD_LOGIC;
  signal KEY_IBUF_4 : STD_LOGIC;
  signal KEY_IBUF_5 : STD_LOGIC;
  signal KEY_IBUF_6 : STD_LOGIC;
  signal KEY_IBUF_7 : STD_LOGIC;
  signal KEY_IBUF_8 : STD_LOGIC;
  signal KEY_IBUF_9 : STD_LOGIC;
  signal KEY_IBUF_10 : STD_LOGIC;
  signal KEY_IBUF_11 : STD_LOGIC;
  signal KEY_IBUF_12 : STD_LOGIC;
  signal KEY_IBUF_13 : STD_LOGIC;
  signal KEY_IBUF_14 : STD_LOGIC;
  signal KEY_IBUF_15 : STD_LOGIC;
  signal OUTPUT_OBUF_0 : STD_LOGIC;
  signal OUTPUT_OBUF_1 : STD_LOGIC;
  signal OUTPUT_OBUF_2 : STD_LOGIC;
  signal OUTPUT_OBUF_3 : STD_LOGIC;
  signal OUTPUT_OBUF_4 : STD_LOGIC;
  signal OUTPUT_OBUF_5 : STD_LOGIC;
  signal OUTPUT_OBUF_6 : STD_LOGIC;
  signal OUTPUT_OBUF_7 : STD_LOGIC;
  signal OUTPUT_OBUF_8 : STD_LOGIC;
  signal OUTPUT_OBUF_9 : STD_LOGIC;
  signal OUTPUT_OBUF_10 : STD_LOGIC;
  signal OUTPUT_OBUF_11 : STD_LOGIC;
  signal OUTPUT_OBUF_12 : STD_LOGIC;
  signal OUTPUT_OBUF_13 : STD_LOGIC;
  signal OUTPUT_OBUF_14 : STD_LOGIC;
  signal OUTPUT_OBUF_15 : STD_LOGIC;
  signal PLAINTEXT_IBUF_0 : STD_LOGIC;
  signal PLAINTEXT_IBUF_1 : STD_LOGIC;
  signal PLAINTEXT_IBUF_2 : STD_LOGIC;
  signal PLAINTEXT_IBUF_3 : STD_LOGIC;
  signal PLAINTEXT_IBUF_4 : STD_LOGIC;
  signal PLAINTEXT_IBUF_5 : STD_LOGIC;
  signal PLAINTEXT_IBUF_6 : STD_LOGIC;
  signal PLAINTEXT_IBUF_7 : STD_LOGIC;
  signal PLAINTEXT_IBUF_8 : STD_LOGIC;
  signal PLAINTEXT_IBUF_9 : STD_LOGIC;
  signal PLAINTEXT_IBUF_10 : STD_LOGIC;
  signal PLAINTEXT_IBUF_11 : STD_LOGIC;
  signal PLAINTEXT_IBUF_12 : STD_LOGIC;
  signal PLAINTEXT_IBUF_13 : STD_LOGIC;
  signal PLAINTEXT_IBUF_14 : STD_LOGIC;
  signal PLAINTEXT_IBUF_15 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_0 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_1 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_2 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_3 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_4 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_5 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_6 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_7 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_8 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_9 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_10 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_11 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_12 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_13 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_14 : STD_LOGIC;
  signal ROUND_AFTER_SBOX_15 : STD_LOGIC;
  signal ROUND_REG_0_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_10_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_11_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_12_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_13_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_14_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_15_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_15_i_2_n_0 : STD_LOGIC;
  signal ROUND_REG_1_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_2_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_3_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_4_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_5_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_6_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_7_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_8_i_1_n_0 : STD_LOGIC;
  signal ROUND_REG_9_i_1_n_0 : STD_LOGIC;
  signal ROUND_XOR_0 : STD_LOGIC;
  signal ROUND_XOR_1 : STD_LOGIC;
  signal ROUND_XOR_2 : STD_LOGIC;
  signal ROUND_XOR_3 : STD_LOGIC;
  signal ROUND_XOR_4 : STD_LOGIC;
  signal ROUND_XOR_5 : STD_LOGIC;
  signal ROUND_XOR_6 : STD_LOGIC;
  signal ROUND_XOR_7 : STD_LOGIC;
  signal ROUND_XOR_8 : STD_LOGIC;
  signal ROUND_XOR_9 : STD_LOGIC;
  signal ROUND_XOR_10 : STD_LOGIC;
  signal ROUND_XOR_11 : STD_LOGIC;
  signal ROUND_XOR_12 : STD_LOGIC;
  signal ROUND_XOR_13 : STD_LOGIC;
  signal ROUND_XOR_14 : STD_LOGIC;
  signal ROUND_XOR_15 : STD_LOGIC;
  signal START_IBUF : STD_LOGIC;
  signal plusOp_0 : STD_LOGIC;
  signal plusOp_1 : STD_LOGIC;
  signal plusOp_2 : STD_LOGIC;
  signal plusOp_3 : STD_LOGIC;
begin
CLK_IBUF_BUFG_inst : BUFG
 port map (
   I => CLK_IBUF,
   O => CLK_IBUF_BUFG
);
CLK_IBUF_inst : IBUF
 port map (
   I => CLK,
   O => CLK_IBUF
);
COUNTER_DONE_inferred_i_1 : LUT4
  generic map(
   INIT => X"eaaa"
  )
 port map (
   I0 => COUNTER_REG_reg_0_3,
   I1 => COUNTER_REG_reg_0_1,
   I2 => COUNTER_REG_reg_0_0,
   I3 => COUNTER_REG_reg_0_2,
   O => COUNTER_DONE
);
COUNTER_REG_0_i_1 : LUT1
  generic map(
   INIT => X"1"
  )
 port map (
   I0 => COUNTER_REG_reg_0_0,
   O => plusOp_0
);
COUNTER_REG_1_i_1 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => COUNTER_REG_reg_0_0,
   I1 => COUNTER_REG_reg_0_1,
   O => plusOp_1
);
COUNTER_REG_2_i_1 : LUT3
  generic map(
   INIT => X"78"
  )
 port map (
   I0 => COUNTER_REG_reg_0_0,
   I1 => COUNTER_REG_reg_0_1,
   I2 => COUNTER_REG_reg_0_2,
   O => plusOp_2
);
COUNTER_REG_3_i_1 : LUT4
  generic map(
   INIT => X"7f80"
  )
 port map (
   I0 => COUNTER_REG_reg_0_1,
   I1 => COUNTER_REG_reg_0_0,
   I2 => COUNTER_REG_reg_0_2,
   I3 => COUNTER_REG_reg_0_3,
   O => plusOp_3
);
COUNTER_REG_reg_0 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => FSM_onehot_STATE_REG_reg_n_0_1,
   D => plusOp_0,
   R => COUNTER_REG,
   Q => COUNTER_REG_reg_0_0
);
COUNTER_REG_reg_1 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => FSM_onehot_STATE_REG_reg_n_0_1,
   D => plusOp_1,
   R => COUNTER_REG,
   Q => COUNTER_REG_reg_0_1
);
COUNTER_REG_reg_2 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => FSM_onehot_STATE_REG_reg_n_0_1,
   D => plusOp_2,
   R => COUNTER_REG,
   Q => COUNTER_REG_reg_0_2
);
COUNTER_REG_reg_3 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => FSM_onehot_STATE_REG_reg_n_0_1,
   D => plusOp_3,
   R => COUNTER_REG,
   Q => COUNTER_REG_reg_0_3
);
DONE_OBUF_inst : OBUF
 port map (
   I => FSM_onehot_STATE_REG_reg_n_0_3,
   O => DONE
);
FSM_onehot_STATE_REG_0_i_1 : LUT3
  generic map(
   INIT => X"ba"
  )
 port map (
   I0 => FSM_onehot_STATE_REG_reg_n_0_3,
   I1 => START_IBUF,
   I2 => COUNTER_REG,
   O => FSM_onehot_STATE_REG_0_i_1_n_0
);
FSM_onehot_STATE_REG_1_i_1 : LUT4
  generic map(
   INIT => X"8f88"
  )
 port map (
   I0 => START_IBUF,
   I1 => COUNTER_REG,
   I2 => COUNTER_DONE,
   I3 => FSM_onehot_STATE_REG_reg_n_0_1,
   O => FSM_onehot_STATE_REG_1_i_1_n_0
);
FSM_onehot_STATE_REG_2_i_1 : LUT2
  generic map(
   INIT => X"8"
  )
 port map (
   I0 => FSM_onehot_STATE_REG_reg_n_0_1,
   I1 => COUNTER_DONE,
   O => FSM_onehot_STATE_REG_2_i_1_n_0
);
FSM_onehot_STATE_REG_reg_0 : FDRE
  generic map(
   INIT => '1'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => '1',
   D => FSM_onehot_STATE_REG_0_i_1_n_0,
   R => '0',
   Q => COUNTER_REG
);
FSM_onehot_STATE_REG_reg_1 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => '1',
   D => FSM_onehot_STATE_REG_1_i_1_n_0,
   R => '0',
   Q => FSM_onehot_STATE_REG_reg_n_0_1
);
FSM_onehot_STATE_REG_reg_2 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => '1',
   D => FSM_onehot_STATE_REG_2_i_1_n_0,
   R => '0',
   Q => FSM_onehot_STATE_REG_reg_n_0_2
);
FSM_onehot_STATE_REG_reg_3 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => '1',
   D => FSM_onehot_STATE_REG_reg_n_0_2,
   R => '0',
   Q => FSM_onehot_STATE_REG_reg_n_0_3
);
KEY_IBUF_0_inst : IBUF
 port map (
   I => KEY_0,
   O => KEY_IBUF_0
);
KEY_IBUF_10_inst : IBUF
 port map (
   I => KEY_10,
   O => KEY_IBUF_10
);
KEY_IBUF_11_inst : IBUF
 port map (
   I => KEY_11,
   O => KEY_IBUF_11
);
KEY_IBUF_12_inst : IBUF
 port map (
   I => KEY_12,
   O => KEY_IBUF_12
);
KEY_IBUF_13_inst : IBUF
 port map (
   I => KEY_13,
   O => KEY_IBUF_13
);
KEY_IBUF_14_inst : IBUF
 port map (
   I => KEY_14,
   O => KEY_IBUF_14
);
KEY_IBUF_15_inst : IBUF
 port map (
   I => KEY_15,
   O => KEY_IBUF_15
);
KEY_IBUF_1_inst : IBUF
 port map (
   I => KEY_1,
   O => KEY_IBUF_1
);
KEY_IBUF_2_inst : IBUF
 port map (
   I => KEY_2,
   O => KEY_IBUF_2
);
KEY_IBUF_3_inst : IBUF
 port map (
   I => KEY_3,
   O => KEY_IBUF_3
);
KEY_IBUF_4_inst : IBUF
 port map (
   I => KEY_4,
   O => KEY_IBUF_4
);
KEY_IBUF_5_inst : IBUF
 port map (
   I => KEY_5,
   O => KEY_IBUF_5
);
KEY_IBUF_6_inst : IBUF
 port map (
   I => KEY_6,
   O => KEY_IBUF_6
);
KEY_IBUF_7_inst : IBUF
 port map (
   I => KEY_7,
   O => KEY_IBUF_7
);
KEY_IBUF_8_inst : IBUF
 port map (
   I => KEY_8,
   O => KEY_IBUF_8
);
KEY_IBUF_9_inst : IBUF
 port map (
   I => KEY_9,
   O => KEY_IBUF_9
);
OUTPUT_OBUF_0_inst : OBUF
 port map (
   I => OUTPUT_OBUF_0,
   O => OUTPUT_0
);
OUTPUT_OBUF_10_inst : OBUF
 port map (
   I => OUTPUT_OBUF_10,
   O => OUTPUT_10
);
OUTPUT_OBUF_11_inst : OBUF
 port map (
   I => OUTPUT_OBUF_11,
   O => OUTPUT_11
);
OUTPUT_OBUF_12_inst : OBUF
 port map (
   I => OUTPUT_OBUF_12,
   O => OUTPUT_12
);
OUTPUT_OBUF_13_inst : OBUF
 port map (
   I => OUTPUT_OBUF_13,
   O => OUTPUT_13
);
OUTPUT_OBUF_14_inst : OBUF
 port map (
   I => OUTPUT_OBUF_14,
   O => OUTPUT_14
);
OUTPUT_OBUF_15_inst : OBUF
 port map (
   I => OUTPUT_OBUF_15,
   O => OUTPUT_15
);
OUTPUT_OBUF_1_inst : OBUF
 port map (
   I => OUTPUT_OBUF_1,
   O => OUTPUT_1
);
OUTPUT_OBUF_2_inst : OBUF
 port map (
   I => OUTPUT_OBUF_2,
   O => OUTPUT_2
);
OUTPUT_OBUF_3_inst : OBUF
 port map (
   I => OUTPUT_OBUF_3,
   O => OUTPUT_3
);
OUTPUT_OBUF_4_inst : OBUF
 port map (
   I => OUTPUT_OBUF_4,
   O => OUTPUT_4
);
OUTPUT_OBUF_5_inst : OBUF
 port map (
   I => OUTPUT_OBUF_5,
   O => OUTPUT_5
);
OUTPUT_OBUF_6_inst : OBUF
 port map (
   I => OUTPUT_OBUF_6,
   O => OUTPUT_6
);
OUTPUT_OBUF_7_inst : OBUF
 port map (
   I => OUTPUT_OBUF_7,
   O => OUTPUT_7
);
OUTPUT_OBUF_8_inst : OBUF
 port map (
   I => OUTPUT_OBUF_8,
   O => OUTPUT_8
);
OUTPUT_OBUF_9_inst : OBUF
 port map (
   I => OUTPUT_OBUF_9,
   O => OUTPUT_9
);
PLAINTEXT_IBUF_0_inst : IBUF
 port map (
   I => PLAINTEXT_0,
   O => PLAINTEXT_IBUF_0
);
PLAINTEXT_IBUF_10_inst : IBUF
 port map (
   I => PLAINTEXT_10,
   O => PLAINTEXT_IBUF_10
);
PLAINTEXT_IBUF_11_inst : IBUF
 port map (
   I => PLAINTEXT_11,
   O => PLAINTEXT_IBUF_11
);
PLAINTEXT_IBUF_12_inst : IBUF
 port map (
   I => PLAINTEXT_12,
   O => PLAINTEXT_IBUF_12
);
PLAINTEXT_IBUF_13_inst : IBUF
 port map (
   I => PLAINTEXT_13,
   O => PLAINTEXT_IBUF_13
);
PLAINTEXT_IBUF_14_inst : IBUF
 port map (
   I => PLAINTEXT_14,
   O => PLAINTEXT_IBUF_14
);
PLAINTEXT_IBUF_15_inst : IBUF
 port map (
   I => PLAINTEXT_15,
   O => PLAINTEXT_IBUF_15
);
PLAINTEXT_IBUF_1_inst : IBUF
 port map (
   I => PLAINTEXT_1,
   O => PLAINTEXT_IBUF_1
);
PLAINTEXT_IBUF_2_inst : IBUF
 port map (
   I => PLAINTEXT_2,
   O => PLAINTEXT_IBUF_2
);
PLAINTEXT_IBUF_3_inst : IBUF
 port map (
   I => PLAINTEXT_3,
   O => PLAINTEXT_IBUF_3
);
PLAINTEXT_IBUF_4_inst : IBUF
 port map (
   I => PLAINTEXT_4,
   O => PLAINTEXT_IBUF_4
);
PLAINTEXT_IBUF_5_inst : IBUF
 port map (
   I => PLAINTEXT_5,
   O => PLAINTEXT_IBUF_5
);
PLAINTEXT_IBUF_6_inst : IBUF
 port map (
   I => PLAINTEXT_6,
   O => PLAINTEXT_IBUF_6
);
PLAINTEXT_IBUF_7_inst : IBUF
 port map (
   I => PLAINTEXT_7,
   O => PLAINTEXT_IBUF_7
);
PLAINTEXT_IBUF_8_inst : IBUF
 port map (
   I => PLAINTEXT_8,
   O => PLAINTEXT_IBUF_8
);
PLAINTEXT_IBUF_9_inst : IBUF
 port map (
   I => PLAINTEXT_9,
   O => PLAINTEXT_IBUF_9
);
ROUND_REG_0_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_0,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_0,
   O => ROUND_REG_0_i_1_n_0
);
ROUND_REG_10_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_10,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_10,
   O => ROUND_REG_10_i_1_n_0
);
ROUND_REG_11_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_11,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_14,
   O => ROUND_REG_11_i_1_n_0
);
ROUND_REG_12_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_12,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_3,
   O => ROUND_REG_12_i_1_n_0
);
ROUND_REG_13_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_13,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_7,
   O => ROUND_REG_13_i_1_n_0
);
ROUND_REG_14_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_14,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_11,
   O => ROUND_REG_14_i_1_n_0
);
ROUND_REG_15_i_1 : LUT3
  generic map(
   INIT => X"fd"
  )
 port map (
   I0 => FSM_onehot_STATE_REG_reg_n_0_3,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   O => ROUND_REG_15_i_1_n_0
);
ROUND_REG_15_i_2 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_15,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_15,
   O => ROUND_REG_15_i_2_n_0
);
ROUND_REG_1_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_1,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_4,
   O => ROUND_REG_1_i_1_n_0
);
ROUND_REG_2_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_2,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_8,
   O => ROUND_REG_2_i_1_n_0
);
ROUND_REG_3_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_3,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_12,
   O => ROUND_REG_3_i_1_n_0
);
ROUND_REG_4_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_4,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_1,
   O => ROUND_REG_4_i_1_n_0
);
ROUND_REG_5_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_5,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_5,
   O => ROUND_REG_5_i_1_n_0
);
ROUND_REG_6_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_6,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_9,
   O => ROUND_REG_6_i_1_n_0
);
ROUND_REG_7_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_7,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_13,
   O => ROUND_REG_7_i_1_n_0
);
ROUND_REG_8_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_8,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_2,
   O => ROUND_REG_8_i_1_n_0
);
ROUND_REG_9_i_1 : LUT4
  generic map(
   INIT => X"fe02"
  )
 port map (
   I0 => PLAINTEXT_IBUF_9,
   I1 => FSM_onehot_STATE_REG_reg_n_0_1,
   I2 => FSM_onehot_STATE_REG_reg_n_0_2,
   I3 => ROUND_AFTER_SBOX_6,
   O => ROUND_REG_9_i_1_n_0
);
ROUND_REG_reg_0 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_0_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_0
);
ROUND_REG_reg_10 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_10_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_10
);
ROUND_REG_reg_11 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_11_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_11
);
ROUND_REG_reg_12 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_12_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_12
);
ROUND_REG_reg_13 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_13_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_13
);
ROUND_REG_reg_14 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_14_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_14
);
ROUND_REG_reg_15 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_15_i_2_n_0,
   R => '0',
   Q => OUTPUT_OBUF_15
);
ROUND_REG_reg_1 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_1_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_1
);
ROUND_REG_reg_2 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_2_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_2
);
ROUND_REG_reg_3 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_3_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_3
);
ROUND_REG_reg_4 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_4_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_4
);
ROUND_REG_reg_5 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_5_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_5
);
ROUND_REG_reg_6 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_6_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_6
);
ROUND_REG_reg_7 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_7_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_7
);
ROUND_REG_reg_8 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_8_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_8
);
ROUND_REG_reg_9 : FDRE
  generic map(
   INIT => '0'
  )
 port map (
   C => CLK_IBUF_BUFG,
   CE => ROUND_REG_15_i_1_n_0,
   D => ROUND_REG_9_i_1_n_0,
   R => '0',
   Q => OUTPUT_OBUF_9
);
ROUND_XOR_inferred_i_1 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_15,
   I1 => OUTPUT_OBUF_15,
   O => ROUND_XOR_15
);
ROUND_XOR_inferred_i_10 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_6,
   I1 => OUTPUT_OBUF_6,
   O => ROUND_XOR_6
);
ROUND_XOR_inferred_i_11 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_5,
   I1 => OUTPUT_OBUF_5,
   O => ROUND_XOR_5
);
ROUND_XOR_inferred_i_12 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_4,
   I1 => OUTPUT_OBUF_4,
   O => ROUND_XOR_4
);
ROUND_XOR_inferred_i_13 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_3,
   I1 => OUTPUT_OBUF_3,
   O => ROUND_XOR_3
);
ROUND_XOR_inferred_i_14 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_2,
   I1 => OUTPUT_OBUF_2,
   O => ROUND_XOR_2
);
ROUND_XOR_inferred_i_15 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_1,
   I1 => OUTPUT_OBUF_1,
   O => ROUND_XOR_1
);
ROUND_XOR_inferred_i_16 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_0,
   I1 => OUTPUT_OBUF_0,
   O => ROUND_XOR_0
);
ROUND_XOR_inferred_i_2 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_14,
   I1 => OUTPUT_OBUF_14,
   O => ROUND_XOR_14
);
ROUND_XOR_inferred_i_3 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_13,
   I1 => OUTPUT_OBUF_13,
   O => ROUND_XOR_13
);
ROUND_XOR_inferred_i_4 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_12,
   I1 => OUTPUT_OBUF_12,
   O => ROUND_XOR_12
);
ROUND_XOR_inferred_i_5 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_11,
   I1 => OUTPUT_OBUF_11,
   O => ROUND_XOR_11
);
ROUND_XOR_inferred_i_6 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_10,
   I1 => OUTPUT_OBUF_10,
   O => ROUND_XOR_10
);
ROUND_XOR_inferred_i_7 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_9,
   I1 => OUTPUT_OBUF_9,
   O => ROUND_XOR_9
);
ROUND_XOR_inferred_i_8 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_8,
   I1 => OUTPUT_OBUF_8,
   O => ROUND_XOR_8
);
ROUND_XOR_inferred_i_9 : LUT2
  generic map(
   INIT => X"6"
  )
 port map (
   I0 => KEY_IBUF_7,
   I1 => OUTPUT_OBUF_7,
   O => ROUND_XOR_7
);
SBOX1_1SBOX1_sbox_LUT_inst_0 : LUT4
  generic map(
   INIT => X"81f6"
  )
 port map (
   I0 => ROUND_XOR_12,
   I1 => ROUND_XOR_13,
   I2 => ROUND_XOR_14,
   I3 => ROUND_XOR_15,
   O => ROUND_AFTER_SBOX_12
);
SBOX1_1SBOX1_sbox_LUT_inst_1 : LUT4
  generic map(
   INIT => X"0e7c"
  )
 port map (
   I0 => ROUND_XOR_12,
   I1 => ROUND_XOR_13,
   I2 => ROUND_XOR_14,
   I3 => ROUND_XOR_15,
   O => ROUND_AFTER_SBOX_13
);
SBOX1_1SBOX1_sbox_LUT_inst_2 : LUT4
  generic map(
   INIT => X"e568"
  )
 port map (
   I0 => ROUND_XOR_12,
   I1 => ROUND_XOR_13,
   I2 => ROUND_XOR_14,
   I3 => ROUND_XOR_15,
   O => ROUND_AFTER_SBOX_14
);
SBOX1_1SBOX1_sbox_LUT_inst_3 : LUT4
  generic map(
   INIT => X"4747"
  )
 port map (
   I0 => ROUND_XOR_12,
   I1 => ROUND_XOR_13,
   I2 => ROUND_XOR_14,
   I3 => ROUND_XOR_15,
   O => ROUND_AFTER_SBOX_15
);
SBOX2_1SBOX2_sbox_LUT_inst_0 : LUT4
  generic map(
   INIT => X"b352"
  )
 port map (
   I0 => ROUND_XOR_8,
   I1 => ROUND_XOR_9,
   I2 => ROUND_XOR_10,
   I3 => ROUND_XOR_11,
   O => ROUND_AFTER_SBOX_8
);
SBOX2_1SBOX2_sbox_LUT_inst_1 : LUT4
  generic map(
   INIT => X"89d9"
  )
 port map (
   I0 => ROUND_XOR_8,
   I1 => ROUND_XOR_9,
   I2 => ROUND_XOR_10,
   I3 => ROUND_XOR_11,
   O => ROUND_AFTER_SBOX_9
);
SBOX2_1SBOX2_sbox_LUT_inst_2 : LUT4
  generic map(
   INIT => X"e0ba"
  )
 port map (
   I0 => ROUND_XOR_8,
   I1 => ROUND_XOR_9,
   I2 => ROUND_XOR_10,
   I3 => ROUND_XOR_11,
   O => ROUND_AFTER_SBOX_10
);
SBOX2_1SBOX2_sbox_LUT_inst_3 : LUT4
  generic map(
   INIT => X"4e5a"
  )
 port map (
   I0 => ROUND_XOR_8,
   I1 => ROUND_XOR_9,
   I2 => ROUND_XOR_10,
   I3 => ROUND_XOR_11,
   O => ROUND_AFTER_SBOX_11
);
SBOX3_1SBOX3_sbox_LUT_inst_0 : LUT4
  generic map(
   INIT => X"875a"
  )
 port map (
   I0 => ROUND_XOR_4,
   I1 => ROUND_XOR_5,
   I2 => ROUND_XOR_6,
   I3 => ROUND_XOR_7,
   O => ROUND_AFTER_SBOX_4
);
SBOX3_1SBOX3_sbox_LUT_inst_1 : LUT4
  generic map(
   INIT => X"990f"
  )
 port map (
   I0 => ROUND_XOR_4,
   I1 => ROUND_XOR_5,
   I2 => ROUND_XOR_6,
   I3 => ROUND_XOR_7,
   O => ROUND_AFTER_SBOX_5
);
SBOX3_1SBOX3_sbox_LUT_inst_2 : LUT4
  generic map(
   INIT => X"fd10"
  )
 port map (
   I0 => ROUND_XOR_4,
   I1 => ROUND_XOR_5,
   I2 => ROUND_XOR_6,
   I3 => ROUND_XOR_7,
   O => ROUND_AFTER_SBOX_6
);
SBOX3_1SBOX3_sbox_LUT_inst_3 : LUT4
  generic map(
   INIT => X"aa33"
  )
 port map (
   I0 => ROUND_XOR_4,
   I1 => ROUND_XOR_5,
   I2 => ROUND_XOR_6,
   I3 => ROUND_XOR_7,
   O => ROUND_AFTER_SBOX_7
);
SBOX4_1SBOX4_sbox_LUT_inst_0 : LUT4
  generic map(
   INIT => X"3dc2"
  )
 port map (
   I0 => ROUND_XOR_0,
   I1 => ROUND_XOR_1,
   I2 => ROUND_XOR_2,
   I3 => ROUND_XOR_3,
   O => ROUND_AFTER_SBOX_0
);
SBOX4_1SBOX4_sbox_LUT_inst_1 : LUT4
  generic map(
   INIT => X"91f1"
  )
 port map (
   I0 => ROUND_XOR_0,
   I1 => ROUND_XOR_1,
   I2 => ROUND_XOR_2,
   I3 => ROUND_XOR_3,
   O => ROUND_AFTER_SBOX_1
);
SBOX4_1SBOX4_sbox_LUT_inst_2 : LUT4
  generic map(
   INIT => X"7874"
  )
 port map (
   I0 => ROUND_XOR_0,
   I1 => ROUND_XOR_1,
   I2 => ROUND_XOR_2,
   I3 => ROUND_XOR_3,
   O => ROUND_AFTER_SBOX_2
);
SBOX4_1SBOX4_sbox_LUT_inst_3 : LUT4
  generic map(
   INIT => X"48db"
  )
 port map (
   I0 => ROUND_XOR_0,
   I1 => ROUND_XOR_1,
   I2 => ROUND_XOR_2,
   I3 => ROUND_XOR_3,
   O => ROUND_AFTER_SBOX_3
);
START_IBUF_inst : IBUF
 port map (
   I => START,
   O => START_IBUF
);
end STRUCTURE;
