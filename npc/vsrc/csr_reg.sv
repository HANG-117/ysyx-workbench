`include "core_defs.sv"

module csr_reg(
    input  logic        clk,
    input  logic        rst,

    input  logic [11:0] addr,
    input  logic [3:0]  csr_type,

    input  logic [4:0]  rs1_addr,
    input  logic [31:0] rs1_data,

    input  logic [31:0] pc,
    input  logic [31:0] a0,

    output logic [31:0] csr_rdata,

    // ECALL / MRET 修改 PC
    output logic        pc_redirect_valid,
    output logic [31:0] pc_redirect_target
);

    import "DPI-C" function void sim_exit(
        input int pc,
        input int a0
    );


    logic [63:0] mcycle;

    logic [31:0] mtvec;
    logic [31:0] mstatus;
    logic [31:0] mepc;
    logic [31:0] mcause;

    logic        csr_wen;
    logic [31:0] zimm;

    logic [31:0] csr_wdata;
    localparam logic [11:0] CSR_MSTATUS = 12'h300;
    localparam logic [11:0] CSR_MTVEC   = 12'h305;
    localparam logic [11:0] CSR_MEPC    = 12'h341;
    localparam logic [11:0] CSR_MCAUSE  = 12'h342;

    localparam logic [11:0] CSR_MCYCLE  = 12'hB00;
    localparam logic [11:0] CSR_MCYCLEH = 12'hB80;

    // 与 NEMU 参考模型一致：立即数形式的 zimm 取 rs1 寄存器值低 5 位
    // （NEMU inst.c: uint32_t zimm = (src1) & 0x1f;）
    assign zimm = rs1_data & 32'h1F;

    always_comb begin
        case (addr)

            CSR_MSTATUS: csr_rdata = mstatus;
            CSR_MTVEC:   csr_rdata = mtvec;
            CSR_MEPC:    csr_rdata = mepc;
            CSR_MCAUSE:  csr_rdata = mcause;

            CSR_MCYCLE:  csr_rdata = mcycle[31:0];
            CSR_MCYCLEH: csr_rdata = mcycle[63:32];

            default:     csr_rdata = 32'b0;

        endcase
    end

    always_comb begin

        // 默认不写 CSR
        csr_wen   = 1'b0;
        csr_wdata = csr_rdata;

        case (csr_type)

            `CSR_CSRRW: begin
                csr_wen   = 1'b1;
                csr_wdata = rs1_data;
            end

            // 与 NEMU 一致：csrrs 只读，不写 CSR（NEMU 忽略 rs1）
            `CSR_CSRRS: begin
            end

            `CSR_CSRRC: begin
                if (rs1_addr != 5'b0) begin
                    csr_wen   = 1'b1;
                    csr_wdata = csr_rdata & ~rs1_data;
                end
            end

            `CSR_CSRRWI: begin
                csr_wen   = 1'b1;
                csr_wdata = zimm;
            end

            `CSR_CSRRSI: begin
                csr_wen   = 1'b1;
                csr_wdata = csr_rdata | zimm;
            end

            `CSR_CSRRCI: begin
                csr_wen   = 1'b1;
                csr_wdata = csr_rdata & ~zimm;
            end
            `CSR_ECALL: begin
            end
            `CSR_EBREAK: begin
            end
            `CSR_MRET: begin
            end
            default: begin
            end

        endcase
    end
    always_comb begin
        pc_redirect_valid  = 1'b0;
        pc_redirect_target = 32'b0;
        case (csr_type)
            `CSR_ECALL: begin
                pc_redirect_valid  = 1'b1;
                pc_redirect_target = {mtvec[31:2], 2'b00};
            end
            `CSR_MRET: begin
                pc_redirect_valid  = 1'b1;
                pc_redirect_target = mepc;
            end
            default: begin
            end

        endcase
    end
    always_ff @(posedge clk or posedge rst) begin

        if (rst) begin

            mtvec   <= 32'b0;
            // 与 NEMU 一致：复位时 mstatus = 0x1800（NEMU init.c: cpu.mstatus = 0x1800）
            mstatus <= 32'h1800;
            mepc    <= 32'b0;
            mcause  <= 32'b0;

        end
        else if (csr_type == `CSR_ECALL) begin

            // 与 NEMU 一致：mepc = epc + 4，且不修改 mstatus
            // （NEMU intr.c: cpu.mepc = epc + 4；rt-thread bsp 的协作式调度依赖此约定）
            mepc   <= pc + 32'd4;
            mcause <= 32'd11;

        end
        else if (csr_type == `CSR_MRET) begin

            // 与 NEMU 一致：mstatus = (mstatus & ~0x1800) | ((mstatus & 0x1800) >> 4)
            mstatus <= (mstatus & ~32'h1800) | ((mstatus & 32'h1800) >> 4);

        end
        else if (csr_wen) begin
            case (addr)
                CSR_MSTATUS: mstatus <= csr_wdata;
                CSR_MTVEC: begin
                    // 当前先允许软件写完整 mtvec
                    mtvec <= csr_wdata;
                end
                CSR_MEPC: begin
                    mepc <= csr_wdata;
                end
                CSR_MCAUSE: begin
                    mcause <= csr_wdata;
                end
                default: begin
                end
            endcase
        end
    end


    always_ff @(posedge clk or posedge rst) begin
        if (rst) begin
            mcycle <= 64'b0;
        end
        else if (csr_wen && addr == CSR_MCYCLE) begin
            mcycle[31:0] <= csr_wdata;
        end
        else if (csr_wen && addr == CSR_MCYCLEH) begin
            mcycle[63:32] <= csr_wdata;
        end
        else begin
            mcycle <= mcycle + 64'd1;
        end

    end


    always_ff @(posedge clk) begin
        if (!rst && csr_type == `CSR_EBREAK) begin
            sim_exit(pc, a0);
        end

    end


endmodule
