TOPNAME = NPC
NXDC_FILES = constr/top.nxdc
INC_PATH ?= 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --x-initial fast --noassert

# RTL synthesis/evaluation.  A recent OSS CAD Suite build is recommended so
# that the read_slang frontend can elaborate packages and packed structs.
YOSYS ?= yosys
YOSYS_BUILD_DIR = $(BUILD_DIR)/yosys
YOSYS_GENERIC_LOG = $(YOSYS_BUILD_DIR)/generic.log
YOSYS_ASIC_LOG = $(YOSYS_BUILD_DIR)/asic.log
LIBERTY ?=

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
# SystemVerilog package 必须先于引用它的模块编译；不要依赖 find 的目录顺序。
CORE_TYPES_PKG = $(abspath ./vsrc)/core_types_pkg.sv
VSRCS = $(CORE_TYPES_PKG) $(filter-out $(DEFS) $(CORE_TYPES_PKG), $(shell find $(abspath ./vsrc) -name "*.sv"))
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

# Generic synthesis gives technology-independent gate/flip-flop counts.  The
# SYNTHESIS define switches IFU/LSU from DPI-C calls to external memory ports.
yosys: yosys-generic

check-yosys:
	@if ! command -v $(YOSYS) >/dev/null 2>&1; then \
		echo "error: yosys was not found; install/activate a recent OSS CAD Suite"; \
		echo "       https://github.com/YosysHQ/oss-cad-suite-build#installation"; \
		exit 127; \
	fi
	@if ! $(YOSYS) -Q -p 'help read_slang' 2>/dev/null | grep -q "Read SystemVerilog sources"; then \
		echo "error: this Yosys build has no read_slang frontend"; \
		echo "       activate a recent OSS CAD Suite, then run make yosys again"; \
		exit 2; \
	fi

yosys-generic: check-yosys
	@mkdir -p $(YOSYS_BUILD_DIR)
	$(YOSYS) -Q -l $(YOSYS_GENERIC_LOG) -p "read_slang --std 1800-2017 --single-unit --top $(TOPNAME) \
			-I $(abspath ./vsrc) -D SYNTHESIS=1 $(VSRCS); \
		hierarchy -check -top $(TOPNAME); \
		check; \
		synth -top $(TOPNAME) -flatten; \
		check; \
		tee -o $(YOSYS_BUILD_DIR)/generic_stat.txt stat -top $(TOPNAME) -width; \
		tee -o $(YOSYS_BUILD_DIR)/generic_stat.json stat -top $(TOPNAME) -json; \
		write_json $(YOSYS_BUILD_DIR)/$(TOPNAME)_generic.json; \
		write_verilog -noattr $(YOSYS_BUILD_DIR)/$(TOPNAME)_generic.v \
	"
	@echo "Yosys generic report: $(YOSYS_BUILD_DIR)/generic_stat.txt"

# Map the CPU to a standard-cell library for a library-dependent area estimate
# and Yosys' preliminary, flattened single-module timing report.
# Usage: make yosys-asic LIBERTY=/absolute/path/to/typical.lib
yosys-asic: check-yosys
	@if [ -z "$(LIBERTY)" ]; then \
		echo "error: set LIBERTY=/absolute/path/to/standard_cells.lib"; \
		exit 2; \
	fi
	@if [ ! -f "$(LIBERTY)" ]; then \
		echo "error: Liberty file does not exist: $(LIBERTY)"; \
		exit 2; \
	fi
	@mkdir -p $(YOSYS_BUILD_DIR)
	$(YOSYS) -Q -l $(YOSYS_ASIC_LOG) -p "read_slang --std 1800-2017 --single-unit --top $(TOPNAME) \
			-I $(abspath ./vsrc) -D SYNTHESIS=1 $(VSRCS); \
		hierarchy -check -top $(TOPNAME); \
		check; \
		synth -top $(TOPNAME) -flatten; \
		dfflibmap -liberty \"$(abspath $(LIBERTY))\"; \
		abc -liberty \"$(abspath $(LIBERTY))\"; \
		clean; \
		check; \
		tee -o $(YOSYS_BUILD_DIR)/asic_stat.txt stat -top $(TOPNAME) -liberty \"$(abspath $(LIBERTY))\"; \
		tee -o $(YOSYS_BUILD_DIR)/asic_timing.txt sta; \
		write_verilog -noattr -noexpr $(YOSYS_BUILD_DIR)/$(TOPNAME)_mapped.v \
	"
	@echo "Yosys ASIC area report: $(YOSYS_BUILD_DIR)/asic_stat.txt"
	@echo "Yosys ASIC timing report: $(YOSYS_BUILD_DIR)/asic_timing.txt"

yosys-help:
	@echo "make yosys"
	@echo "  Generic technology-independent cell statistics."
	@echo "make yosys-asic LIBERTY=/absolute/path/to/typical.lib"
	@echo "  Standard-cell area and preliminary timing statistics."

.PHONY: default all clean run sdb test yosys yosys-generic yosys-asic yosys-help check-yosys
