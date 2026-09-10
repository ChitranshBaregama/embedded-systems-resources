/* Reset path and vector table for Cortex-M3 (LM3S6965 under QEMU).
 *
 * Everything here runs before main(). If you can write this file from
 * memory you understand what "bare metal" actually means.
 */
#include <stdint.h>

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;

int  main(void);
void Reset_Handler(void);
void Default_Handler(void);

/* Weak aliases: any handler a translation unit defines with the same
 * name overrides the default at link time, with no registration code. */
#define WEAK_ALIAS(name) \
    void name(void) __attribute__((weak, alias("Default_Handler")))

WEAK_ALIAS(NMI_Handler);
WEAK_ALIAS(HardFault_Handler);
WEAK_ALIAS(MemManage_Handler);
WEAK_ALIAS(BusFault_Handler);
WEAK_ALIAS(UsageFault_Handler);
WEAK_ALIAS(SVC_Handler);
WEAK_ALIAS(DebugMon_Handler);
WEAK_ALIAS(PendSV_Handler);
WEAK_ALIAS(SysTick_Handler);
WEAK_ALIAS(GPIOA_Handler);
WEAK_ALIAS(UART0_Handler);
WEAK_ALIAS(TIMER0A_Handler);

/* The vector table. Entry 0 is not code - it is the initial MSP value,
 * which the core loads before fetching entry 1. Getting this wrong is
 * the classic "hard faults before main" bug. */
__attribute__((section(".isr_vector"), used))
void (* const g_vectors[])(void) = {
    (void (*)(void))(&_estack),  /*  0: initial stack pointer     */
    Reset_Handler,               /*  1: reset                     */
    NMI_Handler,                 /*  2  */
    HardFault_Handler,           /*  3  */
    MemManage_Handler,           /*  4  */
    BusFault_Handler,            /*  5  */
    UsageFault_Handler,          /*  6  */
    0, 0, 0, 0,                  /*  7-10: reserved               */
    SVC_Handler,                 /* 11  */
    DebugMon_Handler,            /* 12  */
    0,                           /* 13: reserved                  */
    PendSV_Handler,              /* 14  */
    SysTick_Handler,             /* 15  */
    /* --- external IRQ0.. --- */
    GPIOA_Handler,               /* 16 = IRQ 0                    */
    0, 0, 0, 0,                  /* IRQ 1-4                       */
    UART0_Handler,               /* 21 = IRQ 5                    */
    0, 0, 0, 0, 0, 0, 0, 0, 0,   /* IRQ 6-14                      */
    0, 0, 0, 0, 0,               /* IRQ 15-19                     */
    0,                           /* IRQ 20                        */
    0,                           /* IRQ 21                        */
    TIMER0A_Handler,             /* 35 = IRQ 19 on real LM3S; kept
                                  * here only to show the shape.  */
};

void Reset_Handler(void)
{
    /* 1. Copy .data from its flash image into SRAM. Until this loop
     *    finishes, every initialised global holds garbage. */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* 2. Zero .bss. The C standard promises this; nothing else does it. */
    for (dst = &_sbss; dst < &_ebss; dst++) {
        *dst = 0u;
    }

    /* 3. (A real target would init the clock tree here, then call
     *    __libc_init_array() for C++ static constructors.) */

    main();

    /* main() returning on bare metal is a bug, not an exit. Trap it
     * somewhere a debugger can see rather than falling off the end. */
    for (;;) { }
}

void Default_Handler(void)
{
    /* Park here so an unexpected interrupt is visible in the debugger
     * as a specific address rather than a runaway PC. Read IPSR to see
     * which exception number you landed on. */
    for (;;) { }
}
