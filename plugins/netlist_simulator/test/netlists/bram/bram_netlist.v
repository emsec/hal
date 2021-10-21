`timescale 1ps/1ps

module bram_netlist(clk, raddr_4 ,waddr_4 ,din_11,raddr_6 ,raddr_3 ,raddr_5 ,raddr_1 ,waddr_3 ,din_4 ,din_5 ,din_9 ,din_10 ,raddr_0 ,din_8 ,din_6 ,din_7 ,waddr_5 ,waddr_6 ,rclke, din_3 ,waddr_7 ,write_en, din_14 ,din_15 ,din_12 ,din_13 ,raddr_2 ,wclke,waddr_1 ,din_0 ,waddr_2 ,din_1 ,din_2 ,raddr_7 ,read_en,waddr_0 , mask_15 , mask_14 , mask_13 , mask_12 , mask_11 , mask_10 , mask_9 , mask_8 , mask_7 , mask_6 , mask_5 , mask_4 , mask_3 , mask_2 , mask_1 , mask_0 , dout_12 ,dout_6 ,dout_3 ,dout_1 ,dout_10 ,dout_0 ,dout_8 ,dout_11 ,dout_4 ,dout_5 ,dout_2 ,dout_9 ,dout_7 ,dout_13 ,dout_14 ,dout_15) ;

input clk;
input raddr_4 ;
input waddr_4 ;
input din_11 ;
input raddr_6 ;
input raddr_3 ;
input raddr_5 ;
input raddr_1 ;
input waddr_3 ;
input din_4 ;
input din_5 ;
input din_9 ;
input din_10 ;
input raddr_0 ;
input din_8 ;
input din_6 ;
input din_7 ;
input waddr_5 ;
input waddr_6 ;
input rclke;
input din_3 ;
input waddr_7 ;
input write_en;
input din_14 ;
input din_15 ;
input din_12 ;
input din_13 ;
input raddr_2 ;
input wclke;
input waddr_1 ;
input din_0 ;
input waddr_2 ;
input din_1 ;
input din_2 ;
input raddr_7 ;
input read_en;
input waddr_0 ;
input mask_0 ;
input mask_1 ;
input mask_2 ;
input mask_3 ;
input mask_4 ;
input mask_5 ;
input mask_6 ;
input mask_7 ;
input mask_8 ;
input mask_9 ;
input mask_10 ;
input mask_11 ;
input mask_12 ;
input mask_13 ;
input mask_14 ;
input mask_15 ;
output dout_12 ;
output dout_6 ;
output dout_3 ;
output dout_1 ;
output dout_10 ;
output dout_0 ;
output dout_8 ;
output dout_11 ;
output dout_4 ;
output dout_5 ;
output dout_2 ;
output dout_9 ;
output dout_7 ;
output dout_13 ;
output dout_14 ;
output dout_15 ;




SB_RAM256x16 #(
    .INIT_0(256'h0000000000000000000000000000000000000000000000000000000000000000),
    .INIT_1(256'h0003000200020002000200010001000100010001000100010001000100010000),
    .INIT_2(256'h001600130011000f000d000b000a000900080007000600050005000400030003),
    .INIT_3(256'h008e00810075006a005f0055004c0044003c0035002f002900240020001c0019),
    .INIT_4(256'h01c701ae0195017e01670150013b0126011200ff00ec00db00ca00ba00aa009c),
    .INIT_5(256'h03b20390036d034b032a030902e902c902aa028b026d02500233021701fc01e1),
    .INIT_6(256'h05f705d405b1058d05690544051f04fa04d504b0048b04660442041d03f903d6),
    .INIT_7(256'h078f077f076e075b07470732071b070306ea06d006b406970679065a063a0619),
    .INIT_8(256'h07c307cb07d107d607d907db07dc07db07d807d407cf07c807bf07b507aa079d),
    .INIT_9(256'h067f069e06bb06d806f2070b0723073a074f076207750785079407a207af07b9),
    .INIT_A(256'h03e404170448047904a804d60503052f0559058205aa05d105f7061b063e065f),
    .INIT_B(256'h003e007d00bc00fb0139017601b301ef022a0265029e02d7030f0346037b03b0),
    .INIT_C(256'hfc62fc9bfcd4fd0ffd4afd86fdc3fe01fe3ffe7efebefefeff3eff7effbefffe),
    .INIT_D(256'hf984f9a8f9cdf9f4fa1cfa45fa70fa9cfac9faf8fb28fb5afb8cfbc0fbf5fc2b),
    .INIT_E(256'hf811f81df82af839f849f85bf86ff884f89af8b2f8ccf8e7f903f921f941f962),
    .INIT_F(256'hf820f814f80af800f7f9f7f2f7eef7eaf7e8f7e8f7e9f7ecf7f0f7f6f7fef806)
) ram256x16_inst (
    .WE(write_en),
    .WCLK(clk),
    .WCLKE(wclke),
    .RE(read_en),
    .RCLKE(rclke),
    .RCLK(clk),
    .MASK({mask_15 , mask_14 , mask_13 , mask_12 , mask_11 , mask_10 , mask_9 , mask_8 , mask_7 , mask_6 , mask_5 , mask_4 , mask_3 , mask_2 , mask_1 , mask_0 }),
    .WADDR({waddr_7 , waddr_6 , waddr_5 , waddr_4 , waddr_3 , waddr_2 , waddr_1 , waddr_0 }),
    .RADDR({raddr_7 ,raddr_6 ,raddr_5 ,raddr_4 ,raddr_3 ,raddr_2 ,raddr_1 ,raddr_0 }),
    .WDATA({din_15 , din_14 , din_13 , din_12 , din_11 , din_10 , din_9 , din_8 , din_7 , din_6 , din_5 , din_4 , din_3 , din_2 , din_1 , din_0 }),
    .RDATA({dout_15 , dout_14 , dout_13 , dout_12 , dout_11 , dout_10 , dout_9 , dout_8 , dout_7 , dout_6 , dout_5 , dout_4 , dout_3 , dout_2 , dout_1 , dout_0 })
    );
endmodule
