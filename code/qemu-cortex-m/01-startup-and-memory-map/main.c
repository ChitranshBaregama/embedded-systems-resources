/* Example 01 - What Reset_Handler actually did.
 *
 * Runs on QEMU: `make run`
 *
 * The question this answers: when you flash an image, where does each
 * kind of variable live, and who puts it there? Nearly every "my global
 * is garbage" bug traces back to not knowing this.
 */
#include "lm3s6965.h"
#include "uart.h"
#include "semihost.h"

/* .data  - initialised, non-zero. Image in flash, copy in SRAM.
 *          Costs BOTH flash and RAM. */
uint32_t g_initialised = 0xDEADBEEFu;

/* .bss   - zero-initialised. Costs RAM only; the zeroing is done by
 *          the startup loop, not by the loader. */
uint32_t g_zeroed;
uint8_t  g_big_buffer[4096];

/* .rodata - const, stays in flash, never copied. Making a big lookup
 *           table `const` is how you stop it eating RAM. */
static const char g_banner[] = "startup + memory map\n";

/* static local in .bss, to show it is not on the stack */
static uint32_t counter;

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack, _sheap;

static void print_region(const char *name, const void *start, const void *end)
{
    uart_puts(name);
    uart_puts(" ");
    uart_put_hex32((uint32_t)start);
    uart_puts(" .. ");
    uart_put_hex32((uint32_t)end);
    uart_puts("  size=");
    uart_put_u32((uint32_t)((const uint8_t *)end - (const uint8_t *)start));
    uart_puts(" bytes\n");
}

int main(void)
{
    uint32_t stack_local = 0x5A5A5A5Au;

    uart_init(115200u);
    uart_puts("\n=== ");
    uart_puts(g_banner);
    uart_puts("===\n\n");

    /* If the .data copy loop in startup.c were missing, this would print
     * whatever happened to be in SRAM at reset - usually 0x00000000 on
     * QEMU, and random on real silicon. That is the bug you are learning
     * to recognise. */
    uart_puts(".data  g_initialised = ");
    uart_put_hex32(g_initialised);
    uart_puts(g_initialised == 0xDEADBEEFu ? "   (copied OK)\n"
                                           : "   *** COPY LOOP BROKEN ***\n");

    uart_puts(".bss   g_zeroed      = ");
    uart_put_hex32(g_zeroed);
    uart_puts(g_zeroed == 0u ? "   (zeroed OK)\n"
                             : "   *** ZERO LOOP BROKEN ***\n");

    uart_puts("\n-- linker symbols --\n");
    print_region(".data (RAM) ", &_sdata, &_edata);
    uart_puts(".data image in flash at ");
    uart_put_hex32((uint32_t)&_sidata);
    uart_puts("\n");
    print_region(".bss  (RAM) ", &_sbss,  &_ebss);

    uart_puts("\n-- where things actually are --\n");
    uart_puts("g_banner   (.rodata, flash) ");
    uart_put_hex32((uint32_t)g_banner);       uart_puts("\n");
    uart_puts("g_initialised (.data, SRAM) ");
    uart_put_hex32((uint32_t)&g_initialised); uart_puts("\n");
    uart_puts("g_big_buffer  (.bss,  SRAM) ");
    uart_put_hex32((uint32_t)g_big_buffer);   uart_puts("\n");
    uart_puts("counter       (.bss,  SRAM) ");
    uart_put_hex32((uint32_t)&counter);       uart_puts("\n");
    uart_puts("stack_local   (stack, SRAM) ");
    uart_put_hex32((uint32_t)&stack_local);   uart_puts("\n");
    uart_puts("_estack       (top of SRAM) ");
    uart_put_hex32((uint32_t)&_estack);       uart_puts("\n");

    uart_puts("\nStack has grown down ");
    uart_put_u32((uint32_t)((uint8_t *)&_estack - (uint8_t *)&stack_local));
    uart_puts(" bytes from the top.\n");
    uart_puts("Heap would start at ");
    uart_put_hex32((uint32_t)&_sheap);
    uart_puts(" and grow up toward it.\n");

    uart_puts("\nRun `make size` and compare: text = flash, data = both,\n"
              "bss = RAM only. That is the whole story.\n");

    semihost_exit(0);
    return 0;
}
