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

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""


$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		+incdir+$(abspath ./vsrc) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

all: default

IMG ?= test/mem.bin

run: $(BIN)
	@echo "Running NPC with image: $(IMG)"
	@$^ $(IMG)


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
.PHONY: default all clean run test

