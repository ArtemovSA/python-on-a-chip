/*
# This file is Copyright 2007 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
# 
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING up one directory from this.
*/


#undef __FILE_ID__
#define __FILE_ID__ 0x70


/** PyMite platform-specific routines for ARM7 target */


#include "pm.h"

#include "AT91SAM7S64.h"
#include "lib_AT91SAM7S64.h"
#include "Board.h"


#define RTTC_INTERRUPT_LEVEL 0

/** 200 ms for 48 MHz */
#define PIV_200_MS 600000

/** Baud rate of serial port */
#define UART_BAUD 19200


static AT91S_USART * pusart0 = AT91C_BASE_US0;


static void
at91sam7_pit_handler(void)
{
    PmReturn_t retval;

    retval = pm_vmPeriodic(200);
    PM_REPORT_IF_ERROR(retval);
}


PmReturn_t
plat_init(void)
{
    /* Enable PIO's clock for PIOA and USART0 */
    AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, (1 << AT91C_ID_PIOA)
                                                | (1 << AT91C_ID_US0));

    /* Configure PIOA for LEDs and clear them */
    AT91F_PIO_CfgOutput(AT91C_BASE_PIOA, LED_MASK);
    AT91F_PIO_SetOutput(AT91C_BASE_PIOA, LED_MASK);

    /* Configure PIT interrupt */
    AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,
                          AT91C_ID_SYS,
                          RTTC_INTERRUPT_LEVEL,
                          AT91C_AIC_SRCTYPE_EXT_POSITIVE_EDGE,
                          (void*)at91sam7_pit_handler);

    /* Enable PIT interrupt */
    AT91C_BASE_PITC->PITC_PIMR = AT91C_PITC_PITEN
                                 | AT91C_PITC_PITIEN
                                 | PIV_200_MS;
    AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);

    /* Enable RxD0, TxDO pins */
    *AT91C_PIOA_PDR = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;

    /* Reset Rx, Tx and Rx Tx disables */
    pusart0->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX
                    | AT91C_US_RXDIS | AT91C_US_TXDIS;

    /* Normal Mode, Clock = MCK, 8N1 */
    pusart0->US_MR = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK
                    | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE
                    | AT91C_US_NBSTOP_1_BIT;

    /* Baud Rate Divisor */
    pusart0->US_BRGR = (MCK / (16 * UART_BAUD));

    /* Rx, Tx enable */
    pusart0->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;

    return PM_RET_OK;
}


/* TODO: disable the peripherals and interrupts */
PmReturn_t 
plat_deinit(void)
{
    return PM_RET_OK;
}


/*
 * Gets a byte from the address in the designated memory space
 * Post-increments *paddr.
 */
uint8_t
plat_memGetByte(PmMemSpace_t memspace, uint8_t const **paddr)
{
    uint8_t b = 0;

    switch (memspace)
    {
        case MEMSPACE_RAM:
        case MEMSPACE_PROG:
            b = **paddr;
            *paddr += 1;
            return b;

        case MEMSPACE_EEPROM:
        case MEMSPACE_SEEPROM:
        case MEMSPACE_OTHER0:
        case MEMSPACE_OTHER1:
        case MEMSPACE_OTHER2:
        case MEMSPACE_OTHER3:
        default:
            return 0;
    }
}


PmReturn_t
plat_getByte(uint8_t *b)
{
    int c;
    PmReturn_t retval = PM_RET_OK;

    while ((pusart0->US_CSR & AT91C_US_RXRDY) == 0);
    c = (pusart0->US_RHR);
    *b = c & 0xFF;

    if (c > 0xFF)
    {
        PM_RAISE(retval, PM_RET_EX_IO);
    }

    return retval;
}


PmReturn_t
plat_putByte(uint8_t b)
{
    while (!(pusart0->US_CSR & AT91C_US_TXRDY));
    pusart0->US_THR = b;

    return PM_RET_OK;
}


PmReturn_t
plat_getMsTicks(uint32_t *r_ticks)
{
    /* TODO: make access atomic */
    *r_ticks = pm_timerMsTicks;

    return PM_RET_OK;
}


void
plat_reportError(PmReturn_t result)
{
    plat_putByte('E');
    plat_putByte('r');
    plat_putByte('r');
    plat_putByte('\n');
}
