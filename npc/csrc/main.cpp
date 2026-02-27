#include <nvboard.h>
#include <VNPC.h>
#include <iostream>
#include <fstream>
#include <sstream>
#define MEM_SIZE_WORDS (1 << 22)
#define MEM_BASE 0x80000000 
static uint32_t MEM[MEM_SIZE_WORDS];
static TOP_NAME *npc = new TOP_NAME;
int cycle = 0;

static bool simulation_should_exit = false;
extern "C" void sim_exit(){
  simulation_should_exit = true;
}
extern "C" uint32_t pmem_read(uint32_t addr) {
  uint16_t offset = addr - MEM_BASE;
  if (offset >= MEM_SIZE_WORDS * 4) {
    printf("Error: Physical address %08x out of bound!\n", addr);
    exit(1);
  }
  return MEM[offset >> 2];
}
extern "C" void pmem_write(uint32_t addr, uint32_t data,int size) {
  int offset = addr - MEM_BASE;
  if(size == 4){
    MEM[offset >> 2] = data;
    printf("%08xWrite to RAM:%08x: %x\n",npc->PC, addr,data);
  }
  else if(size == 1){
    int shift = (offset & 0b11)*8;
    MEM[offset >> 2] = (MEM[offset >> 2] & ~(0xffu << shift)) | ((data & 0xff) << shift);
    printf("%08xWrite to RAM:%08x: %x\n",npc->PC, addr+MEM_BASE,data);
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
  npc->imem_rdata = pmem_read(npc->PC);
  printf("PC: %08x inst %08x\n", npc->PC, npc->imem_rdata);
  npc->clk = 1;
  npc->eval();
}

static void reset(int n) {
  npc->rst = 1;
  while (n -- > 0) single_cycle();
  npc->rst = 0;
}

int main(int argc, char** argv) {
  nvboard_bind_all_pins(npc);
  nvboard_init();
  npc->PC=0x80000000;
  const char* img_file = "test/mem.bin";
  if(argc >=2) {
    img_file = argv[1];
  };
  load_program(img_file);
  printf("PC:%08x\n",npc->imem_addr);
  reset(10);
  while(1) {
    nvboard_update();
    single_cycle();
    cycle++;
    if(simulation_should_exit) {
      std::cout << "Simulation exited at cycle " << cycle << std::endl;
      break;
    }
  }
}
