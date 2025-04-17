ENTRY(_start)
SECTIONS {
  . = 1M;

  .text ALIGN(4K) : AT(ADDR(.text)) {
    *(.multiboot)
    *(.text*)
  } :text

  .rodata ALIGN(4K) : {
    *(.rodata*)
  } :rodata

  .data ALIGN(4K) : {
    *(.data*)
  } :data

  .bss ALIGN(4K) : {
    *(.bss*)
    *(COMMON)
  } :bss
}

PHDRS {
  text PT_LOAD FLAGS(5);    /* RX */
  rodata PT_LOAD FLAGS(4);  /* R  */
  data PT_LOAD FLAGS(6);    /* RW */
  bss PT_LOAD FLAGS(6);     /* RW */
}
