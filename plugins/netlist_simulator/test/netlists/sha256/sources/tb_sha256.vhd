----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 14.08.2020 09:22:00
-- Design Name: 
-- Module Name: tb_sha256 - Behavioral
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
use std.env.stop;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity tb_sha256 is
--  Port ( );
end tb_sha256;

--entity sha_256_core is
--    generic(
--        RESET_VALUE : std_logic := '0'    --reset enable value
--    );
--    port(
--        clk : in std_logic;
--        rst : in std_logic;
--        data_ready : in std_logic;  --the edge of this signal triggers the capturing of input data and hashing it.
--        n_blocks : in natural; --N, the number of (padded) message blocks
--        msg_block_in : in std_logic_vector(0 to (16 * WORD_SIZE)-1);
--        --mode_in : in std_logic;
--        finished : out std_logic;
--        data_out : out std_logic_vector((WORD_SIZE * 8)-1 downto 0) --SHA-256 results in a 256-bit hash value
--    );
--end entity;

architecture Behavioral of tb_sha256 is


	-- SIGNALS ---------------------------------------------------------------------
    signal msg : std_logic_vector(0 to 511);
    signal digest : std_logic_vector(255 downto 0);
	SIGNAL START                   : STD_LOGIC;
    SIGNAL RST                     : STD_LOGIC;
    SIGNAL DONE                    : STD_LOGIC;
    
    SIGNAL CLK                     : STD_LOGIC := '0';

	-------------------------------------------------------------------------------


BEGIN

	-- UNIT UNDER TEST ------------------------------------------------------------
	UUT : entity work.sha_256_core
        PORT MAP (
            CLK => CLK,
            rst => RST,
            data_ready => START,
            msg_block_in => msg,
            finished => done,
            data_out => digest
	);
	-------------------------------------------------------------------------------
	
	-- STIMULUS PROCESS -----------------------------------------------------------
	STIMULUS : PROCESS
	BEGIN
		
		msg <= x"61626380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000018";
		RST <= '1';
		START <= '0';
		
		WAIT FOR 10 NS;
		
        RST <= '0';
        
		WAIT FOR 10 NS;
		
        START <= '1';

		WAIT FOR 10 NS;

        START <= '0';

		WAIT UNTIL DONE = '1';
        
        WAIT FOR 10 NS;
        STOP;
	END PROCESS;
	-------------------------------------------------------------------------------
    CLK <= not CLK after 5 NS;

end Behavioral;
