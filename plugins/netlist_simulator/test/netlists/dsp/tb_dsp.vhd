

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE std.env.finish;
USE ieee.numeric_std.ALL;
ENTITY DSP_TB IS
    --  Port ( );
END DSP_TB;

ARCHITECTURE testbench OF DSP_TB IS

    -- COMPONENTS -----------------------------------------------------------------
    COMPONENT dsp_netlist IS
        PORT (
            CLK : IN STD_LOGIC := 'X';
            CE : IN STD_LOGIC := 'X';
            IRSTTOP : IN STD_LOGIC := 'X';
            IRSTBOT : IN STD_LOGIC := 'X';
            ORSTTOP : IN STD_LOGIC := 'X';
            ORSTBOT : IN STD_LOGIC := 'X';
            AHOLD : IN STD_LOGIC := 'X';
            BHOLD : IN STD_LOGIC := 'X';
            CHOLD : IN STD_LOGIC := 'X';
            DHOLD : IN STD_LOGIC := 'X';
            OHOLDTOP : IN STD_LOGIC := 'X';
            OHOLDBOT : IN STD_LOGIC := 'X';
            OLOADTOP : IN STD_LOGIC := 'X';
            OLOADBOT : IN STD_LOGIC := 'X';
            ADDSUBTOP : IN STD_LOGIC := 'X';
            ADDSUBBOT : IN STD_LOGIC := 'X';
            CO : OUT STD_LOGIC;
            CI : IN STD_LOGIC := 'X';
            A_0 : IN STD_LOGIC := 'X';
            A_1 : IN STD_LOGIC := 'X';
            A_2 : IN STD_LOGIC := 'X';
            A_3 : IN STD_LOGIC := 'X';
            A_4 : IN STD_LOGIC := 'X';
            A_5 : IN STD_LOGIC := 'X';
            A_6 : IN STD_LOGIC := 'X';
            A_7 : IN STD_LOGIC := 'X';
            A_8 : IN STD_LOGIC := 'X';
            A_9 : IN STD_LOGIC := 'X';
            A_10 : IN STD_LOGIC := 'X';
            A_11 : IN STD_LOGIC := 'X';
            A_12 : IN STD_LOGIC := 'X';
            A_13 : IN STD_LOGIC := 'X';
            A_14 : IN STD_LOGIC := 'X';
            A_15 : IN STD_LOGIC := 'X';
            B_0 : IN STD_LOGIC := 'X';
            B_1 : IN STD_LOGIC := 'X';
            B_2 : IN STD_LOGIC := 'X';
            B_3 : IN STD_LOGIC := 'X';
            B_4 : IN STD_LOGIC := 'X';
            B_5 : IN STD_LOGIC := 'X';
            B_6 : IN STD_LOGIC := 'X';
            B_7 : IN STD_LOGIC := 'X';
            B_8 : IN STD_LOGIC := 'X';
            B_9 : IN STD_LOGIC := 'X';
            B_10 : IN STD_LOGIC := 'X';
            B_11 : IN STD_LOGIC := 'X';
            B_12 : IN STD_LOGIC := 'X';
            B_13 : IN STD_LOGIC := 'X';
            B_14 : IN STD_LOGIC := 'X';
            B_15 : IN STD_LOGIC := 'X';
            C_0 : IN STD_LOGIC := 'X';
            C_1 : IN STD_LOGIC := 'X';
            C_2 : IN STD_LOGIC := 'X';
            C_3 : IN STD_LOGIC := 'X';
            C_4 : IN STD_LOGIC := 'X';
            C_5 : IN STD_LOGIC := 'X';
            C_6 : IN STD_LOGIC := 'X';
            C_7 : IN STD_LOGIC := 'X';
            C_8 : IN STD_LOGIC := 'X';
            C_9 : IN STD_LOGIC := 'X';
            C_10 : IN STD_LOGIC := 'X';
            C_11 : IN STD_LOGIC := 'X';
            C_12 : IN STD_LOGIC := 'X';
            C_13 : IN STD_LOGIC := 'X';
            C_14 : IN STD_LOGIC := 'X';
            C_15 : IN STD_LOGIC := 'X';
            D_0 : IN STD_LOGIC := 'X';
            D_1 : IN STD_LOGIC := 'X';
            D_2 : IN STD_LOGIC := 'X';
            D_3 : IN STD_LOGIC := 'X';
            D_4 : IN STD_LOGIC := 'X';
            D_5 : IN STD_LOGIC := 'X';
            D_6 : IN STD_LOGIC := 'X';
            D_7 : IN STD_LOGIC := 'X';
            D_8 : IN STD_LOGIC := 'X';
            D_9 : IN STD_LOGIC := 'X';
            D_10 : IN STD_LOGIC := 'X';
            D_11 : IN STD_LOGIC := 'X';
            D_12 : IN STD_LOGIC := 'X';
            D_13 : IN STD_LOGIC := 'X';
            D_14 : IN STD_LOGIC := 'X';
            D_15 : IN STD_LOGIC := 'X';
            O_0 : OUT STD_LOGIC;
            O_1 : OUT STD_LOGIC;
            O_2 : OUT STD_LOGIC;
            O_3 : OUT STD_LOGIC;
            O_4 : OUT STD_LOGIC;
            O_5 : OUT STD_LOGIC;
            O_6 : OUT STD_LOGIC;
            O_7 : OUT STD_LOGIC;
            O_8 : OUT STD_LOGIC;
            O_9 : OUT STD_LOGIC;
            O_10 : OUT STD_LOGIC;
            O_11 : OUT STD_LOGIC;
            O_12 : OUT STD_LOGIC;
            O_13 : OUT STD_LOGIC;
            O_14 : OUT STD_LOGIC;
            O_15 : OUT STD_LOGIC;
            O_16 : OUT STD_LOGIC;
            O_17 : OUT STD_LOGIC;
            O_18 : OUT STD_LOGIC;
            O_19 : OUT STD_LOGIC;
            O_20 : OUT STD_LOGIC;
            O_21 : OUT STD_LOGIC;
            O_22 : OUT STD_LOGIC;
            O_23 : OUT STD_LOGIC;
            O_24 : OUT STD_LOGIC;
            O_25 : OUT STD_LOGIC;
            O_26 : OUT STD_LOGIC;
            O_27 : OUT STD_LOGIC;
            O_28 : OUT STD_LOGIC;
            O_29 : OUT STD_LOGIC;
            O_30 : OUT STD_LOGIC;
            O_31 : OUT STD_LOGIC
        );
    END COMPONENT;
    -------------------------------------------------------------------------------
    -- SIGNALS ---------------------------------------------------------------------
    SIGNAL A : STD_LOGIC_VECTOR (15 DOWNTO 0) := (OTHERS => '0');
    SIGNAL B : STD_LOGIC_VECTOR (15 DOWNTO 0) := (OTHERS => '0');
    SIGNAL C : STD_LOGIC_VECTOR (15 DOWNTO 0) := (OTHERS => '0');
    SIGNAL D : STD_LOGIC_VECTOR (15 DOWNTO 0) := (OTHERS => '0');
    SIGNAL O : STD_LOGIC_VECTOR (31 DOWNTO 0) := (OTHERS => '0');

    SIGNAL CLK : STD_LOGIC := '0';
    SIGNAL CE : STD_LOGIC := '0';
    SIGNAL IRSTTOP : STD_LOGIC := '0';
    SIGNAL IRSTBOT : STD_LOGIC := '0';
    SIGNAL ORSTTOP : STD_LOGIC := '0';
    SIGNAL ORSTBOT : STD_LOGIC := '0';
    SIGNAL AHOLD : STD_LOGIC := '0';
    SIGNAL BHOLD : STD_LOGIC := '0';
    SIGNAL CHOLD : STD_LOGIC := '0';
    SIGNAL DHOLD : STD_LOGIC := '0';
    SIGNAL OHOLDTOP : STD_LOGIC := '0';
    SIGNAL OHOLDBOT : STD_LOGIC := '0';
    SIGNAL OLOADTOP : STD_LOGIC := '0';
    SIGNAL OLOADBOT : STD_LOGIC := '0';
    SIGNAL ADDSUBTOP : STD_LOGIC := '0';
    SIGNAL ADDSUBBOT : STD_LOGIC := '0';
    SIGNAL CO : STD_LOGIC := '0';
    SIGNAL CI : STD_LOGIC := '0';

    -------------------------------------------------------------------------------
BEGIN

    -- UNIT UNDER TEST ------------------------------------------------------------
    UUT : dsp_netlist
    PORT MAP(
        CLK => CLK,
        CE => CE,
        IRSTTOP => IRSTTOP,
        IRSTBOT => IRSTBOT,
        ORSTTOP => ORSTTOP,
        ORSTBOT => ORSTBOT,
        AHOLD => AHOLD,
        BHOLD => BHOLD,
        CHOLD => CHOLD,
        DHOLD => DHOLD,
        OHOLDTOP => OHOLDTOP,
        OHOLDBOT => OHOLDBOT,
        OLOADTOP => OLOADTOP,
        OLOADBOT => OLOADBOT,
        ADDSUBTOP => ADDSUBTOP,
        ADDSUBBOT => ADDSUBBOT,
        CO => CO,
        CI => CI,
        A_0 => A(0),
        A_1 => A(1),
        A_2 => A(2),
        A_3 => A(3),
        A_4 => A(4),
        A_5 => A(5),
        A_6 => A(6),
        A_7 => A(7),
        A_8 => A(8),
        A_9 => A(9),
        A_10 => A(10),
        A_11 => A(11),
        A_12 => A(12),
        A_13 => A(13),
        A_14 => A(14),
        A_15 => A(15),
        B_0 => B(0),
        B_1 => B(1),
        B_2 => B(2),
        B_3 => B(3),
        B_4 => B(4),
        B_5 => B(5),
        B_6 => B(6),
        B_7 => B(7),
        B_8 => B(8),
        B_9 => B(9),
        B_10 => B(10),
        B_11 => B(11),
        B_12 => B(12),
        B_13 => B(13),
        B_14 => B(14),
        B_15 => B(15),
        C_0 => C(0),
        C_1 => C(1),
        C_2 => C(2),
        C_3 => C(3),
        C_4 => C(4),
        C_5 => C(5),
        C_6 => C(6),
        C_7 => C(7),
        C_8 => C(8),
        C_9 => C(9),
        C_10 => C(10),
        C_11 => C(11),
        C_12 => C(12),
        C_13 => C(13),
        C_14 => C(14),
        C_15 => C(15),
        D_0 => D(0),
        D_1 => D(1),
        D_2 => D(2),
        D_3 => D(3),
        D_4 => D(4),
        D_5 => D(5),
        D_6 => D(6),
        D_7 => D(7),
        D_8 => D(8),
        D_9 => D(9),
        D_10 => D(10),
        D_11 => D(11),
        D_12 => D(12),
        D_13 => D(13),
        D_14 => D(14),
        D_15 => D(15),
        O_0 => O(0),
        O_1 => O(1),
        O_2 => O(2),
        O_3 => O(3),
        O_4 => O(4),
        O_5 => O(5),
        O_6 => O(6),
        O_7 => O(7),
        O_8 => O(8),
        O_9 => O(9),
        O_10 => O(10),
        O_11 => O(11),
        O_12 => O(12),
        O_13 => O(13),
        O_14 => O(14),
        O_15 => O(15),
        O_16 => O(16),
        O_17 => O(17),
        O_18 => O(18),
        O_19 => O(19),
        O_20 => O(20),
        O_21 => O(21),
        O_22 => O(22),
        O_23 => O(23),
        O_24 => O(24),
        O_25 => O(25),
        O_26 => O(26),
        O_27 => O(27),
        O_28 => O(28),
        O_29 => O(29),
        O_30 => O(30),
        O_31 => O(31)
    );
    -------------------------------------------------------------------------------

    -- STIMULUS PROCESS -----------------------------------------------------------
    STIMULUS : PROCESS
    BEGIN
        A <= std_logic_vector(to_unsigned(6666, 16));
        B <= std_logic_vector(to_unsigned(1234, 16));
        C <= std_logic_vector(to_unsigned(4371, 16));
        D <= std_logic_vector(to_unsigned(43714, 16));
        AHOLD <= '0';
        BHOLD <= '0';
        CHOLD <= '0';
        DHOLD <= '0';
        OHOLDTOP <= '0';
        OHOLDBOT <= '0';
        OLOADTOP <= '0';
        OLOADBOT <= '0';
        ADDSUBTOP <= '0';
        ADDSUBBOT <= '0';
        CI <= '0';
        CE <= '0';

        WAIT FOR 10 NS;
        CE <= '0';

        IRSTTOP <= '1';
        IRSTBOT <= '1';
        ORSTTOP <= '1';
        ORSTBOT <= '1';
        WAIT FOR 10 * 10 NS;

        CE <= '1';
        WAIT FOR 10 * 10 NS;

        IRSTTOP <= '0';
        IRSTBOT <= '0';
        ORSTTOP <= '0';
        ORSTBOT <= '0';
        WAIT FOR 10 * 10 NS;

        AHOLD <= '1';
        WAIT FOR 5 * 10 NS;
        OHOLDTOP <= '1';
        OHOLDBOT <= '1';
        WAIT FOR 5 * 10 NS;

        AHOLD <= '0';
        OHOLDTOP <= '0';
        OHOLDBOT <= '0';
        WAIT FOR 5 * 10 NS;

        OLOADTOP <= '1';
        ADDSUBTOP <= '1';
        WAIT FOR 5 * 10 NS;
        OLOADTOP <= '0';
        ADDSUBTOP <= '0';
        ADDSUBBOT <= '1';
        WAIT FOR 5 * 10 NS;
        ORSTTOP <= '1';
        WAIT FOR 5 * 10 NS;

        ADDSUBBOT <= '0';
        ORSTTOP <= '0';
        WAIT FOR 5 * 10 NS;

        IRSTTOP <= '1';
        CHOLD <= '1';
        WAIT FOR 5 * 10 NS;

        IRSTTOP <= '0';
        CHOLD <= '0';

        A <= std_logic_vector(to_unsigned(1632, 16));
        B <= std_logic_vector(to_unsigned(764254, 16));
        C <= std_logic_vector(to_unsigned(12523, 16));
        D <= std_logic_vector(to_unsigned(263, 16));
        WAIT FOR 5 * 10 NS;

        OLOADTOP <= '1';
        OLOADBOT <= '1';
        WAIT FOR 5 * 10 NS;

        OLOADTOP <= '0';
        OLOADBOT <= '0';
        WAIT FOR 5 * 10 NS;

        ADDSUBTOP <= '1';
        ADDSUBBOT <= '0';
        WAIT FOR 5 * 10 NS;

        ADDSUBTOP <= '0';
        ADDSUBBOT <= '1';
        WAIT FOR 5 * 10 NS;

        ADDSUBTOP <= '0';
        ADDSUBBOT <= '0';
        WAIT FOR 5 * 10 NS;

        WAIT FOR 5 * 10 NS;
        FINISH;
    END PROCESS;
    -------------------------------------------------------------------------------
    CLK <= NOT CLK AFTER 5 NS;
END testbench;