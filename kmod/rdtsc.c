#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static void enable_counters(void* data)
{
    (void)data;
    asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(1)); /* Enable user mode access */
    asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(1 | 16)); 
    asm volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(0x8000000f));
}

static int __init rdtsc_init(void)
{
    u32 tmp;
    on_each_cpu(enable_counters, NULL, 1);
    printk("User land access to PMU - Enabled\n");
    return 0;
}

static int __exit rdtsc_exit(void)
{
    return 0;
}

module_init(rdtsc_init);
module_exit(rdtsc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christophe Fontaine");
MODULE_DESCRIPTION("Enable PMU for userland access to a rdtsc like function on ARM");
