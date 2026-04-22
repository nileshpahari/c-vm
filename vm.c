#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t u16;

// memory structure define
const u16 PC_START = 0x3000;
u16 mem[UINT16_MAX + 1] = {0};

// VM execution
bool running = true;

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
  reg[DR1(i)] = reg[SR1(i)] & (FIMM(i) ? SEXTMM(i) : reg[SR2(i)]);
  uf(DR1(i));
}

static inline void not(u16 i) {
  reg[DR1(i)] = ~reg[SR1(i)];
  uf(DR1(i));
}

#define POFF11(i) sext((i) & 0x7FF, 11)
#define POFF9(i) sext((i) & 0x1FF, 9)
#define POFF6(i) sext((i) & 0x3F, 6)

#define FL(i) ((i >> 11) & 1)

#define FCND(i) ((i >> 9) & 0x7)

// Memory Loading
static inline void ld(u16 i) {
  reg[DR1(i)] = mr(reg[RPC] + POFF9(i));
  uf(DR1(i));
}

static inline void ldi(u16 i) {
  reg[DR1(i)] = mr(mr(reg[RPC] + POFF9(i)));
  uf(DR1(i));
}

static inline void ldr(u16 i) {
  reg[DR1(i)] = mr(reg[SR1(i)] + POFF6(i));
  uf(DR1(i));
}

static inline void lea(u16 i) {
  reg[DR1(i)] = reg[RPC] + POFF9(i);
  uf(DR1(i));
}

// Memory Storage
static inline void st(u16 i) { mw(reg[RPC] + POFF9(i), reg[DR1(i)]); }

static inline void sti(u16 i) { mw(mr(reg[RPC] + POFF9(i)), reg[DR1(i)]); }

static inline void str(u16 i) { mw(reg[SR1(i)] + POFF6(i), reg[DR1(i)]); }

// Control Flow
static inline void jmp(u16 i) { reg[RPC] = reg[SR1(i)]; }

static inline void jsr(u16 i) {
  reg[R7] = reg[RPC];
  reg[RPC] = (FL(i)) ? (reg[RPC] + POFF11(i)) : reg[SR1(i)];
}

static inline void br(u16 i) { // OPCODE [n z p] [offset9]
  if (reg[RCND] & FCND(i)) {
    reg[RPC] = reg[RPC] + POFF9(i);
  }
}

// Trap
//
#define TRP(i) ((i) & 0xFF)

enum { trp_offset = 0x20 };
typedef void (*trp_ex_f)();

// | Function  | Vector | Description                              |
// | `tgetc`   | 0x20   | Read a character into R0                 |
// | `tout`    | 0x21   | Print character in R0                    |
// | `tputs`   | 0x22   | Print string from memory (address in R0) |
// | `tin`     | 0x23   | Read + echo character                    |
// | `tputsp`  | 0x24   | (Not implemented)                        |
// | `thalt`   | 0x25   | Stop execution                           |
// | `tinu16`  | 0x26   | Read 16-bit integer                      |
// | `toutu16` | 0x27   | Print 16-bit integer                     |
//

static inline void tgetc() { reg[R0] = getchar(); }

static inline void tout() { fprintf(stdout, "%c", (char)reg[R0]); }

static inline void tputs() {
  u16 *p = mem + reg[R0]; //  //
  while (*p) {
    fprintf(stdout, "%c", (char)*p);
    p++;
  }
}

static inline void tin() {
  reg[R0] = getchar();
  fprintf(stdout, "%c", (char)reg[R0]);
}

static inline void thalt() {
  running = false;
}

static inline void tinu16() { fscanf(stdin, "%hu", &reg[R0]); }

static inline void tputsp() {}

static inline void toutu16() { fprintf(stdout, "%hu\n", reg[R0]); }

trp_ex_f trp_ex[8] = {tgetc, tout, tputs, tin, tputsp, thalt, tinu16, toutu16};

static inline void trap(u16 i) {
  u16 idx = TRP(i) - trp_offset;
  if (idx < 8) {
    trp_ex[idx]();
  } else {
    printf("Invalid TRAP: %x\n", TRP(i));
    running = false;
  }
}

static inline void res(u16 i) {

}

static inline void rti(u16 i) {

}

// Function pointer array for instruction execution // TODO: impl `res` and
// `rti`
op_ex_f op_ex[NOPS] = {
    br, add, ld, st, jsr, and, ldr, str, rti, not, ldi, sti, jmp, res, lea, trap
};

void start(u16 offset) {
  // Initialize RPC to program start
  reg[RPC] = PC_START + offset;

  while (running) {
    // FETCH
    u16 i = mr(reg[RPC]++);
    // DECODE + EXECUTE
    op_ex[OPC(i)](i);
	printf("PC: %04x INST: %04x\n", reg[RPC]-1, i);
  }
}

void id_img(char *fname, u16 offset) {
  char *check = strstr(fname, ".obj");
  if (check == NULL) {
    printf("\n The provided file is not .obj, Pls pass a .obj file\n");
    exit(1);
  }

  FILE *in = fopen(fname, "rb");
  if (in == NULL) {
    fprintf(stderr, "cannot open file %s.\n", fname);
    exit(1);
  }

  u16 *p = mem + PC_START + offset;
  fread(p, sizeof(u16), (UINT16_MAX - PC_START), in);
  fclose(in);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <program.obj>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Load the program into memory
  id_img(argv[1], 0x0);

  // Start execution
  start(0x0);

  return 0;
}
