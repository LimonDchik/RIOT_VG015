/*
 * RIOT PLIC interface wrapper for k1921vg015
 */

#include "plic.h"

void plic_init(void)
{
    /* Initialize PLIC for RIOT - set threshold high to disable all interrupts initially */
    PLIC_SetThreshold(Plic_Mach_Target, 15);
}

void plic_isr_handler(void)
{
    /* Handle external interrupt via vendor-specific PLIC handler */
    PLIC_MachHandler();
}
