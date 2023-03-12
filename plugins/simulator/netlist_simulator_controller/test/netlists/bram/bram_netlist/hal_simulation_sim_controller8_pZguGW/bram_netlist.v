`timescale 1 ps/1 ps
module bram_netlist(waddr_4,mask_0,din_4,mask_7,mask_10,mask_11,mask_13,mask_15,mask_6,mask_12,read_en,raddr_2,mask_4,raddr_7,mask_5,din_2,din_1,mask_2,waddr_2,din_0,din_7,write_en,mask_14,din_9,raddr_6,rclke,din_6,din_3,clk,waddr_7,waddr_6,din_12,mask_3,din_8,raddr_0,din_13,wclke,mask_1,din_11,raddr_4,mask_8,waddr_3,din_15,din_10,din_5,waddr_5,din_14,raddr_5,raddr_3,waddr_0,mask_9,raddr_1,waddr_1,dout_0,dout_1,dout_14,dout_7,dout_5,dout_9,dout_4,dout_8,dout_12,dout_10,dout_15,dout_3,dout_13,dout_6,dout_2,dout_11);
    input waddr_4;
    input mask_0;
    input din_4;
    input mask_7;
    input mask_10;
    input mask_11;
    input mask_13;
    input mask_15;
    input mask_6;
    input mask_12;
    input read_en;
    input raddr_2;
    input mask_4;
    input raddr_7;
    input mask_5;
    input din_2;
    input din_1;
    input mask_2;
    input waddr_2;
    input din_0;
    input din_7;
    input write_en;
    input mask_14;
    input din_9;
    input raddr_6;
    input rclke;
    input din_6;
    input din_3;
    input clk;
    input waddr_7;
    input waddr_6;
    input din_12;
    input mask_3;
    input din_8;
    input raddr_0;
    input din_13;
    input wclke;
    input mask_1;
    input din_11;
    input raddr_4;
    input mask_8;
    input waddr_3;
    input din_15;
    input din_10;
    input din_5;
    input waddr_5;
    input din_14;
    input raddr_5;
    input raddr_3;
    input waddr_0;
    input mask_9;
    input raddr_1;
    input waddr_1;
    output dout_0;
    output dout_1;
    output dout_14;
    output dout_7;
    output dout_5;
    output dout_9;
    output dout_4;
    output dout_8;
    output dout_12;
    output dout_10;
    output dout_15;
    output dout_3;
    output dout_13;
    output dout_6;
    output dout_2;
    output dout_11;

    SB_RAM256x16 #(
        .INIT_0(256'h0000000000000000000000000000000000000000000000000000000000000000),
        .INIT_1(256'h0003000200020002000200010001000100010001000100010001000100010000),
        .INIT_2(256'h001600130011000F000D000B000A000900080007000600050005000400030003),
        .INIT_3(256'h008E00810075006A005F0055004C0044003C0035002F002900240020001C0019),
        .INIT_4(256'h01C701AE0195017E01670150013B0126011200FF00EC00DB00CA00BA00AA009C),
        .INIT_5(256'h03B20390036D034B032A030902E902C902AA028B026D02500233021701FC01E1),
        .INIT_6(256'h05F705D405B1058D05690544051F04FA04D504B0048B04660442041D03F903D6),
        .INIT_7(256'h078F077F076E075B07470732071B070306EA06D006B406970679065A063A0619),
        .INIT_8(256'h07C307CB07D107D607D907DB07DC07DB07D807D407CF07C807BF07B507AA079D),
        .INIT_9(256'h067F069E06BB06D806F2070B0723073A074F076207750785079407A207AF07B9),
        .INIT_A(256'h03E404170448047904A804D60503052F0559058205AA05D105F7061B063E065F),
        .INIT_B(256'h003E007D00BC00FB0139017601B301EF022A0265029E02D7030F0346037B03B0),
        .INIT_C(256'hFC62FC9BFCD4FD0FFD4AFD86FDC3FE01FE3FFE7EFEBEFEFEFF3EFF7EFFBEFFFE),
        .INIT_D(256'hF984F9A8F9CDF9F4FA1CFA45FA70FA9CFAC9FAF8FB28FB5AFB8CFBC0FBF5FC2B),
        .INIT_E(256'hF811F81DF82AF839F849F85BF86FF884F89AF8B2F8CCF8E7F903F921F941F962),
        .INIT_F(256'hF820F814F80AF800F7F9F7F2F7EEF7EAF7E8F7E8F7E9F7ECF7F0F7F6F7FEF806)
    ) ram256x16_inst (
        .WE(write_en),
        .WCLK(clk),
        .WCLKE(wclke),
        .RE(read_en),
        .RCLKE(rclke),
        .RCLK(clk),
        .WADDR({waddr_7,
waddr_6,
waddr_5,
waddr_4,
waddr_3,
waddr_2,
waddr_1,
waddr_0}),
        .RADDR({raddr_7,
raddr_6,
raddr_5,
raddr_4,
raddr_3,
raddr_2,
raddr_1,
raddr_0}),
        .MASK({mask_15,
mask_14,
mask_13,
mask_12,
mask_11,
mask_10,
mask_9,
mask_8,
mask_7,
mask_6,
mask_5,
mask_4,
mask_3,
mask_2,
mask_1,
mask_0}),
        .WDATA({din_15,
din_14,
din_13,
din_12,
din_11,
din_10,
din_9,
din_8,
din_7,
din_6,
din_5,
din_4,
din_3,
din_2,
din_1,
din_0}),
        .RDATA({dout_15,
dout_14,
dout_13,
dout_12,
dout_11,
dout_10,
dout_9,
dout_8,
dout_7,
dout_6,
dout_5,
dout_4,
dout_3,
dout_2,
dout_1,
dout_0})
    );
endmodule

