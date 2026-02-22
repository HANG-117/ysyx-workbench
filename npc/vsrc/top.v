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
	logic [4:0] rs1, rs2, rd;
	logic [31:0] reg_data1, reg_data2;
	logic [6:0] opcode;
	logic [2:0] funct3;
	logic [6:0] funct7;
	logic [31:0] imm;


	logic [4:0] reg_addr;
 	logic [31:0] reg_wdata;
	logic reg_write;
	
	logic [2:0] ins_type;

	RegisterFile #(5, 32) regfile (
		.clk(clk),
		.rst(rst),
		.pc(PC),
		.raddr1(rs1),
		.raddr2(rs2),
		.rdata1(reg_data1),
		.rdata2(reg_data2),
		.wdata(reg_wdata),
		.waddr(rd),
		.wen(reg_write)
	);

	parameter I = 3'b000, S= 3'b001, R = 3'b010, B = 3'b011, U = 3'b100, J = 3'b101;

	always @(*) begin
		opcode = instruction[6:0];
		rs1 = instruction[19:15];
		rs2 = instruction[24:20];
		funct3 = instruction[14:12];
		funct7 = instruction[31:25];
		case (opcode)
			7'b0010011,7'b1100111,7'b0000011: begin // I-type
				imm = {{20{instruction[31]}}, instruction[31:20]};
				ins_type = I;
			end
			7'b0100011: begin // S-type
				imm = {{20{instruction[31]}}, instruction[31:25], instruction[11:7]};
				ins_type = S;
			end
			7'b0110011: begin // R-type
				imm = 32'b0;
				ins_type = R;
			end
			7'b1100011: begin // B-type
				imm = {{20{instruction[31]}}, instruction[7], instruction[30:25], instruction[11:8], 1'b0};
				ins_type = B;
			end
			7'b0110111, 7'b0010111: begin // U-type
				imm = {instruction[31:12], 12'b0};
				ins_type = U;
			end
			7'b1101111: begin // J-type
				imm = {{12{instruction[31]}}, instruction[19:12], instruction[20], instruction[30:21], 1'b0};
				ins_type = J;
			end

			default: begin
				imm = 32'b0;
				ins_type = 3'b111; 
			end
		endcase
	end


	always @(posedge clk) begin
		if(rst) begin
			PC <= 32'h00000000;
			reg_write <= 1'b0;
		end
		else begin
			reg_write <= 1'b0;
			case (ins_type)
				I: begin
					case (opcode)
						7'b0010011: begin
							case (funct3)
								3'b000: begin
									//addi
									reg_wdata <= reg_data1 + imm;
									reg_write <= 1'b1;
								end
								default: begin
								end 
							endcase 
						end
						default: begin
							
						end
					endcase 
				end
				default: begin
				end
			endcase 
			PC <= PC + 4;
		end
	end
	
endmodule