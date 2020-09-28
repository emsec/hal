----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 27.03.2018 13:59:55
-- Design Name: 
-- Module Name: Half_Adder_TB - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Half_Adder_TB is
--  Port ( );
end Half_Adder_TB;

ARCHITECTURE testbench OF Half_Adder_TB IS

	-- COMPONENTS -----------------------------------------------------------------
	COMPONENT HalfAdder is
		PORT ( A : IN  STD_LOGIC;
				 B : IN  STD_LOGIC;
				 S : OUT STD_LOGIC;
				 C : OUT STD_LOGIC);
	END COMPONENT;
	-------------------------------------------------------------------------------


	-- INPUTS ---------------------------------------------------------------------
	SIGNAL A, B : STD_LOGIC := '0';
	-------------------------------------------------------------------------------

	-- OUTPUTS --------------------------------------------------------------------
	SIGNAL S, C : STD_LOGIC;
	-------------------------------------------------------------------------------

BEGIN

	-- UNIT UNDER TEST ------------------------------------------------------------
	UUT : HalfAdder
	PORT MAP (
		A => A,
		B => B,
		S => S,
		C => C
	);
	-------------------------------------------------------------------------------
	
	-- STIMULUS PROCESS -----------------------------------------------------------
	STIMULUS : PROCESS
	BEGIN
		A <= '0';
		B <= '0';
		WAIT FOR 10 NS;
		A <= '0';
		B <= '1';
		WAIT FOR 10 NS;
		A <= '1';
		B <= '0';
		WAIT FOR 10 NS;
		A <= '1';
		B <= '1';		
		WAIT;
	END PROCESS;
	-------------------------------------------------------------------------------

END testbench;

