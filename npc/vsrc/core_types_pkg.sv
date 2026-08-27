package core_types_pkg;

    // Information that identifies an instruction and must follow it through
    // the backend.  rob_id/exception fields can be added here when the core
    // gains a ROB; keeping the common fields together avoids losing them at a
    // stage boundary.
    typedef struct packed {
        logic [31:0] pc;
        logic [31:0] inst;
        logic [4:0]  rs1_addr;
        logic [4:0]  rs2_addr;
        logic [4:0]  rd_addr;
        logic        rd_valid;
    } uop_meta_t;

    // IDU -> issue decoded micro-operation.
    typedef struct packed {
        uop_meta_t   meta;
        logic [31:0] imm;

        logic        csr;
        logic        exec;
        logic        load;
        logic        store;
        logic        branch;
        logic        jump;
        logic        jump_base_rs1;

        logic [1:0]  mem_size;
        logic        mem_unsigned;
        logic [3:0]  csr_type;
        logic [11:0] csr_addr;
        logic [3:0]  alu_op;
        logic [2:0]  wb_sel;
        logic        alu_rs1_sel;
        logic        alu_rs2_sel;
        logic [2:0]  branch_cond;
    } decoded_uop_t;

    // Issue -> EXU request.  Operand values travel with their control
    // information so this interface can later be registered or queued.
    typedef struct packed {
        uop_meta_t  meta;
        logic [31:0] rs1_data;
        logic [31:0] rs2_data;
        logic [31:0] imm;
        logic        alu_rs1_sel;
        logic        alu_rs2_sel;
        logic [3:0]  alu_op;
        logic [2:0]  branch_cond;
        logic        branch_en;
        logic        jump_en;
        logic        jump_base_rs1;
    } exu_req_t;

    typedef struct packed {
        uop_meta_t   meta;
        logic [31:0] alu_result;
        logic        branch_taken;
        logic [31:0] next_pc;
    } exu_result_t;

    // Issue -> LSU request.  The issue stage generates addr as rs1 + imm,
    // allowing memory uops to bypass EXU entirely.
    typedef struct packed {
        uop_meta_t   meta;
        logic        load;
        logic        store;
        logic [31:0] addr;
        logic [31:0] store_data;
        logic [1:0]  mem_size;
        logic        mem_unsigned;
    } lsu_req_t;

    typedef struct packed {
        uop_meta_t   meta;
        logic [31:0] load_data;
    } lsu_result_t;

    // Backend results collected for architectural register writeback.
    typedef struct packed {
        uop_meta_t   meta;
        logic [2:0]  wb_sel;
        logic [31:0] alu_result;
        logic [31:0] load_data;
        logic [31:0] csr_data;
    } wbu_req_t;

    typedef struct packed {
        logic        wen;
        logic [4:0]  addr;
        logic [31:0] data;
    } reg_write_t;

endpackage
