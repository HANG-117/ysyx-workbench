#include <nvboard.h>
#include <VNPC.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h> // 用于高精度 RTC 时间获取
#include "common.hpp"

#define MEM_SIZE_WORDS (1 << 28) // 256MB 内存
#define MEM_BASE       0x80000000U 

// MMIO 地址映射
#define SERIAL_PORT    0x10000000U  // 串口地址 (SoC 兼容)
#define RTC_ADDR_LOW   0xa0000048U  // RTC 时间低 32 位
#define RTC_ADDR_HIGH  0xa000004cU  // RTC 时间高 32 位


#define MAX_CYCLES     100000

static uint32_t MEM[MEM_SIZE_WORDS];
static TOP_NAME *npc = new TOP_NAME;
int cycle = 0;

static bool simulation_should_exit = false;
extern "C" void sim_exit(int pc, int a0) {
    simulation_should_exit = true;
    if (a0 != 0) {
        // 红色输出
        std::cout << "\033[31m" 
                  << "Simulation exited with error code: " << a0 
                  << "\033[0m" << std::endl;
    } else {
        // 绿色输出
        std::cout << "\033[32m" 
                  << "HIT GOOD TRAP at pc = 0x" << std::hex << pc 
                  << "\033[0m" << std::endl;
    }
}

// 辅助函数：获取系统初始启动后的微秒数
static uint64_t get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

static uint64_t boot_time = 0;

// ------------------- MMIO 内存写 -------------------
extern "C" void pmem_write(int waddr, int wdata, int wmask) {
    uint32_t addr = (uint32_t)waddr;
  if (addr == 0x10000000U) {
      char ch = (char)(wdata & 0xFF);
      putchar(ch);
      return;
  }

    // 2. 物理内存写入 (RAM)
    if (addr < MEM_BASE || addr >= (MEM_BASE + (MEM_SIZE_WORDS * 4))) {
        // 忽略非法地址写操作，防止崩溃
        return;
    }

    uint32_t mem_idx = (addr - MEM_BASE) >> 2;

    if (wmask == 0b0001) {
        MEM[mem_idx] = (MEM[mem_idx] & 0xFFFFFF00) | (wdata & 0x000000FF);
    } else if (wmask == 0b0010) {
        MEM[mem_idx] = (MEM[mem_idx] & 0xFFFF00FF) | (wdata & 0x0000FF00);
    } else if (wmask == 0b0100) {
        MEM[mem_idx] = (MEM[mem_idx] & 0xFF00FFFF) | (wdata & 0x00FF0000);
    } else if (wmask == 0b1000) {
        MEM[mem_idx] = (MEM[mem_idx] & 0x00FFFFFF) | (wdata & 0xFF000000);
    } else if (wmask == 0b0011) {
        MEM[mem_idx] = (MEM[mem_idx] & 0xFFFF0000) | (wdata & 0x0000FFFF);
    } else if (wmask == 0b1100) {
        MEM[mem_idx] = (MEM[mem_idx] & 0x0000FFFF) | (wdata & 0xFFFF0000);
    } else if (wmask == 0b1111) {
        MEM[mem_idx] = wdata;
    } else {
        // 如果 mask 传过来有偏移（如 0b0010, 0b0100, 0b1000），按 byte 写入
        uint32_t mask_val = 0;
        if (wmask & 0x1) mask_val |= 0x000000FF;
        if (wmask & 0x2) mask_val |= 0x0000FF00;
        if (wmask & 0x4) mask_val |= 0x00FF0000;
        if (wmask & 0x8) mask_val |= 0xFF000000;
        MEM[mem_idx] = (MEM[mem_idx] & ~mask_val) | (wdata & mask_val);
    }
}

// ------------------- MMIO 内存读 -------------------
extern "C" int pmem_read(int raddr) {
    uint32_t addr = (uint32_t)raddr;

    // 1. 实时时钟 RTC 读 (MMIO)
    if (addr == RTC_ADDR_LOW) {
        uint64_t now = get_time_us() - boot_time;
        return (uint32_t)(now & 0xFFFFFFFF); // 低 32 位 (单位: 微秒)
    }
    if (addr == RTC_ADDR_HIGH) {
        uint64_t now = get_time_us() - boot_time;
        return (uint32_t)(now >> 32);         // 高 32 位
    }

    // 2. 物理内存读取 (RAM)
    if (addr < MEM_BASE || addr >= (MEM_BASE + (MEM_SIZE_WORDS * 4))) {
        return 0; // 越界/未映射区域返回 0，防止组合逻辑 Glitch 导致崩溃
    }

    uint32_t mem_idx = (addr - MEM_BASE) >> 2;
    return MEM[mem_idx];
}

void nvboard_bind_all_pins(TOP_NAME* top);

void load_program(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        exit(1);
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];
    file.read(buffer, size);
    file.close();

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
    npc->clk = 1;
    npc->eval();
    if(simulation_should_exit) return;
    std::cout<< "Cycle: " << std::dec << cycle << ", PC: 0x" << std::hex << npc->NPC__DOT__ifu_inst__DOT__pc_reg_inst__DOT__pc 
        << ", Instruction: 0x" << std::hex << npc->NPC__DOT__inst 
        << ", Disassembly: " << disassemble_rv32e(npc->NPC__DOT__inst, npc->NPC__DOT__ifu_inst__DOT__pc_reg_inst__DOT__pc) 
        << std::endl;
    cycle++;
}

static void reset(int n) {
    npc->rst = 1;
    while (n-- > 0) single_cycle();
    npc->rst = 0;
}

int main(int argc, char** argv) {
    using namespace std;
    boot_time = get_time_us(); // 记录系统启动基准时间
    const char* img_file = "test/mem.bin";
    if (argc >= 2) {
        img_file = argv[1];
    }
    load_program(img_file);

    reset(10);

    while (1) {
        char choice;
        cin >> choice;
        switch (choice) {
            case 's':
                cout << "请输入需要前进的周期数"<<endl;
                int num;
                cin >> num;
                for(int i = 0;i<num;i++){
                    single_cycle();
                    if(simulation_should_exit) break;
                }
                break;
            case 'm':
                std::cout << "Enter address to read: ";
                uint32_t addr;
                cin >> std::hex >> addr;
                for(int i = -4; i < 5; i++) {
                    cout << "0x" << std::hex << (addr + i * 4) << ": 0x" 
                         << std::hex << pmem_read(addr + i * 4) << std::endl;
                }
                break;
            case 'c':
                while (true) {
                    single_cycle();
                    if(simulation_should_exit) break;
                }
                break;
            case 'p':
                cout<< "=======当前寄存器信息=======" << endl;
                for(int i = 0; i < 32; i++) {
                    cout << "x" << std::dec << i << ": 0x" 
                         << std::hex << npc->NPC__DOT__regfile_inst__DOT__rf[i];
                    if(i % 4 == 3) cout << endl;
                    else cout << "\t";
                }
                break;
            case 'q':
                return 0;
            default:
                std::cout << "Unknown command. Use 's' to step, 'q' to quit." << std::endl;
        }
        if(cycle >= MAX_CYCLES) break;
        if(simulation_should_exit) break;
    }
    return 0;
}