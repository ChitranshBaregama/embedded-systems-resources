/* Linker script for TI Stellaris LM3S6965 (Cortex-M3), the board QEMU
 * emulates as `-M lm3s6965evb`.  256 KB flash @ 0x00000000, 64 KB SRAM @ 0x20000000.
 *
 * Read this file top to bottom once; it explains where every byte of your
 * image ends up and why the startup code has to copy some of them.
 */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
  SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Top of stack = end of SRAM. The Cortex-M core loads this into MSP
 * straight out of vector[0] before executing a single instruction. */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

ENTRY(Reset_Handler)

SECTIONS
{
  /* Vector table must be the first thing at 0x00000000. */
  .isr_vector : ALIGN(4)
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .text : ALIGN(4)
  {
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  /* ARM unwind tables. Discarding them silently breaks anything that
   * throws, so keep them even in C-only builds. */
  .ARM.exidx : { *(.ARM.exidx*) } > FLASH

  /* _sidata is where the initialised-data image LIVES in flash.
   * _sdata.._edata is where it must BE at runtime, in SRAM.
   * Reset_Handler copies between the two. This split is the single
   * most commonly misunderstood thing in a bare-metal image. */
  _sidata = LOADADDR(.data);

  .data : ALIGN(4)
  {
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > SRAM AT > FLASH

  /* .bss occupies no space in the image; it is zeroed at startup. */
  .bss (NOLOAD) : ALIGN(4)
  {
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > SRAM

  /* Anything left over is heap. We do not use malloc here, but the
   * symbol is useful when you want to prove that at link time. */
  .heap (NOLOAD) : ALIGN(8)
  {
    _sheap = .;
    . = . + 1024;
    _eheap = .;
  } > SRAM

  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) }
}
