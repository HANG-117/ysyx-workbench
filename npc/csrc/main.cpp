#include <nvboard.h>
#include <Vtop.h>
#include <iostream>
#include <fstream>
#define MEM_SIZE_WORDS (1 << 18) 
static uint32_t ROM[MEM_SIZE_WORDS];
static uint32_t RAM[MEM_SIZE_WORDS];
static TOP_NAME *npc = new TOP_NAME;

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

void pmem_read(uint32_t addr){
  if (addr < 0x80000000) {
    npc->mem_rdata = ROM[addr >> 2];
  } else {
    npc->mem_rdata = RAM[(addr - 0x80000000) >> 2];
  }
}


static void single_cycle() {
  npc->instruction = pmem_read(npc->PC);
  npc->eval();

  npc->clk = 0; npc->eval();
  if(npc->mem_read) {
		if (npc->mem_addr < 0x80000000) {
			npc->mem_rdata = ROM[npc->mem_addr >> 2];
		} else {
			npc->mem_rdata = RAM[(npc->mem_addr - 0x80000000) >> 2];
		}
	}
  npc->clk = 1; npc->eval();
	if(npc->mem_write) {
		if (npc->mem_addr < 0x80000000) {
			ROM[npc->mem_addr >> 2] = npc->mem_wdata;
		} else {
			RAM[(npc->mem_addr - 0x80000000) >> 2] = npc->mem_wdata;
		}
	}
}

static void reset(int n) {
  npc->rst = 1;
  while (n -- > 0) single_cycle();
  npc->rst = 0;
}

int main() {
  nvboard_bind_all_pins(npc);
  nvboard_init();
  load_program("test/addi_test.bin");
  reset(10);

  while(1) {
    nvboard_update();
    single_cycle();
    if(npc->PC == 0x10) break;

  }
}
