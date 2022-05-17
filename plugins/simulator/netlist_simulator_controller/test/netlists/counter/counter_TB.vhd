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

entity Counter2_VHDL_TB is
--  Port ( );
end Counter2_VHDL_TB;

ARCHITECTURE testbench OF Counter2_VHDL_TB IS

	-- COMPONENTS -----------------------------------------------------------------
	COMPONENT Counter2_VHDL is
		PORT ( Clock_enable_B : IN  STD_LOGIC;
				 Clock : IN  STD_LOGIC;
				 Reset : IN STD_LOGIC;
				 Output : OUT STD_LOGIC_VECTOR(0 to 3));
	END COMPONENT;
	-------------------------------------------------------------------------------


	-- INPUTS ---------------------------------------------------------------------
	SIGNAL Clock_enable_B, Clock, Reset : STD_LOGIC := '0';
	-------------------------------------------------------------------------------

	-- OUTPUTS --------------------------------------------------------------------
	SIGNAL Output : STD_LOGIC_VECTOR(0 to 3);
	-------------------------------------------------------------------------------

BEGIN

	-- UNIT UNDER TEST ------------------------------------------------------------
	UUT : Counter2_VHDL
	PORT MAP (
		Clock_enable_B => Clock_enable_B,
		Clock => Clock,
		Reset => Reset,
		Output => Output
	);
	-------------------------------------------------------------------------------
	
	-- STIMULUS PROCESS -----------------------------------------------------------
	STIMULUS : PROCESS
	BEGIN
		--Clock <= '0';
        Clock_enable_B <= '1';
		Reset <= '0';
		WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        Clock_enable_B <= '0';
		WAIT FOR 10 NS;
		--Clock <= '1';
		WAIT FOR 10 NS;
		--Clock <= '0';
		WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        Reset <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        Reset <= '0';
        WAIT FOR 10 NS;
        --Clock <= '0';
		WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        Clock_enable_B <= '1';
        WAIT FOR 10 NS;
        --Clock <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
        WAIT FOR 3 NS;
        --Clock <= '1';
        Reset <= '1';
        WAIT FOR 10 NS;
        --Clock <= '0';
		WAIT;
	END PROCESS;
	-------------------------------------------------------------------------------
	Clock <= not Clock after 5 NS;

END testbench;

