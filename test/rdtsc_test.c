#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sched.h>

typedef uint64_t dma_addr_t;
#include "../kmod/arm_asm.h"

int worker(void* arg) {
    int i = 0;
    int core_id = *(int*)arg;
    uint64_t start, end;
    start = cpu_time_now();
    for (i=0; i < 1000; i++) {
    };
    end = cpu_time_now();
    printf("[%d] end: %"PRIu64"  delta64: %"PRIu64"\n", core_id, end, end-start);
    return 0;
}

int main(int argc, char ** argv) {
    int cpu;
    printf("get_nprocs = %u get_nprocs_conf = %u\n", get_nprocs(), get_nprocs_conf());

    for(cpu = 0; cpu < get_nprocs() ; cpu++ )
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);
        sched_setaffinity(pthread_self(), sizeof(cpu_set_t), &cpuset);
        printf("PMUSERENR = %x\n", _pmuseren());
        if(_pmuseren())
          worker(&cpu);
    }
    printf("Now on sleeping cores:\n");
    for(; cpu < get_nprocs_conf() ; cpu++ )
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);
        sched_setaffinity(pthread_self(), sizeof(cpu_set_t), &cpuset);
        printf("PMUSERENR = %x\n", _pmuseren());
        if(_pmuseren())
            worker(&cpu);
    }

    return 0;
}
