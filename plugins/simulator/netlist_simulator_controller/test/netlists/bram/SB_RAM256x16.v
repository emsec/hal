`timescale 1ps/1ps
module SB_RAM256x16 (RDATA, RCLK, RCLKE, RE, RADDR, WCLK, WCLKE, WE, WADDR, MASK, WDATA);
output [15:0] RDATA;
input RCLK;
input RCLKE;
input RE;
input [7:0] RADDR;
input WCLK;
input WCLKE;
input WE;
input [7:0] WADDR;
input [15:0] MASK;
input [15:0] WDATA;

parameter INIT_0 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_1 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_2 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_3 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_4 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_5 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_6 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_7 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_8 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_9 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_A = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_B = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_C = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_D = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_E = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter INIT_F = 256'h0000000000000000000000000000000000000000000000000000000000000000;

// local Parameters
localparam			CLOCK_PERIOD = 200;	//
localparam 			DELAY	= (CLOCK_PERIOD/10);		// Clock-to-output delay. Zero
							// time delays can be confusing
							// and sometimes cause problems.
localparam 			BUS_WIDTH = 16;		// Width of RAM (number of bits)

localparam 			ADDRESS_BUS_SIZE = 8;	// Number of bits required to
							// represent the RAM address

localparam   ADDRESSABLE_SPACE  = 2**ADDRESS_BUS_SIZE;	// Decimal address range [2^Size:0]


// SIGNAL DECLARATIONS
wire			   	WCLK_g, RCLK_g;
reg 				WCLKE_sync, RCLKE_sync; 
reg	Memory	[BUS_WIDTH*ADDRESSABLE_SPACE-1:0];
// 
//event Read_e, Write_e;

//////////////////// Collision detect begins here ///////////////////////////////
localparam 	TRUE = 1'b1;
localparam	FALSE = 1'b0;
reg 		Time_Collision_Detected = 1'b0;
wire		Address_Collision_Detected;

//event Collision_e;

time COLLISION_TIME_WINDOW = (CLOCK_PERIOD/8); // This is an arbitray value, but is better than using an absolute 
						    // value, because the actual time window depends on the actual silicon 
						    // implementation. Thus the test is indicative of an Error and not
						    // guaranteed to be an error. Even so this is usefull.
time time_WCLK, time_RCLK;


//function reg Check_Timed_Window_Violation;
function	Check_Timed_Window_Violation;	
input T1, T2, Minimum_Time_Window;
time T1, T2;
time Minimum_Time_Window;
time Difference;	
	begin
		Difference = (T1 - T2);
		if (Difference < 0) Difference = -Difference;
		Check_Timed_Window_Violation = (Difference < Minimum_Time_Window);
	end
endfunction


initial begin
       time_WCLK = CLOCK_PERIOD;	// Arbitrary initialisation value, ensure no window collison error on first clock edge.
       time_RCLK = (CLOCK_PERIOD*8);	// Arbitrary initialisation difference value, ensure no collision error on first clock edge.					
end

integer	i,j;


initial	//	initialize ram_4k by parameter, section by section
begin
	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[BUS_WIDTH*i+j]	=	INIT_0[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*1+BUS_WIDTH*i+j]	=	INIT_1[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*2+BUS_WIDTH*i+j]	=	INIT_2[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*3+BUS_WIDTH*i+j]	=	INIT_3[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*4+BUS_WIDTH*i+j]	=	INIT_4[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*5+BUS_WIDTH*i+j]	=	INIT_5[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*6+BUS_WIDTH*i+j]	=	INIT_6[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*7+BUS_WIDTH*i+j]	=	INIT_7[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*8+BUS_WIDTH*i+j]	=	INIT_8[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*9+BUS_WIDTH*i+j]	=	INIT_9[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*10+BUS_WIDTH*i+j]	=	INIT_A[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*11+BUS_WIDTH*i+j]	=	INIT_B[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*12+BUS_WIDTH*i+j]	=	INIT_C[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*13+BUS_WIDTH*i+j]	=	INIT_D[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*14+BUS_WIDTH*i+j]	=	INIT_E[BUS_WIDTH*i+j];
	end

	for	(i=0; i<=256/BUS_WIDTH -1; i=i+1)
	begin
		for	(j=0; j<=BUS_WIDTH-1; j=j+1)
			Memory[256*15+BUS_WIDTH*i+j]	=	INIT_F[BUS_WIDTH*i+j];
	end

end

assign Address_Collision_Detected = ((RE & WE & WCLKE & RCLKE)&(WADDR == RADDR)); 

always @(WCLK or WCLKE) 
begin 
	if(~WCLK)
	WCLKE_sync = WCLKE;   	
end 

always @(RCLK or RCLKE) 
begin 
	if (~RCLK)
	RCLKE_sync = RCLKE; 	
end 

assign WCLK_g = WCLK & WCLKE_sync;
assign RCLK_g = RCLK & RCLKE_sync;


always @(posedge WCLK_g) begin
	time_WCLK = $time;
end

always @(posedge RCLK_g) begin
    	time_RCLK = $time;
end


//	code modify for universal verilog compiler

always @ (posedge WCLK_g)
begin
        Time_Collision_Detected = Check_Timed_Window_Violation(time_WCLK,time_RCLK,COLLISION_TIME_WINDOW);
        if (Time_Collision_Detected & Address_Collision_Detected)begin
                $display("Warning: Write-Read collision detected, Data read value is XXXX\n");
                $display("WCLK Time: %.3f   RCLK Time:%.3f  ",time_WCLK, time_RCLK,"WADDR: %d   RADDR:%d\n",WADDR, RADDR);
        end

        if	(WE)
	begin
//		-> Write_e;
		for	(i=0;i<=BUS_WIDTH-1; i=i+1)
		begin
			if	(MASK[i] !=1)
				Memory[WADDR*BUS_WIDTH+i]	<=	WDATA[i];
			else
				Memory[WADDR*BUS_WIDTH+i]	<=	Memory[WADDR*BUS_WIDTH+i];
		end
	end
end

//reg	[15:0]	RDATA = 0;
reg	[15:0]	RDATA;

initial
begin
   RDATA = 16'h0000;
end

// Look at the rising edge of the clock

always @ (posedge RCLK_g)
begin
        Time_Collision_Detected = Check_Timed_Window_Violation(time_WCLK,time_RCLK,COLLISION_TIME_WINDOW);
        if (Time_Collision_Detected & Address_Collision_Detected)begin
                $display("Warning: Write-Read collision detected, Data read value is XXXX\n");
                $display("WCLK Time: %.3f   RCLK Time:%.3f  ",time_WCLK, time_RCLK,"WADDR: %d   RADDR:%d\n",WADDR, RADDR);
        end

        if	(RE)
	begin
//		-> Read_e;
		if	(Time_Collision_Detected & Address_Collision_Detected) 
			RDATA <= 16'hXXXX;
		else
			for	(i=0;i<=BUS_WIDTH-1;i=i+1)
				RDATA[i]	<= Memory[RADDR*BUS_WIDTH+i];
	end
end

`ifdef TIMINGCHECK
specify
   (RCLK *> RDATA[0]) = (1.0, 1.0);
   (RCLK *> RDATA[1]) = (1.0, 1.0);
   (RCLK *> RDATA[2]) = (1.0, 1.0);
   (RCLK *> RDATA[3]) = (1.0, 1.0);
   (RCLK *> RDATA[4]) = (1.0, 1.0);
   (RCLK *> RDATA[5]) = (1.0, 1.0);
   (RCLK *> RDATA[6]) = (1.0, 1.0);
   (RCLK *> RDATA[7]) = (1.0, 1.0);
   (RCLK *> RDATA[8]) = (1.0, 1.0);
   (RCLK *> RDATA[9]) = (1.0, 1.0);
   (RCLK *> RDATA[10]) = (1.0, 1.0);
   (RCLK *> RDATA[11]) = (1.0, 1.0);
   (RCLK *> RDATA[12]) = (1.0, 1.0);
   (RCLK *> RDATA[13]) = (1.0, 1.0);
   (RCLK *> RDATA[14]) = (1.0, 1.0);
   (RCLK *> RDATA[15]) = (1.0, 1.0);
   $setup(posedge MASK[0], posedge WCLK, 1.0);
   $setup(negedge MASK[0], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[0], 1.0);
   $hold(posedge WCLK, negedge MASK[0], 1.0);
   $setup(posedge MASK[1], posedge WCLK, 1.0);
   $setup(negedge MASK[1], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[1], 1.0);
   $hold(posedge WCLK, negedge MASK[1], 1.0);
   $setup(posedge MASK[2], posedge WCLK, 1.0);
   $setup(negedge MASK[2], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[2], 1.0);
   $hold(posedge WCLK, negedge MASK[2], 1.0);
   $setup(posedge MASK[3], posedge WCLK, 1.0);
   $setup(negedge MASK[3], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[3], 1.0);
   $hold(posedge WCLK, negedge MASK[3], 1.0);
   $setup(posedge MASK[4], posedge WCLK, 1.0);
   $setup(negedge MASK[4], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[4], 1.0);
   $hold(posedge WCLK, negedge MASK[4], 1.0);
   $setup(posedge MASK[5], posedge WCLK, 1.0);
   $setup(negedge MASK[5], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[5], 1.0);
   $hold(posedge WCLK, negedge MASK[5], 1.0);
   $setup(posedge MASK[6], posedge WCLK, 1.0);
   $setup(negedge MASK[6], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[6], 1.0);
   $hold(posedge WCLK, negedge MASK[6], 1.0);
   $setup(posedge MASK[7], posedge WCLK, 1.0);
   $setup(negedge MASK[7], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[7], 1.0);
   $hold(posedge WCLK, negedge MASK[7], 1.0);
   $setup(posedge MASK[8], posedge WCLK, 1.0);
   $setup(negedge MASK[8], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[8], 1.0);
   $hold(posedge WCLK, negedge MASK[8], 1.0);
   $setup(posedge MASK[9], posedge WCLK, 1.0);
   $setup(negedge MASK[9], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[9], 1.0);
   $hold(posedge WCLK, negedge MASK[9], 1.0);
   $setup(posedge MASK[10], posedge WCLK, 1.0);
   $setup(negedge MASK[10], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[10], 1.0);
   $hold(posedge WCLK, negedge MASK[10], 1.0);
   $setup(posedge MASK[11], posedge WCLK, 1.0);
   $setup(negedge MASK[11], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[11], 1.0);
   $hold(posedge WCLK, negedge MASK[11], 1.0);
   $setup(posedge MASK[12], posedge WCLK, 1.0);
   $setup(negedge MASK[12], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[12], 1.0);
   $hold(posedge WCLK, negedge MASK[12], 1.0);
   $setup(posedge MASK[13], posedge WCLK, 1.0);
   $setup(negedge MASK[13], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[13], 1.0);
   $hold(posedge WCLK, negedge MASK[13], 1.0);
   $setup(posedge MASK[14], posedge WCLK, 1.0);
   $setup(negedge MASK[14], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[14], 1.0);
   $hold(posedge WCLK, negedge MASK[14], 1.0);
   $setup(posedge MASK[15], posedge WCLK, 1.0);
   $setup(negedge MASK[15], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge MASK[15], 1.0);
   $hold(posedge WCLK, negedge MASK[15], 1.0);
   $setup(posedge WADDR[0], posedge WCLK, 1.0);
   $setup(negedge WADDR[0], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[0], 1.0);
   $hold(posedge WCLK, negedge WADDR[0], 1.0);
   $setup(posedge WADDR[1], posedge WCLK, 1.0);
   $setup(negedge WADDR[1], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[1], 1.0);
   $hold(posedge WCLK, negedge WADDR[1], 1.0);
   $setup(posedge WADDR[2], posedge WCLK, 1.0);
   $setup(negedge WADDR[2], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[2], 1.0);
   $hold(posedge WCLK, negedge WADDR[2], 1.0);
   $setup(posedge WADDR[3], posedge WCLK, 1.0);
   $setup(negedge WADDR[3], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[3], 1.0);
   $hold(posedge WCLK, negedge WADDR[3], 1.0);
   $setup(posedge WADDR[4], posedge WCLK, 1.0);
   $setup(negedge WADDR[4], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[4], 1.0);
   $hold(posedge WCLK, negedge WADDR[4], 1.0);
   $setup(posedge WADDR[5], posedge WCLK, 1.0);
   $setup(negedge WADDR[5], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[5], 1.0);
   $hold(posedge WCLK, negedge WADDR[5], 1.0);
   $setup(posedge WADDR[6], posedge WCLK, 1.0);
   $setup(negedge WADDR[6], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[6], 1.0);
   $hold(posedge WCLK, negedge WADDR[6], 1.0);
   $setup(posedge WADDR[7], posedge WCLK, 1.0);
   $setup(negedge WADDR[7], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WADDR[7], 1.0);
   $hold(posedge WCLK, negedge WADDR[7], 1.0);
   $setup(posedge WDATA[0], posedge WCLK, 1.0);
   $setup(negedge WDATA[0], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[0], 1.0);
   $hold(posedge WCLK, negedge WDATA[0], 1.0);
   $setup(posedge WDATA[1], posedge WCLK, 1.0);
   $setup(negedge WDATA[1], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[1], 1.0);
   $hold(posedge WCLK, negedge WDATA[1], 1.0);
   $setup(posedge WDATA[2], posedge WCLK, 1.0);
   $setup(negedge WDATA[2], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[2], 1.0);
   $hold(posedge WCLK, negedge WDATA[2], 1.0);
   $setup(posedge WDATA[3], posedge WCLK, 1.0);
   $setup(negedge WDATA[3], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[3], 1.0);
   $hold(posedge WCLK, negedge WDATA[3], 1.0);
   $setup(posedge WDATA[4], posedge WCLK, 1.0);
   $setup(negedge WDATA[4], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[4], 1.0);
   $hold(posedge WCLK, negedge WDATA[4], 1.0);
   $setup(posedge WDATA[5], posedge WCLK, 1.0);
   $setup(negedge WDATA[5], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[5], 1.0);
   $hold(posedge WCLK, negedge WDATA[5], 1.0);
   $setup(posedge WDATA[6], posedge WCLK, 1.0);
   $setup(negedge WDATA[6], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[6], 1.0);
   $hold(posedge WCLK, negedge WDATA[6], 1.0);
   $setup(posedge WDATA[7], posedge WCLK, 1.0);
   $setup(negedge WDATA[7], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[7], 1.0);
   $hold(posedge WCLK, negedge WDATA[7], 1.0);
   $setup(posedge WDATA[8], posedge WCLK, 1.0);
   $setup(negedge WDATA[8], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[8], 1.0);
   $hold(posedge WCLK, negedge WDATA[8], 1.0);
   $setup(posedge WDATA[9], posedge WCLK, 1.0);
   $setup(negedge WDATA[9], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[9], 1.0);
   $hold(posedge WCLK, negedge WDATA[9], 1.0);
   $setup(posedge WDATA[10], posedge WCLK, 1.0);
   $setup(negedge WDATA[10], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[10], 1.0);
   $hold(posedge WCLK, negedge WDATA[10], 1.0);
   $setup(posedge WDATA[11], posedge WCLK, 1.0);
   $setup(negedge WDATA[11], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[11], 1.0);
   $hold(posedge WCLK, negedge WDATA[11], 1.0);
   $setup(posedge WDATA[12], posedge WCLK, 1.0);
   $setup(negedge WDATA[12], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[12], 1.0);
   $hold(posedge WCLK, negedge WDATA[12], 1.0);
   $setup(posedge WDATA[13], posedge WCLK, 1.0);
   $setup(negedge WDATA[13], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[13], 1.0);
   $hold(posedge WCLK, negedge WDATA[13], 1.0);
   $setup(posedge WDATA[14], posedge WCLK, 1.0);
   $setup(negedge WDATA[14], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[14], 1.0);
   $hold(posedge WCLK, negedge WDATA[14], 1.0);
   $setup(posedge WDATA[15], posedge WCLK, 1.0);
   $setup(negedge WDATA[15], posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WDATA[15], 1.0);
   $hold(posedge WCLK, negedge WDATA[15], 1.0);
   $setup(posedge WCLKE, posedge WCLK, 1.0);
   $setup(negedge WCLKE, posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WCLKE, 1.0);
   $hold(posedge WCLK, negedge WCLKE, 1.0);
   $setup(posedge WE, posedge WCLK, 1.0);
   $setup(negedge WE, posedge WCLK, 1.0);
   $hold(posedge WCLK, posedge WE, 1.0);
   $hold(posedge WCLK, negedge WE, 1.0);
   $setup(posedge RADDR[0], posedge RCLK, 1.0);
   $setup(negedge RADDR[0], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[0], 1.0);
   $hold(posedge RCLK, negedge RADDR[0], 1.0);
   $setup(posedge RADDR[1], posedge RCLK, 1.0);
   $setup(negedge RADDR[1], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[1], 1.0);
   $hold(posedge RCLK, negedge RADDR[1], 1.0);
   $setup(posedge RADDR[2], posedge RCLK, 1.0);
   $setup(negedge RADDR[2], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[2], 1.0);
   $hold(posedge RCLK, negedge RADDR[2], 1.0);
   $setup(posedge RADDR[3], posedge RCLK, 1.0);
   $setup(negedge RADDR[3], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[3], 1.0);
   $hold(posedge RCLK, negedge RADDR[3], 1.0);
   $setup(posedge RADDR[4], posedge RCLK, 1.0);
   $setup(negedge RADDR[4], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[4], 1.0);
   $hold(posedge RCLK, negedge RADDR[4], 1.0);
   $setup(posedge RADDR[5], posedge RCLK, 1.0);
   $setup(negedge RADDR[5], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[5], 1.0);
   $hold(posedge RCLK, negedge RADDR[5], 1.0);
   $setup(posedge RADDR[6], posedge RCLK, 1.0);
   $setup(negedge RADDR[6], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[6], 1.0);
   $hold(posedge RCLK, negedge RADDR[6], 1.0);
   $setup(posedge RADDR[7], posedge RCLK, 1.0);
   $setup(negedge RADDR[7], posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RADDR[7], 1.0);
   $hold(posedge RCLK, negedge RADDR[7], 1.0);
   $setup(posedge RCLKE, posedge RCLK, 1.0);
   $setup(negedge RCLKE, posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RCLKE, 1.0);
   $hold(posedge RCLK, negedge RCLKE, 1.0);
   $setup(posedge RE, posedge RCLK, 1.0);
   $setup(negedge RE, posedge RCLK, 1.0);
   $hold(posedge RCLK, posedge RE, 1.0);
   $hold(posedge RCLK, negedge RE, 1.0);

endspecify
`endif

endmodule //SB_RAM256x16
