
#include <asm/types.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <common.h>


/* r3 should have exception number (2 for buserr, 5 for tick, etc.) */
/* r4 should have handler function address */
extern void _exception_handler_add(int,void(*)(void));
extern void _interrupt_handler(void);

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
	return 0;
}
