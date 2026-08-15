#include <iostream>
#include <string>

<<<<<<< HEAD
static bool simulation_should_exit = false;
extern "C" void sim_exit(){
  simulation_should_exit = true;
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
    printf("%08xWrite to RAM:%08x: %x\n",npc->PC, addr,data);
  }
}
extern "C" uint32_t pmem_read(uint32_t addr) {
  uint32_t offset = addr - MEM_BASE;

  if (offset >= MEM_SIZE_WORDS * 4) {
    printf("Error: Physical address %08x out of bound!\n", addr);
    return 0;
  }
  return MEM[offset >> 2];
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
  if(npc->imem_rdata == 0x0000006f) {
    printf("Encountered ecall at PC: %08x\n", npc->PC);
    sim_exit();
  }
  npc->clk = 1;
  npc->eval();
}

static void reset(int n) {
  npc->rst = 1;
  while (n -- > 0) single_cycle();
  npc->rst = 0;
}
=======
#include "common.hpp"
#include "difftest/difftest.hpp"
#include "memory/memory.hpp"
#include "monitor/monitor.hpp"
#include "sim/sim.hpp"
#include "trace/trace.hpp"
>>>>>>> pa3

int main(int argc, char** argv) {
    // ---- 解析命令行：镜像文件（可选），ftrace 需要同名 .elf ----
    const char* img_file = "test/mem.bin";
    std::string elf_file;  // ftrace 符号文件
    if (argc >= 2) {
        img_file = argv[1];
        if (FTRACE) {
            std::string img_str = argv[1];
            size_t dot_pos = img_str.find_last_of('.');
            if (dot_pos != std::string::npos) {
                elf_file = img_str.substr(0, dot_pos) + ".elf";
            } else {
                elf_file = img_str + ".elf";
            }
        }
    }

    // ---- 初始化：内存 -> 仿真器 -> difftest -> 监视器 ----
    Memory memory;  // 物理内存 + MMIO（构造时记录 RTC 基准时间）
    memory.load_image(img_file);

    if (FTRACE && !elf_file.empty()) {
        ftrace_init(elf_file);
    }

    Simulator sim;
    Difftest difftest(sim, memory);
    sim.set_difftest(&difftest);
    sim.reset(10);

    // 复位完成后启用 difftest（复位期间不参与对比）
    if (!difftest.init()) {
        std::cerr << "difftest: init failed, running WITHOUT reference model" << std::endl;
    }

    // ---- 进入交互式监视器 ----
    Monitor monitor(sim, memory);
    return monitor.run();
}
