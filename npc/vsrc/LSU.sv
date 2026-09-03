module LSU(
    input  logic                        clk,
    input  logic                        rst,
    input  logic                        valid_i,
    input  core_types_pkg::lsu_req_t    req_i,
    output logic                        result_valid_o,
    output core_types_pkg::lsu_result_t result_o
`ifdef SYNTHESIS
    ,
    output logic                        dmem_valid_o,
    output logic                        dmem_we_o,
    output logic [31:0]                 dmem_addr_o,
    output logic [31:0]                 dmem_wdata_o,
    output logic [3:0]                  dmem_wstrb_o,
    input  logic [31:0]                 dmem_rdata_i
`endif
);

    typedef enum logic {
        LSU_IDLE,
        LSU_WAIT_RSP
    } lsu_state_t;

    lsu_state_t               state_q;
    core_types_pkg::lsu_req_t req_q;
    logic                     req_fire;
    logic [31:0]              dmem_rdata;
    logic [31:0]              store_data;
    logic [3:0]               wmask;
    logic                     dmem_we;
    logic [31:0]              dmem_addr;

    // Only one request may be outstanding. The request is pulsed for one
    // cycle, then the synchronous DRAM result is consumed in LSU_WAIT_RSP.
    assign req_fire       = (state_q == LSU_IDLE) && valid_i &&
                            (req_i.load || req_i.store);
    assign result_valid_o = (state_q == LSU_WAIT_RSP);

    always_ff @(posedge clk or posedge rst) begin
        if (rst) begin
            state_q <= LSU_IDLE;
            req_q   <= '0;
        end else begin
            case (state_q)
                LSU_IDLE: begin
                    if (req_fire) begin
                        req_q   <= req_i;
                        state_q <= LSU_WAIT_RSP;
                    end
                end
                LSU_WAIT_RSP: state_q <= LSU_IDLE;
                default:      state_q <= LSU_IDLE;
            endcase
        end
    end

    // RISC-V sb/sh use the low byte/halfword of rs2. Shift that value into
    // the byte lanes selected by wmask before presenting it to DRAM.
    always_comb begin
        case (req_i.mem_size)
            2'b00:   store_data = {4{req_i.store_data[7:0]}} <<
                                   (8 * req_i.addr[1:0]);
            2'b01:   store_data = {2{req_i.store_data[15:0]}} <<
                                   (8 * req_i.addr[1:0]);
            default: store_data = req_i.store_data;
        endcase
    end

    always_comb begin
        case (req_i.mem_size)
            2'b00:   wmask = 4'b0001 << req_i.addr[1:0];
            2'b01:   wmask = (req_i.addr[1:0] == 2'b00) ? 4'b0011 :
                              (req_i.addr[1:0] == 2'b10) ? 4'b1100 :
                                                           4'b0000;
            2'b10:   wmask = 4'b1111;
            default: wmask = 4'b0000;
        endcase
    end

    // Keep the accepted address stable while waiting for the response. The
    // valid/write strobes themselves are asserted only in the request cycle,
    // preventing a held instruction from issuing duplicate stores.
    assign dmem_addr = (state_q == LSU_IDLE) ? req_i.addr : req_q.addr;
    assign dmem_we   = req_fire && req_i.store;

`ifdef SYNTHESIS
    assign dmem_valid_o = req_fire;
    assign dmem_we_o    = dmem_we;
    assign dmem_addr_o  = dmem_addr;
    assign dmem_wdata_o = store_data;
    assign dmem_wstrb_o = wmask;
    assign dmem_rdata   = dmem_rdata_i;
`else
    dram dram_inst(
        .clk  (clk),
        .valid(req_fire),
        .wen  (dmem_we),
        .addr (dmem_addr),
        .wdata(store_data),
        .mask (wmask),
        .rdata(dmem_rdata)
    );
`endif

    // Load extraction uses the latched request, because req_i belongs to the
    // combinational issue path while dmem_rdata belongs to the previous cycle.
    always_comb begin
        result_o.meta      = req_q.meta;
        result_o.load_data = 32'b0;

        if (req_q.load) begin
            case (req_q.mem_size)
                2'b00: begin
                    case (req_q.addr[1:0])
                        2'b00: result_o.load_data = req_q.mem_unsigned
                                                   ? {24'b0, dmem_rdata[7:0]}
                                                   : {{24{dmem_rdata[7]}}, dmem_rdata[7:0]};
                        2'b01: result_o.load_data = req_q.mem_unsigned
                                                   ? {24'b0, dmem_rdata[15:8]}
                                                   : {{24{dmem_rdata[15]}}, dmem_rdata[15:8]};
                        2'b10: result_o.load_data = req_q.mem_unsigned
                                                   ? {24'b0, dmem_rdata[23:16]}
                                                   : {{24{dmem_rdata[23]}}, dmem_rdata[23:16]};
                        2'b11: result_o.load_data = req_q.mem_unsigned
                                                   ? {24'b0, dmem_rdata[31:24]}
                                                   : {{24{dmem_rdata[31]}}, dmem_rdata[31:24]};
                    endcase
                end
                2'b01: begin
                    if (req_q.addr[1:0] == 2'b00) begin
                        result_o.load_data = req_q.mem_unsigned
                                             ? {16'b0, dmem_rdata[15:0]}
                                             : {{16{dmem_rdata[15]}}, dmem_rdata[15:0]};
                    end else if (req_q.addr[1:0] == 2'b10) begin
                        result_o.load_data = req_q.mem_unsigned
                                             ? {16'b0, dmem_rdata[31:16]}
                                             : {{16{dmem_rdata[31]}}, dmem_rdata[31:16]};
                    end
                end
                2'b10:   result_o.load_data = dmem_rdata;
                default: result_o.load_data = 32'b0;
            endcase
        end
    end

endmodule
