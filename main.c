#include <stdint.h>
#include <unitypes.h>

// memory structure define
uint16_t PC_START = 0x3000;
uint16_t mem[UINT16_MAX + 1] = {0};

// mem helper fn
static inline uint16_t mr(uint16_t address) { return mem[address]; }

static inline void mw(uint16_t address, uint16_t val) { mem[address] = val; }

// registers define
enum regist { R0 = 0, R1, R2, R3, R4, R5, R6, R7, RPC, RCND, RCNT };
uint16_t reg[RCNT] = {0};

// IX system
#define OPC(i) ((i) >> 12)

#define NOPS (16) // Total possible instructions
typedef void (*op_ex_f)(uint16_t instruction);

enum flags { FP = 1 << 0, FZ = 1 << 1, FN = 1 << 2 };

static inline void uf(enum regist r) {
  if (reg[r] == 0)
    reg[RCND] = FZ;
  else if (reg[r] >> 15)
    reg[RCND] = FN;
  else
    reg[RCND] = FP;
}

#define DR1(i) ((i >> 9) & 0x7) // destination reg
#define SR1(i) ((i >> 6) & 0x7) // src reg 1
#define SR2(i) ((i) & 0x7) // src reg 2
#define FIMM(i) ((i >> 5) & 1) // immediate mode flag
#define IMM(i) ((i) & 0x1F)    // extract 5-bit immediate

static inline uint16_t sext(uint16_t n, int b) {
  return ((n >> (b - 1)) & 1) ? (n | (0xFFFF << b)) : n;
}

#define SEXTMM(i) sext(IMM(i), 5) // sign-extend IMM5

// IXs
static inline void add(uint16_t i) {
	reg[DR1(i)] = reg[SR1(i)] + (FIMM(i) ? SEXTMM(i) : reg[SR2(i)]);
	uf(DR1(i));
}

static inline void and(uint16_t i) {
	reg[DR1(i)] = reg[SR1(i)] + (FIMM(i) ? SEXTMM(i) : reg[SR2(i)]);
	uf(DR1(i));
}



// Function pointer array for instruction execution
// op_ex_f op_ex[NOPS] = {br,  add, ld,  st,  jsr, and, ldr, str,
//                        rti, not, ldi, sti, jmp, res, lea, trap};
//

int main() {

}

