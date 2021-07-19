`timescale 1ps/1ps

module bram_netlist(clk, raddr_4 ,waddr_4 ,din_11,raddr_6 ,raddr_3 ,raddr_5 ,raddr_1 ,waddr_3 ,din_4 ,din_5 ,din_9 ,din_10 ,raddr_0 ,din_8 ,din_6 ,din_7 ,waddr_5 ,waddr_6 ,rclke, din_3 ,waddr_7 ,write_en, din_14 ,din_15 ,din_12 ,din_13 ,raddr_2 ,wclke,waddr_1 ,din_0 ,waddr_2 ,din_1 ,din_2 ,raddr_7 ,read_en,waddr_0 ,dout_12 ,dout_6 ,dout_3 ,dout_1 ,dout_10 ,dout_0 ,dout_8 ,dout_11 ,dout_4 ,dout_5 ,dout_2 ,dout_9 ,dout_7 ,dout_13 ,dout_14 ,dout_15) ;

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

wire rclk;
wire wclk;

assign rclk = clk;
assign wclk = clk;



SB_RAM256x16 ram256x16_inst (
                 .WE(write_en),
                 .WCLK(wclk),
                 .WCLKE(wclke),
                 .RE(read_en),
                 .RCLKE(rclke),
                 .RCLK(rclk),
                 .MASK(),
                 .WADDR({waddr_7 , waddr_6 , waddr_5 , waddr_4 , waddr_3 , waddr_2 , waddr_1 , waddr_0 }),
                 .RADDR({raddr_7 ,raddr_6 ,raddr_5 ,raddr_4 ,raddr_3 ,raddr_2 ,raddr_1 ,raddr_0 }),
                 .WDATA({din_15 , din_14 , din_13 , din_12 , din_11 , din_10 , din_9 , din_8 , din_7 , din_6 , din_5 , din_4 , din_3 , din_2 , din_1 , din_0 }),
                 .RDATA({dout_15 , dout_14 , dout_13 , dout_12 , dout_11 , dout_10 , dout_9 , dout_8 , dout_7 , dout_6 , dout_5 , dout_4 , dout_3 , dout_2 , dout_1 , dout_0 })
             );
endmodule
