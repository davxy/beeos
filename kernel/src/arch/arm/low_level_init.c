#include "mm/frame.h"
#include "util.h"

#define PAGE_SIZE   4096
#define MEM_BEG     0x0
#define MEM_END     0x1000000   /* 16M */


/* Realview Platform Based ARM926EK-S */
void low_level_init(void)
{
    uint32_t kend;

    /* Initialize the memory allocator */
    frame_zone_add((void *)MEM_BEG, MEM_END-MEM_BEG, PAGE_SIZE, 0);

    /* Get current kernel end (may have growed since the startup */
    kend = ALIGN_UP((uint32_t)kmalloc(0,0), PAGE_SIZE);
    while (kend < MEM_END)
    {
        frame_free((void *)kend, 0);
        kend += PAGE_SIZE;
    }
}
