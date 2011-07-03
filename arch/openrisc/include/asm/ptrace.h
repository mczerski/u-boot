#ifndef __ASM_OPENRISC_PTRACE_H
#define __ASM_OPENRISC_PTRACE_H

#include <asm/spr-defs.h>
/*
 * This struct defines the way the registers are stored on the
 * kernel stack during a system call or other kernel entry.
 *
 * this should only contain volatile regs
 * since we can keep non-volatile in the thread_struct
 * should set this up when only volatiles are saved
 * by intr code.
 *
 * Since this is going on the stack, *CARE MUST BE TAKEN* to insure
 * that the overall structure is a multiple of 16 bytes in length.
 *
 * Note that the offsets of the fields in this struct correspond with
 * the values below.
 */

#ifndef __ASSEMBLY__

struct pt_regs {
	long  pc;
	long  sr;
	long  sp;
	long  gprs[30];
	long  orig_gpr11;  /* Used for restarting system calls */
	long  result;     /* Result of a system call */
	long  syscallno;  /* Syscall no. (used by strace) */
};
#endif /* __ASSEMBLY__ */

#ifdef __KERNEL__
#define STACK_FRAME_OVERHEAD  128  /* size of minimum stack frame */
//#define STACK_FRAME_OVERHEAD  0  /* size of minimum stack frame */

#define instruction_pointer(regs) ((regs)->pc)
#define user_mode(regs) (((regs)->sr & SPR_SR_SM) == 0)
#define profile_pc(regs) instruction_pointer(regs)

#endif /* __KERNEL__ */

/*
 * Offsets used by 'ptrace' system call interface.
 */
#define PC        0
#define SR        4
#define SP        8
#define GPR2      12
#define GPR3      16
#define GPR4      20
#define GPR5      24
#define GPR6      28
#define GPR7      32
#define GPR8      36
#define GPR9      40
#define GPR10     44
#define GPR11     48
#define GPR12     52
#define GPR13     56
#define GPR14     60
#define GPR15     64
#define GPR16     68
#define GPR17     72
#define GPR18     76
#define GPR19     80
#define GPR20     84
#define GPR21     88
#define GPR22     92
#define GPR23     96
#define GPR24     100
#define GPR25     104
#define GPR26     108
#define GPR27     112
#define GPR28     116
#define GPR29     120
#define GPR30     124
#define GPR31     128
#define ORIG_GPR11 132
#define RESULT    136
#define SYSCALLNO 140

#endif /* __ASM_OPENRISC_PTRACE_H */
