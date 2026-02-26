#include <nvboard.h>
#include <VNPC.h>
#include <iostream>
#include <fstream>
#include <sstream>
#define MEM_SIZE_WORDS (1 << 22) 
static uint32_t MEM[MEM_SIZE_WORDS];
static TOP_NAME *npc = new TOP_NAME;
int cycle = 0;

static bool simulation_should_exit = false;
extern "C" void sim_exit(){
  simulation_should_exit = true;
}
extern "C" uint32_t pmem_read(uint32_t addr) {
  if(addr >= MEM_SIZE_WORDS * 4) {
    printf("Error: Memory access out of bounds: %08x\n",addr);
    printf("cycle : %d",cycle);
    exit(1);
  }
  
  return MEM[addr >> 2];
}
extern "C" void pmem_write(uint32_t addr, uint32_t data,int size) {
  if(size == 4){
    MEM[addr >> 2] = data;
    printf("%08xWrite to RAM:%08x: %x\n",npc->PC, addr,data);
  }
  else if(size == 1){
    int shift = (addr & 0b11)*8;
    MEM[addr >> 2] = (MEM[addr >> 2] & ~(0xffu << shift)) | ((data & 0xff) << shift);
    printf("%08xWrite to RAM:%08x: %x\n",npc->PC, addr,data);
  }
}

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
    MEM[i] = words[i];
  }

  delete[] buffer;
  std::cout << "Loaded " << word_count << " instructions from " << filename << std::endl;
}




static void single_cycle() {
  npc->clk = 0;
  npc->eval();
  npc->imem_rdata = MEM[npc->imem_addr >> 2];
  printf("PC: %08x inst %08x\n", npc->PC, npc->imem_rdata);
  npc->clk = 1;
  npc->eval();
}

static void reset(int n) {
  npc->rst = 1;
  while (n -- > 0) single_cycle();
  npc->rst = 0;
}

int main() {
  nvboard_bind_all_pins(npc);
  nvboard_init();
  load_program("test/mem.bin");
  reset(10);
  while(1) {
    nvboard_update();
    single_cycle();
    cycle++;
    if(cycle == 7000) break;
    if(simulation_should_exit) {
      std::cout << "Simulation exited at cycle " << cycle << std::endl;
      break;
    }
  }
}
