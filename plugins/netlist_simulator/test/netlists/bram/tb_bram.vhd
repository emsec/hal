

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use std.env.finish;


entity BRAM_TB is
--  Port ( );
end BRAM_TB;

ARCHITECTURE testbench OF BRAM_TB IS

	-- COMPONENTS -----------------------------------------------------------------
	COMPONENT bram_netlist is
      port (
        clk : in STD_LOGIC := 'X'; 
        rclke : in STD_LOGIC := 'X'; 
        wclke : in STD_LOGIC := 'X'; 
        write_en : in STD_LOGIC := 'X'; 
        read_en : in STD_LOGIC := 'X'; 
        raddr_0 : in STD_LOGIC := 'X'; 
        raddr_1 : in STD_LOGIC := 'X'; 
        raddr_2 : in STD_LOGIC := 'X'; 
        raddr_3 : in STD_LOGIC := 'X'; 
        raddr_4 : in STD_LOGIC := 'X'; 
        raddr_5 : in STD_LOGIC := 'X'; 
        raddr_6 : in STD_LOGIC := 'X'; 
        raddr_7 : in STD_LOGIC := 'X'; 
        waddr_0 : in STD_LOGIC := 'X'; 
        waddr_1 : in STD_LOGIC := 'X'; 
        waddr_2 : in STD_LOGIC := 'X'; 
        waddr_3 : in STD_LOGIC := 'X'; 
        waddr_4 : in STD_LOGIC := 'X'; 
        waddr_5 : in STD_LOGIC := 'X'; 
        waddr_6 : in STD_LOGIC := 'X'; 
        waddr_7 : in STD_LOGIC := 'X'; 
        din_0 : in STD_LOGIC := 'X'; 
        din_1 : in STD_LOGIC := 'X'; 
        din_2 : in STD_LOGIC := 'X'; 
        din_3 : in STD_LOGIC := 'X'; 
        din_4 : in STD_LOGIC := 'X'; 
        din_5 : in STD_LOGIC := 'X'; 
        din_6 : in STD_LOGIC := 'X'; 
        din_7 : in STD_LOGIC := 'X'; 
        din_8 : in STD_LOGIC := 'X'; 
        din_9 : in STD_LOGIC := 'X'; 
        din_10 : in STD_LOGIC := 'X'; 
        din_11 : in STD_LOGIC := 'X'; 
        din_12 : in STD_LOGIC := 'X'; 
        din_13 : in STD_LOGIC := 'X'; 
        din_14 : in STD_LOGIC := 'X'; 
        din_15 : in STD_LOGIC := 'X'; 
        mask_0 : in STD_LOGIC := 'X'; 
        mask_1 : in STD_LOGIC := 'X'; 
        mask_2 : in STD_LOGIC := 'X'; 
        mask_3 : in STD_LOGIC := 'X'; 
        mask_4 : in STD_LOGIC := 'X'; 
        mask_5 : in STD_LOGIC := 'X'; 
        mask_6 : in STD_LOGIC := 'X'; 
        mask_7 : in STD_LOGIC := 'X'; 
        mask_8 : in STD_LOGIC := 'X'; 
        mask_9 : in STD_LOGIC := 'X'; 
        mask_10 : in STD_LOGIC := 'X'; 
        mask_11 : in STD_LOGIC := 'X'; 
        mask_12 : in STD_LOGIC := 'X'; 
        mask_13 : in STD_LOGIC := 'X'; 
        mask_14 : in STD_LOGIC := 'X'; 
        mask_15 : in STD_LOGIC := 'X'; 
        dout_0 : out STD_LOGIC; 
        dout_1 : out STD_LOGIC; 
        dout_2 : out STD_LOGIC; 
        dout_3 : out STD_LOGIC; 
        dout_4 : out STD_LOGIC; 
        dout_5 : out STD_LOGIC; 
        dout_6 : out STD_LOGIC; 
        dout_7 : out STD_LOGIC; 
        dout_8 : out STD_LOGIC; 
        dout_9 : out STD_LOGIC;
        dout_10 : out STD_LOGIC; 
        dout_11 : out STD_LOGIC; 
        dout_12 : out STD_LOGIC; 
        dout_13 : out STD_LOGIC; 
        dout_14 : out STD_LOGIC; 
        dout_15 : out STD_LOGIC
    );
    end COMPONENT;
	-------------------------------------------------------------------------------


	-- SIGNALS ---------------------------------------------------------------------
    SIGNAL raddr            : STD_LOGIC_VECTOR (7 downto 0);
    SIGNAL waddr            : STD_LOGIC_VECTOR (7 downto 0);
    SIGNAL din              : STD_LOGIC_VECTOR (15 downto 0);
    SIGNAL mask             : STD_LOGIC_VECTOR (15 downto 0);
    SIGNAL dout             : STD_LOGIC_VECTOR (15 downto 0);

    SIGNAL clk              : STD_LOGIC := '0';
    SIGNAL wclke            : STD_LOGIC;
    SIGNAL rclke            : STD_LOGIC;
    SIGNAL read_en          : STD_LOGIC;
    SIGNAL write_en         : STD_LOGIC;



	-------------------------------------------------------------------------------


BEGIN

	-- UNIT UNDER TEST ------------------------------------------------------------
	UUT : bram_netlist
      port map (
        clk => clk,
        rclke => rclke,
        wclke => wclke,
        write_en => write_en,
        read_en => read_en,
        raddr_0 => raddr(0),
        raddr_1 => raddr(1),
        raddr_2 => raddr(2),
        raddr_3 => raddr(3),
        raddr_4 => raddr(4),
        raddr_5 => raddr(5),
        raddr_6 => raddr(6),
        raddr_7 => raddr(7),
        waddr_0 => waddr(0),
        waddr_1 => waddr(1),
        waddr_2 => waddr(2),
        waddr_3 => waddr(3),
        waddr_4 => waddr(4),
        waddr_5 => waddr(5),
        waddr_6 => waddr(6),
        waddr_7 => waddr(7),
        din_0 => din(0) ,
        din_1 => din(1) ,
        din_2 => din(2) ,
        din_3 => din(3) ,
        din_4 => din(4) ,
        din_5 => din(5) ,
        din_6 => din(6) ,
        din_7 => din(7) ,
        din_8 => din(8) ,
        din_9 => din(9) ,
        din_10 => din(10) ,
        din_11 => din(11) ,
        din_12 => din(12) ,
        din_13 => din(13) ,
        din_14 => din(14) ,
        din_15 => din(15) ,
        mask_0 => mask(0) ,
        mask_1 => mask(1) ,
        mask_2 => mask(2) ,
        mask_3 => mask(3) ,
        mask_4 => mask(4) ,
        mask_5 => mask(5) ,
        mask_6 => mask(6) ,
        mask_7 => mask(7) ,
        mask_8 => mask(8) ,
        mask_9 => mask(9) ,
        mask_10 => mask(10) ,
        mask_11 => mask(11) ,
        mask_12 => mask(12) ,
        mask_13 => mask(13) ,
        mask_14 => mask(14) ,
        mask_15 => mask(15) ,
        dout_0 => dout(0) ,
        dout_1 => dout(1) ,
        dout_2 => dout(2) ,
        dout_3 => dout(3) ,
        dout_4 => dout(4) ,
        dout_5 => dout(5) ,
        dout_6 => dout(6) ,
        dout_7 => dout(7) ,
        dout_8 => dout(8) ,
        dout_9 => dout(9),
        dout_10 => dout(10) ,
        dout_11 => dout(11) ,
        dout_12 => dout(12) ,
        dout_13 => dout(13) ,
        dout_14 => dout(14) ,
        dout_15 => dout(15)    
        );
	-------------------------------------------------------------------------------

	-- STIMULUS PROCESS -----------------------------------------------------------
	STIMULUS : PROCESS
	BEGIN
        raddr       <= "00000000";
        waddr       <= "00000000";
        din         <= "0000000000000000";
        mask        <= "0000000000000000";
        
        write_en    <= '0';
        read_en     <= '0';
        	
        wclke <= '0';
        rclke <= '0';
	
        WAIT FOR 10 NS;
        
--      // write data without wclke
        waddr       <= x"ff";
        din         <= x"ffff";
        WAIT FOR 10 NS;

        write_en    <= '1';
        WAIT FOR 10 NS;        
    
        write_en    <= '0';
        WAIT FOR 10 NS;        
    
--      read data without rclke
        read_en     <= '1';
        raddr       <= x"ff";
        WAIT FOR 20 NS;        

        WAIT FOR 50 NS;        

    
--      // write data with wclke
        waddr   <= x"ff";
        din     <= x"ffff";
        WAIT FOR 10 NS;


        write_en    <= '1';
        wclke       <= '1';
        WAIT FOR 10 NS;
        
        write_en    <= '0';
        wclke       <= '0';
        WAIT FOR 10 NS;
    
--      // read data without rclke
        read_en    <= '1';
        raddr      <= x"ff";
        WAIT FOR 20 NS;
        WAIT FOR 50 NS;

    
--      // read data with rclke
        read_en    <= '1';
        raddr      <= x"ff";
        rclke      <= '1';
        WAIT FOR 20 NS;        

        rclke      <= '0';
        WAIT FOR 50 NS;
        
    
--        // read some address and see what the result from INIT value is
        read_en    <= '1';
        raddr      <= x"66";
        rclke      <= '1';
        WAIT FOR 20 NS;      
        
        
        -- new stuff
        waddr       <= x"43";
        din         <= x"1111";
        WAIT FOR 10NS;
        
        write_en    <= '1';
        wclke       <= '1';
        WAIT FOR 20NS;
        
        read_en    <= '1';
        raddr      <= x"43";
        rclke      <= '1';
        WAIT FOR 20 NS;      
        
        
        din         <= x"ff11";
        WAIT FOR 200 NS;

        -- new stuff

        read_en     <= '0';
        rclke       <= '0';
        WAIT FOR 20 NS;

        mask        <= x"ffff";
        WAIT FOR 20 NS;

        write_en    <= '0';
        wclke       <= '0';
        read_en     <= '1';
        rclke       <= '1';
        WAIT FOR 20 NS;


        write_en    <= '1';
        wclke       <= '1';
        read_en     <= '0';
        rclke       <= '0';
        mask        <= x"1111";
        WAIT FOR 20 NS;






        WAIT FOR 100*10 NS;
        FINISH;


	END PROCESS;
	-------------------------------------------------------------------------------
    clk <= not clk after 5 NS;


END testbench;
