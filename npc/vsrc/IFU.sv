module IFU(
    input  logic        clk,
    input  logic        rst,
    input  logic [31:0] brach_target_i,
    input  logic        pc_redirect_valid,
    input  logic [31:0] pc_redirect_target,
    input  logic        ready_i,
    output logic        valid_o,
    output logic [31:0] inst_o,
    output logic [31:0] pc_o
`ifdef SYNTHESIS
    ,
    output logic [31:0] imem_addr_o,
    input  logic [31:0] imem_rdata_i
`endif
);

    typedef enum logic {
        IFU_FETCH,
        IFU_VALID
    } ifu_state_t;

    ifu_state_t state_q;
    logic [31:0] next_pc;
    logic [31:0] imem_addr;
    logic [31:0] imem_rdata;
    logic        inst_fire;

    // irom samples imem_addr on a rising edge. IFU_FETCH reserves that
    // cycle for the synchronous read; IFU_VALID exposes the returned word
    // until the rest of the core accepts it.
    assign valid_o   = (state_q == IFU_VALID);
    assign inst_fire = valid_o && ready_i;
    assign next_pc   = inst_fire
                     ? (pc_redirect_valid ? pc_redirect_target : brach_target_i)
                     : pc_o;

    pc_reg pc_reg_inst(
        .clk      (clk),
        .rst      (rst),
        .next_pc_i(next_pc),
        .pc_o     (pc_o)
    );

    always_ff @(posedge clk or posedge rst) begin
        if (rst) begin
            state_q <= IFU_FETCH;
        end else begin
            case (state_q)
                IFU_FETCH: state_q <= IFU_VALID;
                IFU_VALID: begin
                    if (ready_i) begin
                        state_q <= IFU_FETCH;
                    end
                end
                default: state_q <= IFU_FETCH;
            endcase
        end
    end

    assign imem_addr = pc_o;
    assign inst_o    = imem_rdata;

`ifdef SYNTHESIS
    assign imem_addr_o = imem_addr;
    assign imem_rdata  = imem_rdata_i;
`else
    irom irom_inst(
        .clk (clk),
        .addr(imem_addr),
        .data(imem_rdata)
    );
`endif

endmodule
