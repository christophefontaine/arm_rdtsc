#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>

#include <linux/kdev_t.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include "arm_asm.h"

#include <linux/dma-mapping.h>
#include <linux/dma-attrs.h>

#include <linux/mm.h>

static int major_no;
static struct class *my_class;
static struct device *dev;

struct dma_buf_t allocated_va[50];
int allocated_va_count;


static int rdtsc_open(struct inode *inode, 
                       struct file *file)
{
    allocated_va_count = 0;
    return 0;
}

static int rdtsc_release(struct inode *inode, 
                          struct file *file)
{
    int i = 0;
    for(i=0; i<allocated_va_count; i++) {
	dma_free_coherent(dev->parent, allocated_va[i].len, allocated_va[i].vaddr, allocated_va[i].paddr);
    }
    return 0;
}



static long rdtsc_ioctl(
    struct file *file,
    unsigned int ioctl_num,/* The number of the ioctl */
    unsigned long ioctl_param) /* The parameter to it */
{
    switch(ioctl_num) {
	case 0:
	case FLUSH_CACHE:
	    __cpuc_flush_kern_all();
	    __cpuc_flush_user_all();
		break;

	case GET_DMA_BUF:
	    {
	    	struct dma_buf_t *db = (struct dma_buf_t*)ioctl_param;
                gfp_t flag = 0;
                db->vaddr = dma_alloc_coherent(dev->parent, db->len, &db->paddr, flag);
                if(db->vaddr) {
			allocated_va[allocated_va_count].vaddr = db->vaddr;
			allocated_va[allocated_va_count].paddr = db->paddr;
			allocated_va[allocated_va_count].len   = db->len;
			allocated_va_count++;
		}
	    }	
	   break;

	case SET_UNCACHED:
	    {
 		struct dma_buf_t *db = (struct dma_buf_t*)ioctl_param;
		struct thread_info *ti = current_thread_info();
		struct vm_area_struct *vma = find_vma(ti->task->mm, (uint64_t)(uintptr_t)db->vaddr);
		if (vma == NULL)
		    return -EFAULT;
//		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		vma->vm_page_prot = pgprot_dmacoherent(vma->vm_page_prot);
		return remap_pfn_range(vma, vma->vm_start, (uint64_t)(uintptr_t)db->paddr, db->len, vma->vm_page_prot);
	    }
	    break;

	default:
		break;
	}
    
    asm volatile("isb");
    asm volatile("dsb");
    asm volatile("dmb");
    return 0;
}

struct file_operations fops = {
	.open=rdtsc_open,
	.release=rdtsc_release,
	.unlocked_ioctl=rdtsc_ioctl
};

static void enable_counters(void* data)
{
    uint32_t PMCR, PMCNTENSET;
    (void)data;
    /* Enable user mode access */
    asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(1)); /* Enable user mode access */

#if defined(__ARM_ARCH_8A__)
    asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(1 | 2 | 16));
#elif defined(__ARM_ARCH_7A__)
    PMCR = 1 | 2 /* | 8 */ | 16;
    _pmcr_set(PMCR);
#endif

    // asm volatile("mcr p15, 0, %0, c9, c14, 2" :: "r"(0));
    PMCNTENSET = 0x8000000f;
    asm volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(PMCNTENSET));
    printk("cpu_time_now: %llu\n", cpu_time_now());
}

void arm_show_regs(void) {
    uint32_t PMCR, PMINTENCLR, PMCNTENSET;
    printk("show regs\n");
    PMCR = _pmcr();
    printk("pmcr enabled:%d divider:%d number of events: %d \n",
                                !!(PMCR & 1 << 0), !!(PMCR & 1 << 3 ),
                                (PMCR >> 11) & 0x1f);
    asm volatile("mrc p15, 0, %0, c9, c14, 2" : "=r"(PMINTENCLR));
    printk("pm int status = %8x\n", PMINTENCLR);
    asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(PMCNTENSET));
    printk("PMCNTENSET = %8x\n", PMCNTENSET);
    printk("PMUSERENR = %8x\n", _pmuseren());
    printk("pmcr  = %x\n", _pmcr());

    printk("cpu_time_now: %llu\n", cpu_time_now());
}

void force_enable(void* data) {
    (void)data;
    asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(1));
}
void force_disable(void* data) {
    (void)data;
    asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(0));
}

static void show_cpu_features(void) {
    uint32_t features;
    asm volatile("MRC p15, 0, %0, c0, c1, 1" : "=r"(features));
    printk("cpu features: %s %s %s %s %s\n", (features & 0xF << 16 ? "Generic Timer |" : "" ),
                                             (features & 0xF << 12 ? "Virtualization Extension |" : "" ),
                                             (features & 0xF <<  8 ? "Microcontroller Profile |" : "" ),
                                             (features & 0xF <<  4 ? "Security Extensions |" : "" ),
                                             (features & 0xF       ? "Standard Model" : "" ));
}


static int __init rdtsc_init(void)
{
    major_no = register_chrdev(0, DEVICE_NAME, &fops);
    printk("\n Major_no : %d", major_no);

    my_class = class_create(THIS_MODULE, DEVICE_NAME);
    dev = device_create(my_class, NULL, MKDEV(major_no,0), NULL, DEVICE_NAME);
//    printk("dma_set_mask_and_coherent=%d\n", dma_set_mask_and_coherent(dev, DMA_BIT_MASK(32)));

    printk("================= rdtsc_init ============ \n");
    show_cpu_features();
    arm_show_regs();
    
    on_each_cpu(enable_counters, NULL, 1);
    on_each_cpu(force_enable, NULL, 1);
    arm_show_regs();
    printk("User land access to PMU - Enabled\n");
    return 0;
}

static void __exit rdtsc_exit(void)
{
    device_destroy(my_class, MKDEV(major_no,0));
    class_unregister(my_class);
    unregister_chrdev(major_no, DEVICE_NAME);
    on_each_cpu(force_disable, NULL, 1);
    printk("================= rdtsc_exit ============ \n");
    printk("rdtsc_exit cpu_time_now: %llu\n", cpu_time_now());
}



module_init(rdtsc_init);
module_exit(rdtsc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christophe Fontaine");
MODULE_DESCRIPTION("Access to ARM registers only available from kernel space");
