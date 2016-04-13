#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <sys/sysinfo.h>


int worker(void* arg) {
    int core_id = *(int*)arg;
    unsigned int tsc, tsc2, i;
    unsigned long start, end;

    asm volatile("mrrc p15, 0, %Q0, %R0, c9" : "=r" (start));
    for (i=0; i < 1000; i++) {
    };
    asm volatile("mrrc p15, 0, %Q0, %R0, c9" : "=r" (end));
    printf("[%d] end: %12u  delta64: %6u\n", core_id, end, end-start);
    return 0;
}


int main(int argc, char ** argv) {
    int cpu;
    for(cpu = 0; cpu < get_nprocs() ; cpu++ )
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);
        sched_setaffinity(pthread_self(), sizeof(cpu_set_t), &cpuset);
        worker(&cpu);
    }

    return 0;
}
