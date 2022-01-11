Library UNISIM;
use UNISIM.vcomponents.all;
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity Counter2_VHDL is
  port (
Clock : in STD_LOGIC := 'X'; 
  Clock_enable_B : in STD_LOGIC := 'X'; 
  Reset : in STD_LOGIC := 'X'; 
  Output_0 : out STD_LOGIC; 
  Output_1 : out STD_LOGIC; 
  Output_2 : out STD_LOGIC; 
  Output_3 : out STD_LOGIC
);
end Counter2_VHDL;

architecture STRUCTURE of Counter2_VHDL is
  signal Clock_IBUF : STD_LOGIC;
  signal Clock_IBUF_BUFG : STD_LOGIC;
  signal Clock_enable_B_IBUF : STD_LOGIC;
  signal Output_OBUF_0 : STD_LOGIC;
  signal Output_OBUF_1 : STD_LOGIC;
  signal Output_OBUF_2 : STD_LOGIC;
  signal Output_OBUF_3 : STD_LOGIC;
  signal Reset_IBUF : STD_LOGIC;
  signal sel : STD_LOGIC;
  signal temp_0_i_2_n_0 : STD_LOGIC;
  signal temp_1_i_1_n_0 : STD_LOGIC;
  signal temp_2_i_1_n_0 : STD_LOGIC;
  signal temp_3_i_1_n_0 : STD_LOGIC;
begin
temp_2_i_1_inst : LUT4
  generic map(
   INIT => X"5a52"
  )
 port map (
   I0 => Output_OBUF_3,
   I1 => Output_OBUF_0,
   I2 => Output_OBUF_2,
   I3 => Output_OBUF_1,
   O => temp_2_i_1_n_0
);
temp_reg_1_inst : FDCE
  generic map(
   INIT => '0'
  )
 port map (
   C => Clock_IBUF_BUFG,
   CE => sel,
   CLR => Reset_IBUF,
   D => temp_1_i_1_n_0,
   Q => Output_OBUF_1
);
temp_reg_2_inst : FDCE
  generic map(
   INIT => '0'
  )
 port map (
   C => Clock_IBUF_BUFG,
   CE => sel,
   CLR => Reset_IBUF,
   D => temp_2_i_1_n_0,
   Q => Output_OBUF_2
);
Clock_enable_B_IBUF_inst_inst : IBUF
 port map (
   I => Clock_enable_B,
   O => Clock_enable_B_IBUF
);
Clock_IBUF_BUFG_inst_inst : BUFG
 port map (
   I => Clock_IBUF,
   O => Clock_IBUF_BUFG
);
Clock_IBUF_inst_inst : IBUF
 port map (
   I => Clock,
   O => Clock_IBUF
);
temp_reg_0_inst : FDCE
  generic map(
   INIT => '0'
  )
 port map (
   C => Clock_IBUF_BUFG,
   CE => sel,
   CLR => Reset_IBUF,
   D => temp_0_i_2_n_0,
   Q => Output_OBUF_0
);
Output_OBUF_3_inst_inst : OBUF
 port map (
   I => Output_OBUF_3,
   O => Output_3
);
temp_3_i_1_inst : LUT1
  generic map(
   INIT => X"1"
  )
 port map (
   I0 => Output_OBUF_3,
   O => temp_3_i_1_n_0
);
temp_0_i_1_inst : LUT1
  generic map(
   INIT => X"1"
  )
 port map (
   I0 => Clock_enable_B_IBUF,
   O => sel
);
temp_1_i_1_inst : LUT3
  generic map(
   INIT => X"78"
  )
 port map (
   I0 => Output_OBUF_3,
   I1 => Output_OBUF_2,
   I2 => Output_OBUF_1,
   O => temp_1_i_1_n_0
);
temp_reg_3_inst : FDCE
  generic map(
   INIT => '0'
  )
 port map (
   C => Clock_IBUF_BUFG,
   CE => sel,
   CLR => Reset_IBUF,
   D => temp_3_i_1_n_0,
   Q => Output_OBUF_3
);
Reset_IBUF_inst_inst : IBUF
 port map (
   I => Reset,
   O => Reset_IBUF
);
temp_0_i_2_inst : LUT4
  generic map(
   INIT => X"6cc4"
  )
 port map (
   I0 => Output_OBUF_3,
   I1 => Output_OBUF_0,
   I2 => Output_OBUF_2,
   I3 => Output_OBUF_1,
   O => temp_0_i_2_n_0
);
Output_OBUF_1_inst_inst : OBUF
 port map (
   I => Output_OBUF_1,
   O => Output_1
);
Output_OBUF_0_inst_inst : OBUF
 port map (
   I => Output_OBUF_0,
   O => Output_0
);
Output_OBUF_2_inst_inst : OBUF
 port map (
   I => Output_OBUF_2,
   O => Output_2
);
end STRUCTURE;
