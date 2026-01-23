all: 
	@make sim

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@verilator -Wall --cc vsrc/top.v --exe csrc/main.cpp --trace --build
	@make -C obj_dir -f Vtop.mk
	@./obj_dir/Vtop
clean:
	@rm -rf obj_dir wave.vcd
include ../Makefile
