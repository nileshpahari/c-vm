#include <inttypes.h>
#include <stdint.h>

typedef uint16_t u16;

// memory structure define
const u16 PC_START = 0x3000;
u16 mem[UINT16_MAX + 1] = {0};

// mem helper fn
static inline u16 mr(u16 address) { return mem[address]; }

static inline void mw(u16 address, u16 val) { mem[address] = val; }

// registers define
enum regist { R0 = 0, R1, R2, R3, R4, R5, R6, R7, RPC, RCND, RCNT };
u16 reg[RCNT] = {0};

// IX system
#define OPC(i) ((i) >> 12)

#define NOPS (16) // Total possible instructions
typedef void (*op_ex_f)(u16 instruction);

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
#define SR2(i) ((i) & 0x7)      // src reg 2
#define FIMM(i) ((i >> 5) & 1)  // immediate mode flag
#define IMM(i) ((i) & 0x1F)     // extract 5-bit immediate

static inline u16 sext(u16 n, int b) {
  return ((n >> (b - 1)) & 1) ? (n | (0xFFFF << b)) : n;
}

#define SEXTMM(i) sext(IMM(i), 5) // sign-extend IMM5

// IXs
//

// Mathematical Operations
static inline void add(u16 i) {
  reg[DR1(i)] = reg[SR1(i)] + (FIMM(i) ? SEXTMM(i) : reg[SR2(i)]);
  uf(DR1(i));
}

static inline void and(u16 i) {
  reg[DR1(i)] = reg[SR1(i)] + (FIMM(i) ? SEXTMM(i) : reg[SR2(i)]);
  uf(DR1(i));
}

static inline void not(u16 i) {
	reg[DR1(i)] = ~reg[SR1(i)];
	uf(DR1(i));
}

#define POFF11(i) sext((i) & 0x7FF, 11)
#define POFF9(i) sext((i) & 0x1FF, 9)
#define POFF6(i) sext((i) & 0x3F, 6)

#define FL(i) ((i >> 10) & 1)

// Memory Loading
static inline void ld(u16 i) {
  reg[DR1(i)] = mr(reg[RPC] + POFF9(i));
  uf(reg[DR1(i)]);
}

static inline void ldi(u16 i) {
  reg[DR1(i)] = mr(mr(reg[RPC] + POFF9(i)));
  uf(reg[DR1(i)]);
}

static inline void ldr(u16 i) {
	reg[DR1(i)] = mr(reg[SR1(i)] + POFF6(i));
	uf(reg[DR1(i)]);
}

static inline void lea(u16 i) {
	reg[DR1(i)] = reg[RPC] + POFF9(i);
	uf(DR1(i));
}

// Memory Storage
static inline void st(u16 i) {
	mw(reg[RPC] + POFF9(i), reg[DR1(i)]);
}

static inline void sti(u16 i) {
	mw(mr(reg[RPC] + POFF9(i)), reg[DR1(i)]);
}

static inline void str(u16 i) {
	mw(reg[SR1(i)] + POFF6(i), reg[DR1(i)]);
}

// Control Flow
static inline void jmp(u16 i) {
	reg[RPC] = reg[SR1(i)];
}

static inline void jsr(u16 i) {
	reg[R7] = reg[RPC];
	reg[RPC] = (FL(i)) ? (reg[RPC] + POFF11(i)) : reg[SR1(i)];
}

static inline void br(u16 i) {

}

// Function pointer array for instruction execution
op_ex_f op_ex[NOPS] = {br,  add, ld,  st,  jsr, and, ldr, str,
                       rti, not, ldi, sti, jmp, res, lea, trap};


int main() {}
