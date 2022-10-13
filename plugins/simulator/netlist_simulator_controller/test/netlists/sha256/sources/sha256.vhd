--MIT License
--
--Copyright (c) 2017  Danny Savory
--
--Permission is hereby granted, free of charge, to any person obtaining a copy
--of this software and associated documentation files (the "Software"), to deal
--in the Software without restriction, including without limitation the rights
--to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
--copies of the Software, and to permit persons to whom the Software is
--furnished to do so, subject to the following conditions:
--
--The above copyright notice and this permission notice shall be included in all
--copies or substantial portions of the Software.
--
--THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
--IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
--FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
--AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
--LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
--OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
--SOFTWARE.


-- ############################################################################
--  The official specifications of the SHA-256 algorithm can be found here:
--      http://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf


-- ##################################################################
--     This SHA_256_CORE module reads in PADDED message blocks (from
--      an external source) and hashes the resulting message
-- ##################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.sha_256_pkg.all;

entity sha_256_core is
    generic(
        RESET_VALUE : std_logic := '1';    --reset enable value
        n_blocks : natural := 1
    );
    port(
        clk : in std_logic;
        rst : in std_logic;
        data_ready : in std_logic;  --the edge of this signal triggers the capturing of input data and hashing it.
         --N, the number of (padded) message blocks
        msg_block_in : in std_logic_vector(0 to (16 * WORD_SIZE)-1);
        --mode_in : in std_logic;
        finished : out std_logic;
        data_out : out std_logic_vector((WORD_SIZE * 8)-1 downto 0) --SHA-256 results in a 256-bit hash value
    );
end entity;

architecture sha_256_core_ARCH of sha_256_core is
    signal HASH_ROUND_COUNTER : natural := 0;
    signal MSG_BLOCK_COUNTER : natural := 0;
    signal HASH_02_COUNTER : natural := 0;
    constant HASH_02_COUNT_LIMIT : natural := 64;
    
    --Temporary words
    signal T1 : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal T2 : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');

    --Working variables, 8 32-bit words
    signal a : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal b : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal c : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal d : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal e : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal f : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal g : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    signal h : std_logic_vector(WORD_SIZE-1 downto 0) := (others => '0');
    
    constant K : K_DATA := (
        --address 0
        X"428a2f98", X"71374491", X"b5c0fbcf", X"e9b5dba5",
        X"3956c25b", X"59f111f1", X"923f82a4", X"ab1c5ed5",
        X"d807aa98", X"12835b01", X"243185be", X"550c7dc3",
        X"72be5d74", X"80deb1fe", X"9bdc06a7", X"c19bf174",
        X"e49b69c1", X"efbe4786", X"0fc19dc6", X"240ca1cc",
        X"2de92c6f", X"4a7484aa", X"5cb0a9dc", X"76f988da",
        X"983e5152", X"a831c66d", X"b00327c8", X"bf597fc7",
        X"c6e00bf3", X"d5a79147", X"06ca6351", X"14292967",
        X"27b70a85", X"2e1b2138", X"4d2c6dfc", X"53380d13",
        X"650a7354", X"766a0abb", X"81c2c92e", X"92722c85",
        X"a2bfe8a1", X"a81a664b", X"c24b8b70", X"c76c51a3",
        X"d192e819", X"d6990624", X"f40e3585", X"106aa070",
        X"19a4c116", X"1e376c08", X"2748774c", X"34b0bcb5",
        X"391c0cb3", X"4ed8aa4a", X"5b9cca4f", X"682e6ff3",
        X"748f82ee", X"78a5636f", X"84c87814", X"8cc70208",
        X"90befffa", X"a4506ceb", X"bef9a3f7", X"c67178f2"
    );
    
    --Message schedule, W(00), W(01), ...W(63) (64 32-bit words)
    signal W : K_DATA := (
        --address 0
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000",
        X"00000000", X"00000000", X"00000000", X"00000000"
    );
    
    --Message blocks, the padded message should be a multiple of 512 bits,
    signal M : M_DATA;
    --Hash values w/ initial hash values; 8 32-bit words
    signal HV : H_DATA;
    signal HV_INITIAL_VALUES : H_DATA := (X"6a09e667", X"bb67ae85", X"3c6ef372",
                                        X"a54ff53a", X"510e527f", X"9b05688c",
                                        X"1f83d9ab", X"5be0cd19");
    
    --intermediate Message block values; for use with a for-generate loop;
    signal M_INT : M_DATA;
    
    --intermediate Message Schedule values; for use with a for-generate loop;
    signal W_INT : K_DATA;
    
    
    type SHA_256_HASH_CORE_STATE is ( RESET, IDLE, READ_MSG_BLOCK, PREP_MSG_SCHEDULE_00, PREP_MSG_SCHEDULE_01, PREP_MSG_SCHEDULE_02, PREP_MSG_SCHEDULE_03, HASH_01, HASH_02, HASH_02b, HASH_02c, HASH_03, DONE );
    signal CURRENT_STATE, NEXT_STATE : SHA_256_HASH_CORE_STATE;
    signal PREVIOUS_STATE : SHA_256_HASH_CORE_STATE := READ_MSG_BLOCK;
begin


    --current state logic
    process(clk, rst)
    begin
        if(rst=RESET_VALUE) then
            CURRENT_STATE <= RESET;
        elsif(clk'event and clk='1') then
            CURRENT_STATE <= NEXT_STATE;
        end if;
    end process;
    
    
    --next state logic
    process(CURRENT_STATE, rst, HASH_ROUND_COUNTER, HASH_02_COUNTER, data_ready)
    begin
        case CURRENT_STATE is
            when RESET =>
                if(rst=RESET_VALUE) then
                    NEXT_STATE <= RESET;
                else
                    NEXT_STATE <= IDLE;
                end if;
            when IDLE =>
                if(data_ready='1') then
                    NEXT_STATE <= READ_MSG_BLOCK;
                else
                    NEXT_STATE <= IDLE;
                end if;
            when READ_MSG_BLOCK =>
                NEXT_STATE <= PREP_MSG_SCHEDULE_00;
            when PREP_MSG_SCHEDULE_00 =>
                NEXT_STATE <= PREP_MSG_SCHEDULE_01;
            when PREP_MSG_SCHEDULE_01 =>
                NEXT_STATE <= PREP_MSG_SCHEDULE_02;
            when PREP_MSG_SCHEDULE_02 =>
                NEXT_STATE <= PREP_MSG_SCHEDULE_03;
            when PREP_MSG_SCHEDULE_03 =>
                NEXT_STATE <= HASH_01;
            when HASH_01 =>
                NEXT_STATE <= HASH_02;
            when HASH_02 =>
                if(HASH_02_COUNTER = HASH_02_COUNT_LIMIT) then
                    NEXT_STATE <= HASH_03;
                else
                    NEXT_STATE <= HASH_02b;
                end if;
            when HASH_02b =>
                    NEXT_STATE <= HASH_02c;
            when HASH_02c =>
                    NEXT_STATE <= HASH_02;
            when HASH_03 =>
                if(HASH_ROUND_COUNTER = n_blocks-1) then
                    NEXT_STATE <= DONE;
                else
                    NEXT_STATE <= IDLE;
                end if;
            when DONE =>
                NEXT_STATE <= DONE; --stay in done state unless reset
        end case;
    end process;
    
    
    --hash logic
    process(clk, rst, CURRENT_STATE)
    begin
        if(rst=RESET_VALUE) then
            HASH_ROUND_COUNTER <= 0;
            MSG_BLOCK_COUNTER <= 0;
        elsif(clk'event and clk='1') then
            a <= a;     b <= b;     c <= c;     d <= d;
            e <= e;     f <= f;     g <= g;     h <= h;
            T1 <= T1;   T2 <= T2;
            W <= W;     M <= M;     HV <= HV;
            HASH_02_COUNTER <= HASH_02_COUNTER;
            HASH_ROUND_COUNTER <= HASH_ROUND_COUNTER;
            case CURRENT_STATE is
                when RESET =>
                    HV <= HV_INITIAL_VALUES;
                    HASH_02_COUNTER <= 0;
                    HASH_ROUND_COUNTER <= 0;
                when IDLE =>    --the IDLE stage is a stall stage, perhaps waiting for new message block to arrive.
                when READ_MSG_BLOCK =>
                    if(HASH_ROUND_COUNTER = 0) then
                        HV <= HV_INITIAL_VALUES;
                    end if;
                    M <= M_INT;
                when PREP_MSG_SCHEDULE_00 =>
                    W(0 to 15) <= W_INT(0 to 15);
                when PREP_MSG_SCHEDULE_01 =>
                    W(16 to 31) <= W_INT(16 to 31);
                when PREP_MSG_SCHEDULE_02 =>
                    W(32 to 47) <= W_INT(32 to 47);
                when PREP_MSG_SCHEDULE_03 =>
                    W(48 to 63) <= W_INT(48 to 63);
                when HASH_01 =>
                    a <= HV(0);
                    b <= HV(1);
                    c <= HV(2);
                    d <= HV(3);
                    e <= HV(4);
                    f <= HV(5);
                    g <= HV(6);
                    h <= HV(7);
                when HASH_02 =>
                    if(HASH_02_COUNTER = HASH_02_COUNT_LIMIT) then
                        HASH_02_COUNTER <= 0;
                    else
                        --you have to set T1 and T2 in a different state, due to how
                        --VHDL sequential/process statements are evaluated.
                        T1 <= std_logic_vector(unsigned(h) + unsigned(SIGMA_UCASE_1(e)) + unsigned(CH(e, f, g)) + unsigned(K(HASH_02_COUNTER)) + unsigned(W(HASH_02_COUNTER)));
                        T2 <= std_logic_vector(unsigned(SIGMA_UCASE_0(a)) + unsigned(MAJ(a, b, c)));
                    end if;
                when HASH_02b =>
                    h <= g;
                    g <= f;
                    f <= e;
                    e <= std_logic_vector(unsigned(d) + unsigned(T1));
                    d <= c;
                    c <= b;
                    b <= a;
                    a <= std_logic_vector(unsigned(T1) + unsigned(T2));
                when HASH_02c =>
                    HASH_02_COUNTER <= HASH_02_COUNTER + 1;    --increment counter
                when HASH_03 =>
                    HV(0) <= std_logic_vector(unsigned(a) + unsigned(HV(0)));
                    HV(1) <= std_logic_vector(unsigned(b) + unsigned(HV(1)));
                    HV(2) <= std_logic_vector(unsigned(c) + unsigned(HV(2)));
                    HV(3) <= std_logic_vector(unsigned(d) + unsigned(HV(3)));
                    HV(4) <= std_logic_vector(unsigned(e) + unsigned(HV(4)));
                    HV(5) <= std_logic_vector(unsigned(f) + unsigned(HV(5)));
                    HV(6) <= std_logic_vector(unsigned(g) + unsigned(HV(6)));
                    HV(7) <= std_logic_vector(unsigned(h) + unsigned(HV(7)));
                    if(HASH_ROUND_COUNTER = n_blocks-1) then
                        HASH_ROUND_COUNTER <= 0;
                    else
                        HASH_ROUND_COUNTER <= HASH_ROUND_COUNTER + 1;    --increment counter, read in next message block
                    end if;
                when DONE =>
            end case;
        end if;
    end process;
    
    
    MESSAGE_BLOCK_INTERMEDIATE :
    for i in 0 to 15 generate
    begin
        --M_INT(i) <= msg_block_in((WORD_SIZE * (i+1))-1 downto WORD_SIZE * i);
        M_INT(i) <= msg_block_in((WORD_SIZE * i) to WORD_SIZE * (i+1)-1);
    end generate;
    
    MESSAGE_SCHEDULE_INTERMEDIATE_00:
    for i in 0 to 15 generate
    begin
        W_INT(i) <= M(i);
    end generate;
    
    MESSAGE_SCHEDULE_INTERMEDIATE_01:
    for i in 16 to 63 generate
    begin
        W_INT(i) <= std_logic_vector(unsigned(SIGMA_LCASE_1(W_INT(i-2))) + unsigned(W_INT(i-7)) + unsigned(SIGMA_LCASE_0(W_INT(i-15))) + unsigned(W_INT(i-16)));
    end generate;
    
    --FINISHED signal asserts when hashing is done
    finished <= '1' when CURRENT_STATE = DONE else
                '0';
                
    data_out <= HV(0) & HV(1) & HV(2) & HV(3) & HV(4) & HV(5) & HV(6) & HV(7);
end architecture;

