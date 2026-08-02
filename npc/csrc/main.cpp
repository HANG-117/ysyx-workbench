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
extern "C" void pmem_write(int waddr, int wdata,int wmask) {
  waddr = (uint32_t)((uint32_t)waddr - (uint32_t)MEM_BASE) >> 2;
  if(wmask == 0b0001){
    MEM[waddr] = (MEM[waddr] & 0xFFFFFF00) | (wdata & 0x000000FF);
  }
  else if(wmask == 0b0010){
    MEM[waddr] = (MEM[waddr] & 0xFFFF00FF) | (wdata & 0x0000FF00);
  }
  else if(wmask == 0b0100){
    MEM[waddr] = (MEM[waddr] & 0xFF00FFFF) | (wdata & 0x00FF0000);
  }
  else if(wmask == 0b1000){
    MEM[waddr] = (MEM[waddr] & 0x00FFFFFF) | (wdata & 0xFF000000);
  }
  else if(wmask == 0b0011){
    MEM[waddr] = (MEM[waddr] & 0xFFFF0000) | (wdata & 0x0000FFFF);
  }
  else if(wmask == 0b1100){
    MEM[waddr] = (MEM[waddr] & 0x0000FFFF) | (wdata & 0xFFFF0000);
  }
  else if(wmask == 0b1111){
    MEM[waddr] = wdata;
  }
  else{
    std::cerr << "Error: Invalid write mask " << wmask << std::endl;
    exit(1);
  }
}
extern "C" int pmem_read(int raddr) {
  if ((uint32_t)raddr < MEM_BASE) {
    return 0; 
  }
  raddr = (uint32_t)((uint32_t)raddr - (uint32_t)MEM_BASE) >> 2;
  return MEM[raddr];
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
      const char *COLOR_GREEN = "\033[1;32m"; // 绿色 (成功)
      const char *COLOR_RED   = "\033[1;31m"; // 红色 (失败)
      const char *COLOR_RESET = "\033[0m";  // 重置颜色 (非常重要，否则后面的输出也会变色)

      if(npc->a0 == 0){
          // 成功：绿色显示
          printf("%shit good trap at %08x%s\n", COLOR_GREEN, npc->PC, COLOR_RESET);
      }
      else{
          // 失败：红色显示
          printf("%shit bad trap at %08x%s\n", COLOR_RED, npc->PC, COLOR_RESET);
      }
      break;
      }
  }
  
}
