#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

class HivekMem {
public:
    uint8_t* mem;

    void initialize(int n) {
        mem = new uint8_t[n];
    }

    void read_bin(char* path) {
        int c;
        auto f = fopen(path, "rb");
        auto i = 0;
        while (true) {
            c = fgetc(f);
            if (c == EOF) {
                break;
            }
            mem[i] = c;
            i = i + 1;
        }
    }

    void dump(int n) {
        for (auto i = 0; i < n; ++i) {
            printf("%02x\n", mem[i]);
        }
    }

    uint64_t read32(uint64_t addr) {
        uint64_t v = 0;
        uint64_t k;
        v = (v << 8) | mem[addr + 0];
        v = (v << 8) | mem[addr + 1];
        v = (v << 8) | mem[addr + 2];
        v = (v << 8) | mem[addr + 3];
        return v;
    }

    void write32(uint64_t addr, uint32_t value) {
        mem[addr + 0] = (value >> 24) & 0x0ff;
        mem[addr + 1] = (value >> 16) & 0x0ff;
        mem[addr + 2] = (value >> 8) & 0x0ff;
        mem[addr + 3] = (value >> 0) & 0x0ff;
    }


};

class HivekVM {
public:
    uint64_t ip;
    uint64_t* regs;
    HivekMem mem;
    bool run_flag;

    void initialize() {
        ip = 0;
        mem.initialize(8 * 1024 * 1024);
        regs = new uint64_t[32];
        run_flag = true;
    }

    void print_byte(uint8_t byte) {
        printf("%x\n", byte);
    }

    void run() {
        while (run_flag) {
            execute_instruction();
            printf("\nip = %lx\r", ip);
        }
    }

    void execute_instruction() {
        auto inst = mem.read32(ip);
        run_flag = false;
        if (is_16bit_instruction(inst)) {
            exec_16bit(inst);
        }
        else if (is_24bit_instruction(inst)) {
            exec_24bit(inst);
        }
        else if (is_32bit_instruction(inst)) {
            exec_32bit(inst);
            run_flag = true;
        }
    }

    bool is_16bit_instruction(uint32_t inst) {
        return (inst >> 30) & 0x3 == 0 || (inst >> 30) & 0x3 == 1 || (inst >> 30) & 0x3 == 2;
    }

    bool is_24bit_instruction(uint32_t inst) {
        return (inst >> 24) == 6;
    }

    bool is_32bit_instruction(uint32_t inst) {
        return (inst >> 29) & 0x7 == 7;
    }

    void exec_16bit(uint32_t inst) {
        ip += 2;
    }

    void exec_24bit(uint32_t inst) {
        ip += 3;
    }

    void exec_32bit(uint32_t inst) {
        auto OPCODE_32_SH = 23;
        auto OPCODE_32_MASK = 0x3F;
        auto FUNCT_32_SH = 0;
        auto FUNCT_32_MASK = 0x7f;
        auto RA_32_MASK = 0x1f;
        auto RB_32_MASK = 0x1f;
        auto RC_32_MASK = 0x1f;
        auto RA_32_SH = 18;
        auto RB_32_SH = 13;
        auto RC_32_SH = 8;
        auto FUNCT_32_ADD_64 = 0;
        auto FUNCT_32_SUB_64 = 1;
        auto IMMD13_32_SH = 0;
        auto IMMD13_32_MASK = 0x1FFF;
        auto OPCODE_32_ADDI_64 = 1;
        auto opcode = (inst >> OPCODE_32_SH) & OPCODE_32_MASK;
        auto funct = (inst >> FUNCT_32_SH) & FUNCT_32_MASK;
        auto ra = (inst >> RA_32_SH) & RA_32_MASK;
        auto rb = (inst >> RB_32_SH) & RB_32_MASK;
        auto rc = (inst >> RC_32_SH) & RC_32_MASK;
        auto immd = (inst >> IMMD13_32_SH) & IMMD13_32_MASK;
        printf("op = %i\n", opcode);
        printf("funct = %i\n", funct);
        printf("ra = %i\n", ra);
        printf("rb = %i\n", rb);
        printf("rc = %i\n", rc);
        printf("immd = %i\n", immd);
        if (opcode == 0) {
            if (funct == FUNCT_32_ADD_64) {
                regs[rc] = regs[ra] + regs[rb];
                ip += 4;
            }
            else if (funct == FUNCT_32_SUB_64) {
                regs[rc] = regs[ra] - regs[rb];
                ip += 4;
            }
        }
        else if (opcode == OPCODE_32_ADDI_64) {
            regs[rb] = regs[ra] + immd;
            ip += 4;
        }
        else {
            printf("none\n");
        }
    }

    void read_bin(char* path) {
        mem.read_bin(path);
    }

    void dump() {
        auto i = 0;
        while (i < 32) {
            printf("regs[%i] = 0x%016lx\n", i, regs[i]);
            i = i + 1;
        }
        mem.dump(10);
    }


};

int main() {
    auto v = HivekVM();
    v.initialize();
    v.read_bin("init.bin");
    v.run();
    v.dump();
}

