
#include <asm/types.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <common.h>

#define INTERRUPT_HANDLER_NOT_SET -1

/* r3 should have exception number (2 for buserr, 5 for tick, etc.) */
/* r4 should have handler function address */
extern void _exception_handler_add(int,void(*)(void));
extern void _interrupt_handler(void);

extern unsigned long _interrupt_handler_table;
extern unsigned long _interrupt_handler_data_ptr_table;

int
interrupt_init(void)
{

	
	/* install handler for external interrupt exception */
	_exception_handler_add(8,_interrupt_handler);
	
	/* Enable interrupts in supervisor register */
	mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_IEE);
	
	return 0;
}

void 
enable_interrupts  (void)
{
	/* Set interrupt enable bit in supervisor register */
	mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_IEE);
}


int 
disable_interrupts  (void)
{
	/* Clear interrupt enable bit in supervisor register */
	mtspr (SPR_SR, mfspr (SPR_SR) & ~SPR_SR_IEE);
	/* 
	 * HACK: Timer exception needs to be disabled before loading 
	 * os since the exception vector will be trashed. 
	 * TODO: Find a better hook in do_bootm than here.
	 */
	mtspr (SPR_SR, mfspr (SPR_SR) & ~SPR_SR_TEE);
	return 0;
}

#if defined(CONFIG_CMD_IRQ)
int do_irqinfo (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	ulong *handler, *arg; 

	handler = &_interrupt_handler_table;
	arg     = &_interrupt_handler_data_ptr_table;

	printf ("\nInterrupt-Information:\n\n");
	printf ("Nr  Routine   Arg\n");
	printf ("-----------------\n");

	for (i=0; i<32; i++) {
		if (handler[i] != INTERRUPT_HANDLER_NOT_SET) {
			printf ("%02d  %08lx  %08lx\n",
				i,
				(ulong)handler[i],
				(ulong)arg[i]);
		}
	}
	printf ("\n");

	return (0);
}
#endif
