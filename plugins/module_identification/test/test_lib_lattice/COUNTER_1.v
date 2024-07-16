`timescale 1 ps/1 ps
module ibex_simple_system(\if_stage_i/N_1884 ,\pc_if(30) ,\u_ibex_core/N_1959 ,\u_ibex_core/N_1955 ,\pc_if(15) ,\u_ibex_core/N_1939 ,\fifo_i/GND ,\u_ibex_core/N_1935 ,\u_ibex_core/N_1941 ,\u_ibex_core/N_1960 ,\u_ibex_core/N_1962 ,\u_ibex_core/N_1950 ,\u_ibex_core/N_1961 ,\u_ibex_core/N_1956 ,\pc_if(26) ,\u_ibex_core/N_1936 ,N_1884_i,\u_ibex_core/N_1946 ,\u_ibex_core/N_1957 ,\pc_if(6) ,\pc_if(1) ,\u_ibex_core/N_1933 ,\pc_if(27) ,\u_ibex_core/N_1948 ,\u_ibex_core/N_1943 ,\pc_if(22) ,\u_ibex_core/N_1953 ,\u_ibex_core/N_1945 ,\u_ibex_core/N_1940 ,\u_ibex_core/N_1944 ,\pc_if(12) ,\u_ibex_core/N_1947 ,\u_ibex_core/N_1951 ,\pc_if(10) ,_05__779_RESYNTH,_05__764_RESYNTH,_05__765_RESYNTH,_05__782_RESYNTH,_05__778_RESYNTH,_05__771_RESYNTH,_05__759_RESYNTH,_05__762_RESYNTH,_05__783_RESYNTH,\instr_addr_next(1) ,_05__776_RESYNTH,_05__773_RESYNTH,_05__785_RESYNTH,_05__458_RESYNTH,_05__781_RESYNTH,_05__780_RESYNTH,_05__775_RESYNTH,_05__767_RESYNTH,_05__784_RESYNTH,_05__774_RESYNTH,_05__763_RESYNTH,_05__766_RESYNTH,_05__777_RESYNTH,_05__457_RESYNTH,_05__770_RESYNTH,_05__760_RESYNTH,_05__768_RESYNTH,_05__761_RESYNTH,_05__772_RESYNTH,_05__786_RESYNTH,_05__769_RESYNTH);
    input \if_stage_i/N_1884 ;
    input \pc_if(30) ;
    input \u_ibex_core/N_1959 ;
    input \u_ibex_core/N_1955 ;
    input \pc_if(15) ;
    input \u_ibex_core/N_1939 ;
    input \fifo_i/GND ;
    input \u_ibex_core/N_1935 ;
    input \u_ibex_core/N_1941 ;
    input \u_ibex_core/N_1960 ;
    input \u_ibex_core/N_1962 ;
    input \u_ibex_core/N_1950 ;
    input \u_ibex_core/N_1961 ;
    input \u_ibex_core/N_1956 ;
    input \pc_if(26) ;
    input \u_ibex_core/N_1936 ;
    input N_1884_i;
    input \u_ibex_core/N_1946 ;
    input \u_ibex_core/N_1957 ;
    input \pc_if(6) ;
    input \pc_if(1) ;
    input \u_ibex_core/N_1933 ;
    input \pc_if(27) ;
    input \u_ibex_core/N_1948 ;
    input \u_ibex_core/N_1943 ;
    input \pc_if(22) ;
    input \u_ibex_core/N_1953 ;
    input \u_ibex_core/N_1945 ;
    input \u_ibex_core/N_1940 ;
    input \u_ibex_core/N_1944 ;
    input \pc_if(12) ;
    input \u_ibex_core/N_1947 ;
    input \u_ibex_core/N_1951 ;
    input \pc_if(10) ;
    output _05__779_RESYNTH;
    output _05__764_RESYNTH;
    output _05__765_RESYNTH;
    output _05__782_RESYNTH;
    output _05__778_RESYNTH;
    output _05__771_RESYNTH;
    output _05__759_RESYNTH;
    output _05__762_RESYNTH;
    output _05__783_RESYNTH;
    output \instr_addr_next(1) ;
    output _05__776_RESYNTH;
    output _05__773_RESYNTH;
    output _05__785_RESYNTH;
    output _05__458_RESYNTH;
    output _05__781_RESYNTH;
    output _05__780_RESYNTH;
    output _05__775_RESYNTH;
    output _05__767_RESYNTH;
    output _05__784_RESYNTH;
    output _05__774_RESYNTH;
    output _05__763_RESYNTH;
    output _05__766_RESYNTH;
    output _05__777_RESYNTH;
    output _05__457_RESYNTH;
    output _05__770_RESYNTH;
    output _05__760_RESYNTH;
    output _05__768_RESYNTH;
    output _05__761_RESYNTH;
    output _05__772_RESYNTH;
    output _05__786_RESYNTH;
    output _05__769_RESYNTH;
    wire instr_addr_next_cry_15;
    wire instr_addr_next_cry_6;
    wire instr_addr_next_cry_4;
    wire instr_addr_next_cry_24;
    wire instr_addr_next_cry_7;
    wire instr_addr_next_cry_25;
    wire instr_addr_next_cry_12;
    wire instr_addr_next_cry_23;
    wire instr_addr_next_cry_29;
    wire instr_addr_next_cry_3;
    wire instr_addr_next_cry_9;
    wire instr_addr_next_cry_1;
    wire instr_addr_next_cry_5;
    wire instr_addr_next_cry_14;
    wire instr_addr_next_cry_2;
    wire instr_addr_next_cry_0;
    wire instr_addr_next_cry_8;
    wire instr_addr_next_cry_27;
    wire instr_addr_next_cry_13;
    wire instr_addr_next_cry_26;
    wire instr_addr_next_cry_21;
    wire instr_addr_next_cry_11;
    wire instr_addr_next_cry_22;
    wire instr_addr_next_cry_20;
    wire instr_addr_next_cry_19;
    wire instr_addr_next_cry_17;
    wire instr_addr_next_cry_18;
    wire instr_addr_next_cry_16;
    wire instr_addr_next_cry_10;
    wire instr_addr_next_cry_28;

    SB_CARRY instr_addr_next_cry_15_c (
        .CI(instr_addr_next_cry_14),
        .I0(\u_ibex_core/N_1948 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_15)
    );

    SB_CARRY instr_addr_next_cry_14_c (
        .CI(instr_addr_next_cry_13),
        .I0(\pc_if(15) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_14)
    );

    SB_CARRY instr_addr_next_cry_13_c (
        .CI(instr_addr_next_cry_12),
        .I0(\u_ibex_core/N_1950 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_13)
    );

    SB_CARRY instr_addr_next_cry_4_c (
        .CI(instr_addr_next_cry_3),
        .I0(\u_ibex_core/N_1959 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_4)
    );

    SB_CARRY instr_addr_next_cry_3_c (
        .CI(instr_addr_next_cry_2),
        .I0(\u_ibex_core/N_1960 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_3)
    );

    SB_CARRY instr_addr_next_cry_2_c (
        .CI(instr_addr_next_cry_1),
        .I0(\u_ibex_core/N_1961 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_2)
    );

    SB_CARRY instr_addr_next_cry_1_c (
        .CI(instr_addr_next_cry_0),
        .I0(N_1884_i),
        .I1(\u_ibex_core/N_1962 ),
        .CO(instr_addr_next_cry_1)
    );

    SB_CARRY instr_addr_next_cry_8_c (
        .CI(instr_addr_next_cry_7),
        .I0(\u_ibex_core/N_1955 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_8)
    );

    HAL_XOR2 _14__9211_RESYNTH (
        .A(\pc_if(27) ),
        .B(instr_addr_next_cry_25),
        .O(_05__783_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_7_c (
        .CI(instr_addr_next_cry_6),
        .I0(\u_ibex_core/N_1956 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_7)
    );

    SB_CARRY instr_addr_next_cry_6_c (
        .CI(instr_addr_next_cry_5),
        .I0(\u_ibex_core/N_1957 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_6)
    );

    HAL_XOR2 _14__9209_RESYNTH (
        .A(\pc_if(26) ),
        .B(instr_addr_next_cry_24),
        .O(_05__782_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_5_c (
        .CI(instr_addr_next_cry_4),
        .I0(\pc_if(6) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_5)
    );

    HAL_XOR2 _14__4219_RESYNTH (
        .A(\if_stage_i/N_1884 ),
        .B(\pc_if(1) ),
        .O(_05__457_RESYNTH)
    );

    HAL_XOR2 _14__9215_RESYNTH (
        .A(\u_ibex_core/N_1935 ),
        .B(instr_addr_next_cry_27),
        .O(_05__785_RESYNTH)
    );

    HAL_XOR2 _14__9213_RESYNTH (
        .A(\u_ibex_core/N_1936 ),
        .B(instr_addr_next_cry_26),
        .O(_05__784_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_0_c (
        .CI(\fifo_i/GND ),
        .I0(\if_stage_i/N_1884 ),
        .I1(\pc_if(1) ),
        .CO(instr_addr_next_cry_0)
    );

    HAL_XOR2 _14__9203_RESYNTH (
        .A(\u_ibex_core/N_1941 ),
        .B(instr_addr_next_cry_21),
        .O(_05__779_RESYNTH)
    );

    HAL_XOR2 _14__4221_RESYNTH (
        .A(\u_ibex_core/N_1933 ),
        .B(instr_addr_next_cry_29),
        .O(_05__458_RESYNTH)
    );

    HAL_XOR2 _14__9201_RESYNTH (
        .A(\pc_if(22) ),
        .B(instr_addr_next_cry_20),
        .O(_05__778_RESYNTH)
    );

    HAL_XOR2 _14__9207_RESYNTH (
        .A(\u_ibex_core/N_1939 ),
        .B(instr_addr_next_cry_23),
        .O(_05__781_RESYNTH)
    );

    HAL_XOR2 _14__9205_RESYNTH (
        .A(\u_ibex_core/N_1940 ),
        .B(instr_addr_next_cry_22),
        .O(_05__780_RESYNTH)
    );

    HAL_XOR2 _14__9195_RESYNTH (
        .A(\u_ibex_core/N_1945 ),
        .B(instr_addr_next_cry_17),
        .O(_05__775_RESYNTH)
    );

    HAL_XOR2 _14__9193_RESYNTH (
        .A(\u_ibex_core/N_1946 ),
        .B(instr_addr_next_cry_16),
        .O(_05__774_RESYNTH)
    );

    HAL_XOR2 _14__9199_RESYNTH (
        .A(\u_ibex_core/N_1943 ),
        .B(instr_addr_next_cry_19),
        .O(_05__777_RESYNTH)
    );

    HAL_XOR2 _14__9197_RESYNTH (
        .A(\u_ibex_core/N_1944 ),
        .B(instr_addr_next_cry_18),
        .O(_05__776_RESYNTH)
    );

    HAL_XOR2 _14__9187_RESYNTH (
        .A(\pc_if(15) ),
        .B(instr_addr_next_cry_13),
        .O(_05__771_RESYNTH)
    );

    HAL_XOR2 _14__9185_RESYNTH (
        .A(\u_ibex_core/N_1950 ),
        .B(instr_addr_next_cry_12),
        .O(_05__770_RESYNTH)
    );

    HAL_XOR2 _14__9191_RESYNTH (
        .A(\u_ibex_core/N_1947 ),
        .B(instr_addr_next_cry_15),
        .O(_05__773_RESYNTH)
    );

    HAL_XOR2 _14__9217_RESYNTH (
        .A(\pc_if(30) ),
        .B(instr_addr_next_cry_28),
        .O(_05__786_RESYNTH)
    );

    HAL_XOR2 _14__9189_RESYNTH (
        .A(\u_ibex_core/N_1948 ),
        .B(instr_addr_next_cry_14),
        .O(_05__772_RESYNTH)
    );

    HAL_XOR2 _14__9179_RESYNTH (
        .A(\u_ibex_core/N_1953 ),
        .B(instr_addr_next_cry_9),
        .O(_05__767_RESYNTH)
    );

    HAL_XOR2 _14__9177_RESYNTH (
        .A(\pc_if(10) ),
        .B(instr_addr_next_cry_8),
        .O(_05__766_RESYNTH)
    );

    HAL_XOR2 _14__9183_RESYNTH (
        .A(\u_ibex_core/N_1951 ),
        .B(instr_addr_next_cry_11),
        .O(_05__769_RESYNTH)
    );

    HAL_XOR2 _14__9181_RESYNTH (
        .A(\pc_if(12) ),
        .B(instr_addr_next_cry_10),
        .O(_05__768_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_28_c (
        .CI(instr_addr_next_cry_27),
        .I0(\u_ibex_core/N_1935 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_28)
    );

    SB_CARRY instr_addr_next_cry_27_c (
        .CI(instr_addr_next_cry_26),
        .I0(\u_ibex_core/N_1936 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_27)
    );

    HAL_XOR2 _14__9171_RESYNTH (
        .A(\u_ibex_core/N_1957 ),
        .B(instr_addr_next_cry_5),
        .O(_05__763_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_26_c (
        .CI(instr_addr_next_cry_25),
        .I0(\pc_if(27) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_26)
    );

    SB_CARRY instr_addr_next_cry_25_c (
        .CI(instr_addr_next_cry_24),
        .I0(\pc_if(26) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_25)
    );

    HAL_XOR2 _14__9169_RESYNTH (
        .A(\pc_if(6) ),
        .B(instr_addr_next_cry_4),
        .O(_05__762_RESYNTH)
    );

    HAL_XOR2 _14__9175_RESYNTH (
        .A(\u_ibex_core/N_1955 ),
        .B(instr_addr_next_cry_7),
        .O(_05__765_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_29_c (
        .CI(instr_addr_next_cry_28),
        .I0(\pc_if(30) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_29)
    );

    SB_CARRY instr_addr_next_cry_20_c (
        .CI(instr_addr_next_cry_19),
        .I0(\u_ibex_core/N_1943 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_20)
    );

    HAL_XOR2 _14__9173_RESYNTH (
        .A(\u_ibex_core/N_1956 ),
        .B(instr_addr_next_cry_6),
        .O(_05__764_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_19_c (
        .CI(instr_addr_next_cry_18),
        .I0(\u_ibex_core/N_1944 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_19)
    );

    HAL_XOR2 _14__9163_RESYNTH (
        .A(\u_ibex_core/N_1961 ),
        .B(instr_addr_next_cry_1),
        .O(_05__759_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_18_c (
        .CI(instr_addr_next_cry_17),
        .I0(\u_ibex_core/N_1945 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_18)
    );

    SB_CARRY instr_addr_next_cry_17_c (
        .CI(instr_addr_next_cry_16),
        .I0(\u_ibex_core/N_1946 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_17)
    );

    HAL_XOR3 _15__9162_RESYNTH (
        .A(\u_ibex_core/N_1962 ),
        .B(instr_addr_next_cry_0),
        .C(N_1884_i),
        .O(\instr_addr_next(1) )
    );

    SB_CARRY instr_addr_next_cry_24_c (
        .CI(instr_addr_next_cry_23),
        .I0(\u_ibex_core/N_1939 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_24)
    );

    SB_CARRY instr_addr_next_cry_23_c (
        .CI(instr_addr_next_cry_22),
        .I0(\u_ibex_core/N_1940 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_23)
    );

    HAL_XOR2 _14__9167_RESYNTH (
        .A(\u_ibex_core/N_1959 ),
        .B(instr_addr_next_cry_3),
        .O(_05__761_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_22_c (
        .CI(instr_addr_next_cry_21),
        .I0(\u_ibex_core/N_1941 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_22)
    );

    SB_CARRY instr_addr_next_cry_21_c (
        .CI(instr_addr_next_cry_20),
        .I0(\pc_if(22) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_21)
    );

    HAL_XOR2 _14__9165_RESYNTH (
        .A(\u_ibex_core/N_1960 ),
        .B(instr_addr_next_cry_2),
        .O(_05__760_RESYNTH)
    );

    SB_CARRY instr_addr_next_cry_12_c (
        .CI(instr_addr_next_cry_11),
        .I0(\u_ibex_core/N_1951 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_12)
    );

    SB_CARRY instr_addr_next_cry_11_c (
        .CI(instr_addr_next_cry_10),
        .I0(\pc_if(12) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_11)
    );

    SB_CARRY instr_addr_next_cry_10_c (
        .CI(instr_addr_next_cry_9),
        .I0(\u_ibex_core/N_1953 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_10)
    );

    SB_CARRY instr_addr_next_cry_9_c (
        .CI(instr_addr_next_cry_8),
        .I0(\pc_if(10) ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_9)
    );

    SB_CARRY instr_addr_next_cry_16_c (
        .CI(instr_addr_next_cry_15),
        .I0(\u_ibex_core/N_1947 ),
        .I1(\fifo_i/GND ),
        .CO(instr_addr_next_cry_16)
    );
endmodule

