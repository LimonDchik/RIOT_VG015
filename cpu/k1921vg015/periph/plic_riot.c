/*
 * RIOT PLIC interface wrapper for k1921vg015
 */

#include "plic.h"

void plic_init(void)
{
    /* Allow all configured external interrupts by default. */
    PLIC_SetThreshold(Plic_Mach_Target, 0);
}

void plic_isr_handler(void)
{
    /* Handle external interrupt via vendor-specific PLIC handler */
    PLIC_MachHandler();
}
