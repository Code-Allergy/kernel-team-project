#ifndef MEM_H
#define MEM_H

/*
 * Memory Layout Overview:
 * 0x0 - 0x7FFFFFFF = User space, managed by elf loader or linker script.
 * We can decide on where we place the user heap and user stack in virtual memory.
 */
#define MEM_USER_BASE      0x0         /* 0x0-0x7FFFFFFF */

/* Physical memory identity mapping (512MB) */
#define MEM_PHYS_BASE      0x80000000  /* 0x80000000-0x9FFFFFFF */
/* We will identity map the entire physical memory here, so we can access it with ttbr0 page. */

/* Kernel code and data region (512MB) */
#define MEM_KERNEL_BASE    0xA0000000  /* 0xA0000000-0xBFFFFFFF */
/* This maps directly to the start of physical memory, but will let us enforce page permissions
 * and be free to move the kernel around in physical memory if required.
 * This is where our linker script will place the kernel, make sure sections are 4k aligned.
 * We probably also want a header section of the kernel, that will contain some basic info.
 */

/* Hardware and device mapping region (256MB) */
#define MEM_DEVICE_BASE    0xC0000000  /* 0xC0000000-0xCFFFFFFF */
/* Remap physical addresses from lower memory to here, so we can access with ttbr1 page.
 * This region needs to specifically be uncached.
 * We can also slice up some of this extra space for DMA buffers.
 * Might also remap our other memory here, if we want to access it.
 */

/* Kernel dynamic memory region (512MB) */
#define MEM_HEAP_BASE      0xD0000000  /* 0xD0000000-0xEFFFFFFF */
/* Kernel heap space, can split into multiple regions if needed. */

/* Kernel stacks region (256MB) */
#define MEM_STACK_BASE     0xF0000000  /* 0xF0000000-0xFFFFFFFF */
/* We can decide on how to split up this 256MB later. */

/*
 * The regions are larger than we will ever need, but it makes it easier to understand
 * the type of pointer we are working with.
 */

/* Memory constants */
#define MEM_SECTION_SIZE   0x100000    /* 1MB section size */
#define MEM_PAGE_SIZE      4096        /* 4KB page size */

#endif /* MEM_H */
