/* ARM semihosting: the guest traps with BKPT #0xAB and the emulator (or
 * an attached debugger) services the call. QEMU implements it, so an
 * example can exit with a status code instead of spinning forever -
 * which is what makes these runnable in CI.
 *
 * On real hardware with no debugger attached, BKPT faults. This is a
 * test-harness facility, never something you ship.
 *
 * Two calls, and the difference matters:
 *
 *   SYS_EXIT (0x18)           - on 32-bit ARM, R1 holds the REASON CODE
 *                               ITSELF, not a pointer. QEMU maps
 *                               ADP_Stopped_ApplicationExit to process
 *                               exit status 0 and every other reason to 1.
 *                               There is no way to return your own code.
 *
 *   SYS_EXIT_EXTENDED (0x20)  - R1 holds a POINTER to {reason, exit_code},
 *                               so the guest can return an arbitrary
 *                               status. This is the one you want in CI.
 *
 * Passing the pointer form to 0x18 on a 32-bit target - an easy mistake,
 * and one this file made on its first draft - means the emulator reads
 * your pointer value as a reason code, does not recognise it, and exits
 * 1 no matter what your firmware decided. Green tests that always fail.
 */
#ifndef SEMIHOST_H
#define SEMIHOST_H

#include <stdint.h>

#define SYS_EXIT             0x18u
#define SYS_EXIT_EXTENDED    0x20u
#define ADP_STOPPED_APPEXIT  0x20026u

static inline void semihost_call(uint32_t op, uint32_t arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register uint32_t r1 __asm__("r1") = arg;
    __asm__ volatile ("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
}

static inline void semihost_exit(int code)
{
    volatile uint32_t block[2] = { ADP_STOPPED_APPEXIT, (uint32_t)code };
    semihost_call(SYS_EXIT_EXTENDED, (uint32_t)&block[0]);

    /* Fallback for hosts that do not implement 0x20: this at least gets
     * a clean exit-0 for success. */
    semihost_call(SYS_EXIT, ADP_STOPPED_APPEXIT);

    for (;;) { }
}

#endif
