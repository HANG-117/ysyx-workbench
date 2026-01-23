#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

static Vtop* top = NULL;
static VerilatedVcdC* tfp = NULL;
static vluint64_t main_time = 0; 

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true); 
    
    top = new Vtop;
    
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99); 
    tfp->open("wave.vcd");  
    
    top->a = 0;
    top->b = 0;
    top->eval();
    tfp->dump(main_time);
    main_time += 5;
    
    while(1) {
        top->a = rand() & 1;
        top->b = rand() & 1;
        top->eval();
        tfp->dump(main_time);
        main_time += 5;  
        
        printf("a=%d b=%d f=%d\n", top->a, top->b, top->f);
        assert(top->f == ((top->a & ~top->b) | (~top->a & top->b)));
        
        static int cycle_count = 0;
        cycle_count++;
        if (cycle_count >= 10000) {
            printf("已完成10000个随机测试周期\n");
            break;
        }
    }
    
    top->eval();
    tfp->dump(main_time);
    
    tfp->close();
    delete tfp;
    delete top;
    
    printf("仿真完成，波形文件 wave.vcd 已生成\n");
    return 0;
}