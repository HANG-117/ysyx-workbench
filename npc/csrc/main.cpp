#include <nvboard.h>
#include <Vtop.h>
#include <iostream>
#include <fstream>
#define MEM_SIZE_WORDS (1 << 18) 
static uint32_t ROM[MEM_SIZE_WORDS];
static uint32_t RAM[MEM_SIZE_WORDS];
static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);

void load_program(const char* filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error: Cannot open " << filename << std::endl;
    exit(1);
  }

  // 读取整个文件到 buffer
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  char* buffer = new char[size];
  file.read(buffer, size);
  file.close();

  // 按字（4 字节）写入 ROM
  uint32_t* words = (uint32_t*)buffer;
  size_t word_count = size / 4;
  for (size_t i = 0; i < word_count; i++) {
    ROM[i] = words[i];
  }

  delete[] buffer;
  std::cout << "Loaded " << word_count << " instructions from " << filename << std::endl;
}


static void single_cycle() {
  dut.instruction = ROM[dut.PC >> 2];
  dut.clk = 0; dut.eval();
  if(dut.mem_read) {
		if (dut.mem_addr < 0x80000000) {
			dut.mem_rdata = ROM[dut.mem_addr >> 2];
		} else {
			dut.mem_rdata = RAM[(dut.mem_addr - 0x80000000) >> 2];
		}
	}
  dut.clk = 1; dut.eval();
	if(dut.mem_write) {
		if (dut.mem_addr < 0x80000000) {
			ROM[dut.mem_addr >> 2] = dut.mem_wdata;
		} else {
			RAM[(dut.mem_addr - 0x80000000) >> 2] = dut.mem_wdata;
		}
	}
}

static void reset(int n) {
  dut.rst = 1;
  while (n -- > 0) single_cycle();
  dut.rst = 0;
}

int main() {
  nvboard_bind_all_pins(&dut);
  nvboard_init();
  load_program("test/addi_test.bin");
  reset(10);

  while(1) {
    nvboard_update();
    single_cycle();
    if(dut.PC == 0x10) break;

  }
}
