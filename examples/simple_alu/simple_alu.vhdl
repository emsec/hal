-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2019.2 (win64) Build 2708876 Wed Nov  6 21:40:23 MST 2019
-- Date        : Fri May 29 11:06:56 2020
-- Host        : AnnikasLaptop running 64-bit major release  (build 9200)
-- Command     : write_vhdl C:/Users/Annika/Documents/02_Uni/8bizalu.vhd
-- Design      : four_bit_alu
-- Purpose     : This VHDL netlist is a functional simulation representation of the design and should not be modified or
--               synthesized. This netlist cannot be used for SDF annotated simulation.
-- Device      : xc7k70tfbv676-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity four_bit_alu is
  port (
    op : in STD_LOGIC_VECTOR ( 1 downto 0 );
    A : in STD_LOGIC_VECTOR ( 7 downto 0 );
    B : in STD_LOGIC_VECTOR ( 7 downto 0 );
    Z : out STD_LOGIC_VECTOR ( 7 downto 0 )
  );
  attribute NotValidForBitStream : boolean;
  attribute NotValidForBitStream of four_bit_alu : entity is true;
  attribute ADD_OP : string;
  attribute ADD_OP of four_bit_alu : entity is "2'b00";
  attribute AND_OP : string;
  attribute AND_OP of four_bit_alu : entity is "2'b10";
  attribute N : integer;
  attribute N of four_bit_alu : entity is 8;
  attribute SUB_OP : string;
  attribute SUB_OP of four_bit_alu : entity is "2'b11";
  attribute XOR_OP : string;
  attribute XOR_OP of four_bit_alu : entity is "2'b01";
end four_bit_alu;

architecture STRUCTURE of four_bit_alu is
  signal A_IBUF : STD_LOGIC_VECTOR ( 7 downto 0 );
  signal B_IBUF : STD_LOGIC_VECTOR ( 7 downto 0 );
  signal Z_OBUF : STD_LOGIC_VECTOR ( 7 downto 0 );
  signal \Z_OBUF[3]_inst_i_2_n_0\ : STD_LOGIC;
  signal \Z_OBUF[3]_inst_i_2_n_1\ : STD_LOGIC;
  signal \Z_OBUF[3]_inst_i_2_n_2\ : STD_LOGIC;
  signal \Z_OBUF[3]_inst_i_2_n_3\ : STD_LOGIC;
  signal \Z_OBUF[3]_inst_i_3_n_0\ : STD_LOGIC;
  signal \Z_OBUF[3]_inst_i_4_n_0\ : STD_LOGIC;
  signal \Z_OBUF[3]_inst_i_5_n_0\ : STD_LOGIC;
  signal \Z_OBUF[3]_inst_i_6_n_0\ : STD_LOGIC;
  signal \Z_OBUF[7]_inst_i_2_n_1\ : STD_LOGIC;
  signal \Z_OBUF[7]_inst_i_2_n_2\ : STD_LOGIC;
  signal \Z_OBUF[7]_inst_i_2_n_3\ : STD_LOGIC;
  signal \Z_OBUF[7]_inst_i_3_n_0\ : STD_LOGIC;
  signal \Z_OBUF[7]_inst_i_4_n_0\ : STD_LOGIC;
  signal \Z_OBUF[7]_inst_i_5_n_0\ : STD_LOGIC;
  signal \Z_OBUF[7]_inst_i_6_n_0\ : STD_LOGIC;
  signal op_IBUF : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal p_0_in : STD_LOGIC_VECTOR ( 7 downto 0 );
  signal \NLW_Z_OBUF[7]_inst_i_2_CO_UNCONNECTED\ : STD_LOGIC_VECTOR ( 3 to 3 );
  attribute METHODOLOGY_DRC_VIOS : string;
  attribute METHODOLOGY_DRC_VIOS of \Z_OBUF[3]_inst_i_2\ : label is "{SYNTH-8 {cell *THIS*}}";
  attribute METHODOLOGY_DRC_VIOS of \Z_OBUF[7]_inst_i_2\ : label is "{SYNTH-8 {cell *THIS*}}";
begin
\A_IBUF[0]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(0),
      O => A_IBUF(0)
    );
\A_IBUF[1]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(1),
      O => A_IBUF(1)
    );
\A_IBUF[2]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(2),
      O => A_IBUF(2)
    );
\A_IBUF[3]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(3),
      O => A_IBUF(3)
    );
\A_IBUF[4]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(4),
      O => A_IBUF(4)
    );
\A_IBUF[5]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(5),
      O => A_IBUF(5)
    );
\A_IBUF[6]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(6),
      O => A_IBUF(6)
    );
\A_IBUF[7]_inst\: unisim.vcomponents.IBUF
     port map (
      I => A(7),
      O => A_IBUF(7)
    );
\B_IBUF[0]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(0),
      O => B_IBUF(0)
    );
\B_IBUF[1]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(1),
      O => B_IBUF(1)
    );
\B_IBUF[2]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(2),
      O => B_IBUF(2)
    );
\B_IBUF[3]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(3),
      O => B_IBUF(3)
    );
\B_IBUF[4]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(4),
      O => B_IBUF(4)
    );
\B_IBUF[5]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(5),
      O => B_IBUF(5)
    );
\B_IBUF[6]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(6),
      O => B_IBUF(6)
    );
\B_IBUF[7]_inst\: unisim.vcomponents.IBUF
     port map (
      I => B(7),
      O => B_IBUF(7)
    );
\Z_OBUF[0]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(0),
      O => Z(0)
    );
\Z_OBUF[0]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(0),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(0),
      I4 => A_IBUF(0),
      O => Z_OBUF(0)
    );
\Z_OBUF[1]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(1),
      O => Z(1)
    );
\Z_OBUF[1]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(1),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(1),
      I4 => A_IBUF(1),
      O => Z_OBUF(1)
    );
\Z_OBUF[2]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(2),
      O => Z(2)
    );
\Z_OBUF[2]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(2),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(2),
      I4 => A_IBUF(2),
      O => Z_OBUF(2)
    );
\Z_OBUF[3]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(3),
      O => Z(3)
    );
\Z_OBUF[3]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(3),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(3),
      I4 => A_IBUF(3),
      O => Z_OBUF(3)
    );
\Z_OBUF[3]_inst_i_2\: unisim.vcomponents.CARRY4
     port map (
      CI => '0',
      CO(3) => \Z_OBUF[3]_inst_i_2_n_0\,
      CO(2) => \Z_OBUF[3]_inst_i_2_n_1\,
      CO(1) => \Z_OBUF[3]_inst_i_2_n_2\,
      CO(0) => \Z_OBUF[3]_inst_i_2_n_3\,
      CYINIT => A_IBUF(0),
      DI(3 downto 1) => A_IBUF(3 downto 1),
      DI(0) => op_IBUF(1),
      O(3 downto 0) => p_0_in(3 downto 0),
      S(3) => \Z_OBUF[3]_inst_i_3_n_0\,
      S(2) => \Z_OBUF[3]_inst_i_4_n_0\,
      S(1) => \Z_OBUF[3]_inst_i_5_n_0\,
      S(0) => \Z_OBUF[3]_inst_i_6_n_0\
    );
\Z_OBUF[3]_inst_i_3\: unisim.vcomponents.LUT3
    generic map(
      INIT => X"96"
    )
        port map (
      I0 => B_IBUF(3),
      I1 => op_IBUF(1),
      I2 => A_IBUF(3),
      O => \Z_OBUF[3]_inst_i_3_n_0\
    );
\Z_OBUF[3]_inst_i_4\: unisim.vcomponents.LUT3
    generic map(
      INIT => X"96"
    )
        port map (
      I0 => B_IBUF(2),
      I1 => op_IBUF(1),
      I2 => A_IBUF(2),
      O => \Z_OBUF[3]_inst_i_4_n_0\
    );
\Z_OBUF[3]_inst_i_5\: unisim.vcomponents.LUT3
    generic map(
      INIT => X"96"
    )
        port map (
      I0 => B_IBUF(1),
      I1 => op_IBUF(1),
      I2 => A_IBUF(1),
      O => \Z_OBUF[3]_inst_i_5_n_0\
    );
\Z_OBUF[3]_inst_i_6\: unisim.vcomponents.LUT1
    generic map(
      INIT => X"2"
    )
        port map (
      I0 => B_IBUF(0),
      O => \Z_OBUF[3]_inst_i_6_n_0\
    );
\Z_OBUF[4]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(4),
      O => Z(4)
    );
\Z_OBUF[4]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(4),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(4),
      I4 => A_IBUF(4),
      O => Z_OBUF(4)
    );
\Z_OBUF[5]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(5),
      O => Z(5)
    );
\Z_OBUF[5]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(5),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(5),
      I4 => A_IBUF(5),
      O => Z_OBUF(5)
    );
\Z_OBUF[6]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(6),
      O => Z(6)
    );
\Z_OBUF[6]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(6),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(6),
      I4 => A_IBUF(6),
      O => Z_OBUF(6)
    );
\Z_OBUF[7]_inst\: unisim.vcomponents.OBUF
     port map (
      I => Z_OBUF(7),
      O => Z(7)
    );
\Z_OBUF[7]_inst_i_1\: unisim.vcomponents.LUT5
    generic map(
      INIT => X"8EB2B282"
    )
        port map (
      I0 => p_0_in(7),
      I1 => op_IBUF(1),
      I2 => op_IBUF(0),
      I3 => B_IBUF(7),
      I4 => A_IBUF(7),
      O => Z_OBUF(7)
    );
\Z_OBUF[7]_inst_i_2\: unisim.vcomponents.CARRY4
     port map (
      CI => \Z_OBUF[3]_inst_i_2_n_0\,
      CO(3) => \NLW_Z_OBUF[7]_inst_i_2_CO_UNCONNECTED\(3),
      CO(2) => \Z_OBUF[7]_inst_i_2_n_1\,
      CO(1) => \Z_OBUF[7]_inst_i_2_n_2\,
      CO(0) => \Z_OBUF[7]_inst_i_2_n_3\,
      CYINIT => '0',
      DI(3) => '0',
      DI(2 downto 0) => A_IBUF(6 downto 4),
      O(3 downto 0) => p_0_in(7 downto 4),
      S(3) => \Z_OBUF[7]_inst_i_3_n_0\,
      S(2) => \Z_OBUF[7]_inst_i_4_n_0\,
      S(1) => \Z_OBUF[7]_inst_i_5_n_0\,
      S(0) => \Z_OBUF[7]_inst_i_6_n_0\
    );
\Z_OBUF[7]_inst_i_3\: unisim.vcomponents.LUT3
    generic map(
      INIT => X"96"
    )
        port map (
      I0 => B_IBUF(7),
      I1 => op_IBUF(1),
      I2 => A_IBUF(7),
      O => \Z_OBUF[7]_inst_i_3_n_0\
    );
\Z_OBUF[7]_inst_i_4\: unisim.vcomponents.LUT3
    generic map(
      INIT => X"96"
    )
        port map (
      I0 => B_IBUF(6),
      I1 => op_IBUF(1),
      I2 => A_IBUF(6),
      O => \Z_OBUF[7]_inst_i_4_n_0\
    );
\Z_OBUF[7]_inst_i_5\: unisim.vcomponents.LUT3
    generic map(
      INIT => X"96"
    )
        port map (
      I0 => B_IBUF(5),
      I1 => op_IBUF(1),
      I2 => A_IBUF(5),
      O => \Z_OBUF[7]_inst_i_5_n_0\
    );
\Z_OBUF[7]_inst_i_6\: unisim.vcomponents.LUT3
    generic map(
      INIT => X"96"
    )
        port map (
      I0 => B_IBUF(4),
      I1 => op_IBUF(1),
      I2 => A_IBUF(4),
      O => \Z_OBUF[7]_inst_i_6_n_0\
    );
\op_IBUF[0]_inst\: unisim.vcomponents.IBUF
     port map (
      I => op(0),
      O => op_IBUF(0)
    );
\op_IBUF[1]_inst\: unisim.vcomponents.IBUF
     port map (
      I => op(1),
      O => op_IBUF(1)
    );
end STRUCTURE;