#include <stdio.h>
#include <stdint.h>

uint16_t program[] = {
    0xF026,    // TRAP tinu16  (Read input to R0)
    0x1220,    // ADD R1,R0,x0 (Move R0 to R1)
    0xF026,    // TRAP tinu16  (Read second input to R0)
    0x1240,    // ADD R1,R1,R0 (R1 = R1 + R0)
    0x1060,    // ADD R0,R1,x0 (Move R1 to R0)
    0xF027,    // TRAP toutu16 (Print result)
    0xF025     // HALT         (Stop execution)
};

int main(void) {
    char *outf = "sum.obj";
    FILE *f = fopen(outf, "wb");

    if (!f) {
        fprintf(stderr, "Cannot open file %s\n", outf);
        return 1;
    }

    size_t count = sizeof(program) / sizeof(uint16_t);
    fwrite(program, sizeof(uint16_t), count, f);

    printf("Wrote %zu instructions to %s\n", count, outf);
    fclose(f);
    return 0;
}
