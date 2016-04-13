#include <stdio.h>

int main(int argc, char ** argv) {
    unsigned int tsc, tsc2, i;
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r" (tsc));
    for (i=0; i < 1000; i++) {
    };
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r" (tsc2));
    printf("delta = %d\n", tsc2-tsc);
    return 0;
}
