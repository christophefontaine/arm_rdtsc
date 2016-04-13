#include <stdio.h>

int main(int argc, char ** argv) {
    unsigned int tsc, tsc2, i;
    unsigned long start, end;
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r" (tsc));
    asm volatile("mrrc p15, 0, %Q0, %R0, c9" : "=r" (start));
    for (i=0; i < 1000; i++) {
    };
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r" (tsc2));
    asm volatile("mrrc p15, 0, %Q0, %R0, c9" : "=r" (end));

    printf("delta = %u\n", tsc2-tsc);
    printf("delta64 = %u\n", end-start);

    return 0;
}
