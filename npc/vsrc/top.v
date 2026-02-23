module top (
	input clk,
	input rst,


	output logic mem_write,
	output logic mem_read,
	output logic [31:0] mem_addr,
	output logic [31:0] mem_wdata,
	input  logic [31:0] mem_rdata,

	output logic [31:0] PC,
	output logic [31:0] instruction	
);
	wire [4:0] rd,rs1,rs2;
	wire [2:0] funct3;
	wire [6:0] funct7;
	wire [6:0] opcode;
	wire [31:0] imm;
	
	assign opcode = instruction[6:0];
	assign rd = instruction[11:7];
	assign funct3 = instruction[14:12];
	assign rs1 = instruction[19:15];
	assign rs2 = instruction[24:20];
	assign funct7 = instruction[31:25];

	// assign mem_addr    = PC;
	// assign mem_read    = 1'b1;               
	// assign mem_write   = 1'b0;               
	// assign mem_wdata   = 32'h0;
	// assign instruction = mem_rdata;

	imm_gen immgen (
		.instruction(instruction),
		.imm(imm)
	);


	reg reg_write;
	reg [31:0] reg_wdata, reg_rdata1, reg_rdata2;
	reg [31:0] alu_result;
	reg [31:0] alu_in1, alu_in2;
	reg [2:0] alu_op;

	Alu alu (
		.in1(alu_in1),
		.in2(alu_in2),
		.alu_op(alu_op),
		.result(alu_result)
	);

	RegisterFile #(.ADDR_WIDTH(5), .DATA_WIDTH(32)) regfile (
		.clk(clk),
		.rst(rst),
		.pc(PC),
		.raddr1(rs1),
		.raddr2(rs2),
		.rdata1(reg_rdata1),
		.rdata2(reg_rdata2),
		.wdata(reg_wdata),
		.waddr(rd),
		.wen(reg_write&&(rd != 0)&&(rst == 0))
	);


	always_comb begin
		reg_write = 1'b0;
		reg_wdata = 32'b0;
		alu_op    = 3'b000;    
		alu_in1   = reg_rdata1;
		alu_in2   = reg_rdata2;   
		case(opcode)
			7'b0010011: begin 
				case(funct3)
					3'b000: begin
						alu_op = 3'b000; // ADDI
						alu_in1 = reg_rdata1;
						alu_in2 = imm;
						reg_wdata = alu_result;
						reg_write = 1;
					end
					default: begin
						reg_write = 1'b0;
					end
				endcase
			end
			default: begin
				reg_write = 1'b0;
			end
		endcase
	end

	// PC
	always_ff @(posedge clk) begin
		if (rst) begin
			PC <= 32'h0;
		end else begin
			$display("PC: %08h Instruction: %08h", PC, instruction);
			PC <= PC + 4;
		end
	end

endmodule