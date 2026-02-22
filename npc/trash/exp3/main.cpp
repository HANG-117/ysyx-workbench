#include <Vtop.h>
#include <nvboard.h>

#include <stdint.h>
static TOP_NAME dut;
void nvboard_bind_all_pins(TOP_NAME* top);

int main() {
	nvboard_bind_all_pins(&dut);
	nvboard_init();
    Vtop* top = new Vtop;
	while(1){
		dut.eval();
		nvboard_update();
	}
	nvboard_quit();
    return 0;
}
