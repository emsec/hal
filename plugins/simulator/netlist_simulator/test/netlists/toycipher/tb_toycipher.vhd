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

entity ToyCipher_TB is
--  Port ( );
end ToyCipher_TB;

ARCHITECTURE testbench OF ToyCipher_TB IS

	-- COMPONENTS -----------------------------------------------------------------
	COMPONENT CIPHER is
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
    end COMPONENT;
	-------------------------------------------------------------------------------


	-- SIGNALS ---------------------------------------------------------------------
    SIGNAL PLAINTEXT_0            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_1            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_2            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_3            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_4            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_5            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_6            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_7            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_8            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_9            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_10            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_11            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_12            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_13            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_14            : STD_LOGIC := '0';
    SIGNAL PLAINTEXT_15            : STD_LOGIC := '0';

    SIGNAL KEY_0            : STD_LOGIC := '0';
    SIGNAL KEY_1            : STD_LOGIC := '0';
    SIGNAL KEY_2            : STD_LOGIC := '0';
    SIGNAL KEY_3            : STD_LOGIC := '0';
    SIGNAL KEY_4            : STD_LOGIC := '0';
    SIGNAL KEY_5            : STD_LOGIC := '0';
    SIGNAL KEY_6            : STD_LOGIC := '0';
    SIGNAL KEY_7            : STD_LOGIC := '0';
    SIGNAL KEY_8            : STD_LOGIC := '0';
    SIGNAL KEY_9            : STD_LOGIC := '0';
    SIGNAL KEY_10            : STD_LOGIC := '0';
    SIGNAL KEY_11            : STD_LOGIC := '0';
    SIGNAL KEY_12            : STD_LOGIC := '0';
    SIGNAL KEY_13            : STD_LOGIC := '0';
    SIGNAL KEY_14            : STD_LOGIC := '0';
    SIGNAL KEY_15            : STD_LOGIC := '0';

    SIGNAL OUTPUT_0            : STD_LOGIC;
    SIGNAL OUTPUT_1            : STD_LOGIC;
    SIGNAL OUTPUT_2            : STD_LOGIC;
    SIGNAL OUTPUT_3            : STD_LOGIC;
    SIGNAL OUTPUT_4            : STD_LOGIC;
    SIGNAL OUTPUT_5            : STD_LOGIC;
    SIGNAL OUTPUT_6            : STD_LOGIC;
    SIGNAL OUTPUT_7            : STD_LOGIC;
    SIGNAL OUTPUT_8            : STD_LOGIC;
    SIGNAL OUTPUT_9            : STD_LOGIC;
    SIGNAL OUTPUT_10            : STD_LOGIC;
    SIGNAL OUTPUT_11            : STD_LOGIC;
    SIGNAL OUTPUT_12            : STD_LOGIC;
    SIGNAL OUTPUT_13            : STD_LOGIC;
    SIGNAL OUTPUT_14            : STD_LOGIC;
    SIGNAL OUTPUT_15            : STD_LOGIC;


	SIGNAL START                   : STD_LOGIC := '0';
    SIGNAL DONE                    : STD_LOGIC := '0';
    SIGNAL CLK                     : STD_LOGIC := '0';

	-------------------------------------------------------------------------------


BEGIN

	-- UNIT UNDER TEST ------------------------------------------------------------
	UUT : CIPHER
        PORT MAP (
            CLK => CLK,
            KEY_0 => KEY_0,
            KEY_1 => KEY_1,
            KEY_10 => KEY_10,
            KEY_11 => KEY_11,
            KEY_12 => KEY_12,
            KEY_13 => KEY_13,
            KEY_14 => KEY_14,
            KEY_15 => KEY_15,
            KEY_2 => KEY_2,
            KEY_3 => KEY_3,
            KEY_4 => KEY_4,
            KEY_5 => KEY_5,
            KEY_6 => KEY_6,
            KEY_7 => KEY_7,
            KEY_8 => KEY_8,
            KEY_9 => KEY_9,
            PLAINTEXT_0 => PLAINTEXT_0,
            PLAINTEXT_1 => PLAINTEXT_1,
            PLAINTEXT_10 => PLAINTEXT_10,
            PLAINTEXT_11 => PLAINTEXT_11,
            PLAINTEXT_12 => PLAINTEXT_12,
            PLAINTEXT_13 => PLAINTEXT_13,
            PLAINTEXT_14 => PLAINTEXT_14,
            PLAINTEXT_15 => PLAINTEXT_15,
            PLAINTEXT_2 => PLAINTEXT_2,
            PLAINTEXT_3 => PLAINTEXT_3,
            PLAINTEXT_4 => PLAINTEXT_4,
            PLAINTEXT_5 => PLAINTEXT_5,
            PLAINTEXT_6 => PLAINTEXT_6,
            PLAINTEXT_7 => PLAINTEXT_7,
            PLAINTEXT_8 => PLAINTEXT_8,
            PLAINTEXT_9 => PLAINTEXT_9,
            START => START,
            DONE => DONE, 
            OUTPUT_0 => OUTPUT_0, 
            OUTPUT_1 => OUTPUT_1, 
            OUTPUT_10 => OUTPUT_10, 
            OUTPUT_11 => OUTPUT_11, 
            OUTPUT_12 => OUTPUT_12, 
            OUTPUT_13 => OUTPUT_13, 
            OUTPUT_14 => OUTPUT_14, 
            OUTPUT_15 => OUTPUT_15, 
            OUTPUT_2 => OUTPUT_2, 
            OUTPUT_3 => OUTPUT_3, 
            OUTPUT_4 => OUTPUT_4, 
            OUTPUT_5 => OUTPUT_5, 
            OUTPUT_6 => OUTPUT_6, 
            OUTPUT_7 => OUTPUT_7, 
            OUTPUT_8 => OUTPUT_8, 
            OUTPUT_9 => OUTPUT_9
	);
	-------------------------------------------------------------------------------
	
	-- STIMULUS PROCESS -----------------------------------------------------------
	STIMULUS : PROCESS
	BEGIN
		PLAINTEXT_0 <= '0';
		PLAINTEXT_1 <= '0';
		PLAINTEXT_2 <= '0';
		PLAINTEXT_3 <= '0';
		PLAINTEXT_4 <= '0';
		PLAINTEXT_5 <= '0';
		PLAINTEXT_6 <= '0';
		PLAINTEXT_7 <= '0';
		PLAINTEXT_8 <= '0';
		PLAINTEXT_9 <= '0';
		PLAINTEXT_10 <= '0';
		PLAINTEXT_11 <= '0';
		PLAINTEXT_12 <= '0';
		PLAINTEXT_13 <= '0';
		PLAINTEXT_14 <= '0';
		PLAINTEXT_15 <= '0';

        KEY_0 <= '0';
		KEY_1 <= '0';
		KEY_2 <= '0';
		KEY_3 <= '0';
		KEY_4 <= '0';
		KEY_5 <= '0';
		KEY_6 <= '0';
		KEY_7 <= '0';
		KEY_8 <= '0';
		KEY_9 <= '0';
		KEY_10 <= '0';
		KEY_11 <= '0';
		KEY_12 <= '0';
		KEY_13 <= '0';
		KEY_14 <= '0';
		KEY_15 <= '0';

		START <= '0';

		WAIT FOR 10 NS;


        START <= '1';

		WAIT FOR 10 NS;


        START <= '0';

		WAIT FOR 100 NS;


        PLAINTEXT_0 <= '1';
		PLAINTEXT_1 <= '1';
		PLAINTEXT_2 <= '1';
		PLAINTEXT_3 <= '1';
		PLAINTEXT_4 <= '1';
		PLAINTEXT_5 <= '1';
		PLAINTEXT_6 <= '1';
		PLAINTEXT_7 <= '1';
		PLAINTEXT_8 <= '1';
		PLAINTEXT_9 <= '1';
		PLAINTEXT_10 <= '1';
		PLAINTEXT_11 <= '1';
		PLAINTEXT_12 <= '1';
		PLAINTEXT_13 <= '1';
		PLAINTEXT_14 <= '1';
		PLAINTEXT_15 <= '1';

        KEY_0 <= '1';
		KEY_1 <= '1';
		KEY_2 <= '1';
		KEY_3 <= '1';
		KEY_4 <= '1';
		KEY_5 <= '1';
		KEY_6 <= '1';
		KEY_7 <= '1';
		KEY_8 <= '1';
		KEY_9 <= '1';
		KEY_10 <= '1';
		KEY_11 <= '1';
		KEY_12 <= '1';
		KEY_13 <= '1';
		KEY_14 <= '1';
		KEY_15 <= '1';
        START <= '0';

        WAIT FOR 10 NS;
        
        START <= '1';
        
        
        WAIT FOR 10 NS;
        START <= '0';
		
        
        WAIT FOR 100 NS;
	END PROCESS;
	-------------------------------------------------------------------------------
    CLK <= not CLK after 5 NS;


END testbench;
