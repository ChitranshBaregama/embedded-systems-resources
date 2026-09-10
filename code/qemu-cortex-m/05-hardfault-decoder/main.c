/* Example 05 - A HardFault handler that tells you what actually happened.
 *
 * Runs on QEMU: `make run`
 *
 * The default `while(1)` fault handler is where firmware debugging goes
 * to die. When the core takes a fault it has already pushed eight
 * registers - including the faulting PC - onto whichever stack was
 * active. Recover that frame, read the fault status registers, and a
 * "random hang" becomes a line number.
 *
 * This is worth being able to write from memory in an interview.
 */
#include "lm3s6965.h"
#include "uart.h"
#include "semihost.h"

/* --- Fault status registers (identical on every Cortex-M3/M4/M7) ---- */
#define SCB_CFSR   REG32(0xE000ED28u)   /* Configurable Fault Status     */
#define SCB_HFSR   REG32(0xE000ED2Cu)   /* HardFault Status              */
#define SCB_MMFAR  REG32(0xE000ED34u)   /* MemManage Fault Address       */
#define SCB_BFAR   REG32(0xE000ED38u)   /* BusFault Address              */
#define SCB_SHCSR  REG32(0xE000ED24u)   /* System Handler Control        */

/* The exception stack frame, in push order. */
typedef struct {
    uint32_t r0, r1, r2, r3, r12;
    uint32_t lr;      /* return address of the function that was running */
    uint32_t pc;      /* THE FAULTING INSTRUCTION - the one you want     */
    uint32_t psr;
} exception_frame_t;

static volatile int g_expect_fault;

static void report(const char *label, uint32_t v)
{
    uart_puts("  "); uart_puts(label); uart_puts(" = ");
    uart_put_hex32(v); uart_puts("\n");
}

/* Called from the naked assembly trampoline below with the frame pointer
 * already in r0. Keeping the decode in C is what makes it readable. */
void hardfault_report(exception_frame_t *frame, uint32_t exc_return)
{
    /* IPSR tells you which exception you are actually in - do not assume
     * HardFault just because that is the handler you wrote. */
    uint32_t ipsr;
    __asm__ volatile ("mrs %0, ipsr" : "=r"(ipsr));

    uart_puts("\n*** FAULT: ");
    switch (ipsr & 0x1FFu) {
        case 3u:  uart_puts("HardFault");  break;
        case 4u:  uart_puts("MemManage");  break;
        case 5u:  uart_puts("BusFault");   break;
        case 6u:  uart_puts("UsageFault"); break;
        default:  uart_puts("exception #"); uart_put_u32(ipsr & 0x1FFu); break;
    }
    uart_puts(" ***\n");

    uart_puts("\nStacked frame (recovered from ");
    uart_puts((exc_return & 0x4u) ? "PSP" : "MSP");
    uart_puts("):\n");
    report("PC (faulting instr)", frame->pc);
    report("LR (caller)        ", frame->lr);
    report("xPSR               ", frame->psr);
    report("r0                 ", frame->r0);
    report("r1                 ", frame->r1);
    report("r2                 ", frame->r2);
    report("r3                 ", frame->r3);
    report("r12                ", frame->r12);

    uart_puts("\nEXC_RETURN = "); uart_put_hex32(exc_return);
    uart_puts((exc_return & 0x8u) ? "  (returning to thread mode)\n"
                                  : "  (returning to handler mode - nested)\n");

    uint32_t hfsr = SCB_HFSR;
    uint32_t cfsr = SCB_CFSR;
    uart_puts("\nHFSR = "); uart_put_hex32(hfsr); uart_puts("\n");
    if (hfsr & (1u << 30)) {
        uart_puts("  FORCED: a configurable fault escalated. Read CFSR.\n");
    }
    if (hfsr & (1u << 1)) {
        uart_puts("  VECTTBL: fault while reading the vector table.\n");
    }

    uart_puts("\nCFSR = "); uart_put_hex32(cfsr); uart_puts("\n");

    /* --- MemManage (bits 7:0) --- */
    if (cfsr & (1u << 0)) uart_puts("  IACCVIOL:  instruction fetch from a no-execute region\n");
    if (cfsr & (1u << 1)) uart_puts("  DACCVIOL:  data access violated the MPU\n");
    if (cfsr & (1u << 3)) uart_puts("  MUNSTKERR: fault while unstacking on exception return\n");
    if (cfsr & (1u << 4)) uart_puts("  MSTKERR:   fault while stacking on exception entry\n");
    if (cfsr & (1u << 7)) { uart_puts("  MMFAR valid -> "); uart_put_hex32(SCB_MMFAR); uart_puts("\n"); }

    /* --- BusFault (bits 15:8) --- */
    if (cfsr & (1u << 8))  uart_puts("  IBUSERR:   bus error on an instruction fetch\n");
    if (cfsr & (1u << 9))  uart_puts("  PRECISERR: bus error, PC above is exact\n");
    if (cfsr & (1u << 10)) uart_puts("  IMPRECISERR: buffered write failed later - PC is NOT exact.\n"
                                     "               Add a DSB after suspect writes to make it precise.\n");
    if (cfsr & (1u << 11)) uart_puts("  UNSTKERR:  bus fault while unstacking\n");
    if (cfsr & (1u << 12)) uart_puts("  STKERR:    bus fault while stacking - classic stack overflow\n");
    if (cfsr & (1u << 15)) { uart_puts("  BFAR valid -> "); uart_put_hex32(SCB_BFAR); uart_puts("\n"); }

    /* --- UsageFault (bits 25:16) --- */
    if (cfsr & (1u << 16)) uart_puts("  UNDEFINSTR: undefined instruction (often a bad function pointer)\n");
    if (cfsr & (1u << 17)) uart_puts("  INVSTATE:   tried to execute in ARM state - LSB of a pointer was 0\n");
    if (cfsr & (1u << 18)) uart_puts("  INVPC:      bad EXC_RETURN or corrupted stacked PC\n");
    if (cfsr & (1u << 19)) uart_puts("  NOCP:       coprocessor access - FPU used but not enabled\n");
    if (cfsr & (1u << 24)) uart_puts("  UNALIGNED:  unaligned access with UNALIGN_TRP set\n");
    if (cfsr & (1u << 25)) uart_puts("  DIVBYZERO:  divide by zero with DIV_0_TRP set\n");

    uart_puts("\nNext step on a real board:\n");
    uart_puts("  arm-none-eabi-addr2line -e firmware.elf ");
    uart_put_hex32(frame->pc);
    uart_puts("\n");

    if (g_expect_fault) {
        uart_puts("\n(this fault was deliberate - test passed)\n");
        semihost_exit(0);
    }
    semihost_exit(1);
}

/* Naked trampoline: pick the right stack pointer out of EXC_RETURN and
 * hand it to C. It must be naked - any prologue would push registers
 * and move the frame we are trying to read. */
__attribute__((naked)) void fault_trampoline(void)
{
    __asm__ volatile (
        "tst   lr, #4          \n"   /* EXC_RETURN bit 2: which stack?  */
        "ite   eq              \n"
        "mrseq r0, msp         \n"
        "mrsne r0, psp         \n"
        "mov   r1, lr          \n"
        "b     hardfault_report\n"
    );
}

/* Point EVERY fault vector at the same decoder.
 *
 * This matters more than it looks. Once you enable the configurable
 * faults in SHCSR (see main), a UsageFault no longer escalates into
 * HardFault - it dispatches to UsageFault_Handler. If you only replaced
 * HardFault_Handler, your careful decoder is bypassed and the weak
 * default `while(1)` swallows the fault instead. That is a real trap and
 * it is exactly what happened while writing this example. */
void HardFault_Handler(void)  __attribute__((naked, alias("fault_trampoline")));
void MemManage_Handler(void)  __attribute__((naked, alias("fault_trampoline")));
void BusFault_Handler(void)   __attribute__((naked, alias("fault_trampoline")));
void UsageFault_Handler(void) __attribute__((naked, alias("fault_trampoline")));

#if defined(WHICH_FAULT) && (WHICH_FAULT == 5)
/* GCC's -Winfinite-recursion catches the naive version of this, which is
 * a useful reminder that the compiler finds some stack bugs for free.
 * The volatile guard hides the recursion from the analyser so the
 * example can still demonstrate the runtime failure. */
static volatile uint32_t g_keep_going = 1u;

void blow_the_stack(uint32_t depth)
{
    volatile uint32_t frame[64];
    frame[0] = depth;
    if (g_keep_going) {
        blow_the_stack(depth + 1u);
    }
    (void)frame;
}
#endif

int main(void)
{
    uart_init(115200u);
    uart_puts("\n=== HardFault decoder ===\n");

    /* Promote configurable faults so CFSR gets populated instead of
     * everything arriving as a bare escalated HardFault. */
    SCB_SHCSR |= (1u << 16) | (1u << 17) | (1u << 18);  /* MEM/BUS/USG ENA */

    /* Trap divide-by-zero and unaligned access rather than silently
     * returning 0. Costs nothing; catches real bugs. */
    REG32(0xE000ED14u) |= (1u << 4) | (1u << 3);        /* CCR: DIV_0_TRP,
                                                         * UNALIGN_TRP    */
    g_expect_fault = 1;

    /* Pick your fault. Each one exercises a different CFSR bit; try them
     * all and learn to read the status word without a lookup table.
     *
     * Note which ones an emulator reproduces: QEMU is permissive about
     * accesses to unmapped addresses that real silicon rejects, so the
     * bus-fault cases below need actual hardware to observe. That
     * difference is itself worth knowing. */
#define FAULT_UNDEFINED_INSTRUCTION 1
#define FAULT_BAD_FUNCTION_POINTER  2
#define FAULT_DIVIDE_BY_ZERO        3
#define FAULT_UNALIGNED_ACCESS      4
#define FAULT_STACK_OVERFLOW        5
#define FAULT_UNMAPPED_ADDRESS      6   /* not reproduced under QEMU */

#ifndef WHICH_FAULT
#define WHICH_FAULT FAULT_UNDEFINED_INSTRUCTION
#endif

#if WHICH_FAULT == FAULT_UNDEFINED_INSTRUCTION
    uart_puts("\nExecuting an undefined instruction.\n");
    uart_puts("Expect UNDEFINSTR. This is what a corrupted function\n"
              "pointer or a jump into erased flash (0xFFFF) looks like.\n");
    __asm__ volatile (".short 0xFFFF");

#elif WHICH_FAULT == FAULT_BAD_FUNCTION_POINTER
    uart_puts("\nCalling a function pointer with bit 0 clear.\n");
    uart_puts("Expect INVSTATE: Cortex-M is Thumb-only, so the low bit of\n"
              "every code address must be 1. Forgetting the |1 when you\n"
              "build a jump table by hand lands you exactly here.\n");
    void (*fn)(void) = (void (*)(void))0x00000200u;   /* LSB = 0 */
    fn();

#elif WHICH_FAULT == FAULT_DIVIDE_BY_ZERO
    uart_puts("\nDividing by zero with DIV_0_TRP set.\n");
    uart_puts("Expect DIVBYZERO. Without DIV_0_TRP this quietly yields 0\n"
              "and you chase the wrong bug for a day.\n");
    volatile int zero = 0;
    volatile int r = 100 / zero;
    (void)r;

#elif WHICH_FAULT == FAULT_UNALIGNED_ACCESS
    uart_puts("\nUnaligned 32-bit load with UNALIGN_TRP set.\n");
    uart_puts("Expect UNALIGNED. Classic when you cast a byte pointer\n"
              "mid-packet to uint32_t* to pull a length field out.\n");
    static uint8_t packet[8] = {0,1,2,3,4,5,6,7};
    volatile uint32_t *p = (volatile uint32_t *)(void *)&packet[1];
    volatile uint32_t v = *p;
    (void)v;

#elif WHICH_FAULT == FAULT_STACK_OVERFLOW
    uart_puts("\nRecursing until the stack runs into .bss.\n");
    uart_puts("Expect STKERR, or silent corruption on a part with no MPU\n"
              "guard region - which is why you set one up.\n");
    extern void blow_the_stack(uint32_t depth);
    blow_the_stack(0u);

#else
    uart_puts("\nReading unmapped address 0xE0000000.\n");
    uart_puts("Expect PRECISERR + BFAR. QEMU does not model this - on a\n"
              "real part it faults immediately.\n");
    volatile uint32_t *bad = (volatile uint32_t *)0xE0000000u;
    volatile uint32_t sink = *bad;
    (void)sink;
#endif

    uart_puts("*** did not fault - unexpected ***\n");
    semihost_exit(1);
    return 0;
}
