`timescale 1 ps/1 ps

module counter (Reset,Clock_enable_B,Clock,Output_0,Output_3,Output_2,Output_1);
input Reset;
input Clock_enable_B;
input Clock;
output Output_0;
output Output_3;
output Output_2;
output Output_1;
wire temp_2_i_1_n_0;
wire temp_0_i_2_n_0;
wire Clock_enable_B_IBUF;
wire Reset_IBUF;
wire temp_1_i_1_n_0;
wire Output_OBUF_2;
wire Output_OBUF_1;
wire Output_OBUF_3;
wire Output_OBUF_0;
wire temp_3_i_1_n_0;
wire Clock_enable_B_IBUF;
wire Clock_IBUF_BUFG;
wire Clock_IBUF;


LUT4 #(
         .INIT(15'h5A52)
     ) temp_2_i_1_inst (
         .I0(Output_OBUF_3),
         .I1(Output_OBUF_0),
         .I2(Output_OBUF_2),
         .I3(Output_OBUF_1),
         .O(temp_2_i_1_n_0)
     );

FDCE #(
         .INIT(1'b0)
     ) temp_reg_1_inst (
         .C(Clock_IBUF_BUFG),
         .CE(Clock_enable_B_IBUF),
         .CLR(Reset_IBUF),
         .D(temp_1_i_1_n_0),
         .Q(Output_OBUF_1)
     );

FDCE #(
         .INIT(1'b0)
     ) temp_reg_2_inst (
         .C(Clock_IBUF_BUFG),
         .CE(Clock_enable_B_IBUF),
         .CLR(Reset_IBUF),
         .D(temp_2_i_1_n_0),
         .Q(Output_OBUF_2)
     );

IBUF Clock_enable_B_IBUF_inst_inst (
         .I(Clock_enable_B),
         .O(Clock_enable_B_IBUF)
     );

BUFG Clock_IBUF_BUFG_inst_inst (
         .I(Clock_IBUF),
         .O(Clock_IBUF_BUFG)
     );

IBUF Clock_IBUF_inst_inst (
         .I(Clock),
         .O(Clock_IBUF)
     );

INV temp_3_i_1_inst (
        .I(Output_OBUF_3),
        .O(temp_3_i_1_n_0)
    );

FDCE #(
         .INIT(1'b0)
     ) temp_reg_0_inst (
         .C(Clock_IBUF_BUFG),
         .CE(Clock_enable_B_IBUF),
         .CLR(Reset_IBUF),
         .D(temp_0_i_2_n_0),
         .Q(Output_OBUF_0)
     );

OBUF Output_OBUF_3_inst_inst (
         .I(Output_OBUF_3),
         .O(Output_3)
     );


LUT4 #(
         .INIT(16'h7878)
     ) temp_1_i_1_inst (
         .I0(Output_OBUF_3),
         .I1(Output_OBUF_2),
         .I2(Output_OBUF_1),
         .I3(),
         .O(temp_1_i_1_n_0)
     );

FDCE #(
         .INIT(1'b0)
     ) temp_reg_3_inst (
         .C(Clock_IBUF_BUFG),
         .CE(Clock_enable_B_IBUF),
         .CLR(Reset_IBUF),
         .D(temp_3_i_1_n_0),
         .Q(Output_OBUF_3)
     );

IBUF Reset_IBUF_inst_inst (
         .I(Reset),
         .O(Reset_IBUF)
     );

LUT4 #(
         .INIT(15'h6CC4)
     ) temp_0_i_2_inst (
         .I0(Output_OBUF_3),
         .I1(Output_OBUF_0),
         .I2(Output_OBUF_2),
         .I3(Output_OBUF_1),
         .O(temp_0_i_2_n_0)
     );

OBUF Output_OBUF_1_inst_inst (
         .I(Output_OBUF_1),
         .O(Output_1)
     );

OBUF Output_OBUF_0_inst_inst (
         .I(Output_OBUF_0),
         .O(Output_0)
     );

OBUF Output_OBUF_2_inst_inst (
         .I(Output_OBUF_2),
         .O(Output_2)
     );
endmodule

