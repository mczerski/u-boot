
#ifndef __ASM_OPENRISC_SYSTEM_H
#define __ASM_OPENRISC_SYSTEM_H


#include <asm/spr-defs.h>

static inline unsigned long mfspr(unsigned long add)
{
        unsigned long ret;
        __asm__ __volatile__ ("l.mfspr %0,r0,%1" : "=r" (ret) : "K" (add));
        return ret;
}

#define mtspr_off(_spr, _off, _val) __asm__ __volatile__ ("l.mtspr %0,%1,%2" :: "r" (_off), "r" (_val), "K" (_spr))

static inline void mtspr(unsigned long add, unsigned long val)
{
	__asm__ __volatile__ ("l.mtspr r0,%1,%0" :: "K" (add), "r" (val));
}

#endif /* __ASM_OPENRISC_SYSTEM_H */
