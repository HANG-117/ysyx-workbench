#include "common.hpp"
#include <fstream>
#include <vector>
#include <elf.h>
#include <cstring>

#define MAX_FTRACE_SYM 4096
static ftrace_sym_t ftrace_table[MAX_FTRACE_SYM];
static int ftrace_nr = 0;
static int ftrace_depth = 0;
void mtrace_write(uint32_t addr, uint32_t data) {
    if (MTRACE && addr >= MTRACE_START && addr <= MTRACE_END) {
        std::cout << "Memory Write Trace - Address: 0x" << std::hex << addr 
                  << ", Data: 0x" << std::hex << data << std::endl;
    }
}

void mtrace_read(uint32_t addr, uint32_t data) {
    if (MTRACE && addr >= MTRACE_START && addr <= MTRACE_END) {
        std::cout << "Memory Read Trace - Address: 0x" << std::hex << addr 
                  << ", Data: 0x" << std::hex << data << std::endl;
    }
}

void ftrace_init(std::string elf_file)
{
    std::ifstream file(elf_file,std::ios::binary);
    if(!file.is_open()){
        std::cout<<"open elf failed\n";
        return;
    }

    Elf32_Ehdr ehdr;
    file.read((char*)&ehdr,sizeof(ehdr));

    std::vector<Elf32_Shdr> shdr(ehdr.e_shnum);
    file.seekg(ehdr.e_shoff);
    file.read((char*)shdr.data(),ehdr.e_shnum*sizeof(Elf32_Shdr));

    Elf32_Shdr symtab{},strtab{};
    for(int i=0;i<ehdr.e_shnum;i++){
        if(shdr[i].sh_type==SHT_SYMTAB){
            symtab=shdr[i];
            strtab=shdr[shdr[i].sh_link];
            break;
        }
    }

    char *str_buf=new char[strtab.sh_size];
    file.seekg(strtab.sh_offset);
    file.read(str_buf,strtab.sh_size);

    int num=symtab.sh_size/sizeof(Elf32_Sym);

    for(int i=0;i<num;i++){
        Elf32_Sym sym;
        file.seekg(symtab.sh_offset+i*sizeof(Elf32_Sym));
        file.read((char*)&sym,sizeof(sym));

        if(ELF32_ST_TYPE(sym.st_info)==STT_FUNC && sym.st_size){
            if(ftrace_nr>=MAX_FTRACE_SYM) break;

            ftrace_table[ftrace_nr].addr=sym.st_value;
            ftrace_table[ftrace_nr].size=sym.st_size;
            ftrace_table[ftrace_nr].name=strdup(str_buf+sym.st_name);
            ftrace_nr++;
        }
    }

    delete []str_buf;
    file.close();

    std::cout<<"ftrace load "<<ftrace_nr<<" functions\n";
}

std::string ftrace_lookup(uint32_t addr)
{
    for(int i=0;i<ftrace_nr;i++){
        uint32_t start=ftrace_table[i].addr;
        uint32_t end=start+ftrace_table[i].size;
        if(addr>=start && addr<end)
            return ftrace_table[i].name;
    }
    return "";
}

void ftrace_call(uint32_t call_addr,uint32_t target_addr)
{
    std::string name=ftrace_lookup(target_addr);
    if(name.empty()) name="<unknown>";

    printf("0x%08x: ",call_addr);
    for(int i=0;i<ftrace_depth;i++)
        printf("  ");

    printf("call [%s]\n",name.c_str());
    ftrace_depth++;
}
void ftrace_return(uint32_t pc,uint32_t ret_target)
{
    if(ftrace_depth>0)
        ftrace_depth--;

    std::string name=ftrace_lookup(ret_target);
    if(name.empty()) name="<unknown>";

    printf("0x%08x: ",pc);
    for(int i=0;i<ftrace_depth;i++)
        printf("  ");

    printf("ret [%s]\n",name.c_str());
}

void ftrace_check(uint32_t pc,uint32_t inst,uint32_t ret_addr)
{
    uint32_t opcode = inst & 0x7f;
    // jal
    if(opcode == 0x6f)
    {
        uint32_t rd = (inst >> 7) & 0x1f;
        if(rd == 1)   // jal x1,target
        {
            int32_t imm =
                ((inst >> 31) << 20) |
                (((inst >> 12)&0xff)<<12) |
                (((inst >> 20)&1)<<11) |
                (((inst >> 21)&0x3ff)<<1);

            uint32_t target = pc + imm;

            ftrace_call(pc,target);
        }
    }
    // jalr
    else if(opcode == 0x67)
    {
        uint32_t rd = (inst >> 7)&0x1f;
        uint32_t rs1 = (inst >> 15)&0x1f;
        int32_t imm = (int32_t)inst >> 20;
        // ret:
        // jalr x0,0(x1)
        if(rd==0 && rs1==1 && imm==0)
        {
            ftrace_return(pc,ret_addr);
        }
    }
}