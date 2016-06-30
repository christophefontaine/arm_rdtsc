#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <fcntl.h>

typedef uint64_t dma_addr_t;
#include "../kmod/arm_asm.h"

int main(int argc, char ** argv) {
    printf("Cache Test:\n");
    int fd = open("/dev/rdtsc", O_RDWR);
    if(fd) {
        ioctl(fd, FLUSH_CACHE);
        close(fd);
    } else {
	printf("no dev");
    }

    return 0;
}
