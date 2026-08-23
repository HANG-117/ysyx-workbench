TOPNAME = NPC
NXDC_FILES = constr/top.nxdc
INC_PATH ?= 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --x-initial fast --noassert

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# project source
# core_defs.sv 是纯宏头文件，不作为独立编译单元参与编译，
# 仅通过 +incdir+vsrc 被各模块 `include 解析。
DEFS = $(abspath ./vsrc)/core_defs.sv
VSRCS = $(filter-out $(DEFS), $(shell find $(abspath ./vsrc) -name "*.sv"))
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
CSRCS += $(SRC_AUTO_BIND)
# 头文件也纳入依赖：修改 .hpp/.h 会触发重编
HDRS = $(shell find $(abspath ./csrc) -name "*.h" -or -name "*.hpp")

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH) $(abspath ./csrc) $(abspath $(BUILD_DIR)))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

# difftest 参考模型 (NEMU) 动态库路径：生成到头文件供源码引用，
# 运行时可用环境变量 NPC_REF_SO 覆盖。
REF_SO ?= $(abspath ../nemu/build/riscv32-nemu-interpreter-so)
REF_SO_HDR = $(abspath $(BUILD_DIR)/ref_so_path.h)
$(REF_SO_HDR):
	@echo '#pragma once' > $@
	@echo '#define REF_SO_PATH "$(REF_SO)"' >> $@

# dlopen 需要
LDFLAGS += -ldl


$(BIN): $(VSRCS) $(CSRCS) $(HDRS) $(NVBOARD_ARCHIVE) $(REF_SO_HDR)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $(filter-out $(REF_SO_HDR) $(HDRS), $^) \
		+incdir+$(abspath ./vsrc) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

all: default

IMG ?= test/mem.bin

run: $(BIN)
	@echo "Running NPC with image: $(IMG)"
	@$^ $(IMG)

sdb: $(BIN)
	@echo "Running NPC with image: $(IMG)"
	@$^ $(IMG) --sdb

clean:
	rm -rf $(BUILD_DIR)

test:
	riscv64-unknown-elf-gcc \
	-march=rv32i \
	-mabi=ilp32 \
	-nostdlib \
	-T test/link.ld \
	-o test/test.elf \
	test/test.S
	riscv64-unknown-elf-objcopy -O binary test/test.elf test/test.bin
convert:
	tail -n +2 test/mem.hex | sed -e 's/.*: //' -e 's/ /\n/g' | sed -e 's/\(..\)\(..\)\(..\)\(..\)/\4 \3 \2 \1/' | xxd -r -p > test/mem.bin
.PHONY: default all clean run sdb test
