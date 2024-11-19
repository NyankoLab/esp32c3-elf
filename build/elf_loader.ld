SECTIONS
{
  .text 0x00000000 :
  {
    *(.text)
    *(.text.*)
  }

  .rodata :
  {
    *(.rodata)
    *(.rodata.*)
  }

  .data :
  {
    *(.data)
    *(.data.*)
    *(.sdata)
    *(.sdata.*)
  }

  .bss :
  {
    *(.bss)
    *(.bss.*)
    *(.sbss)
    *(.sbss.*)
  }

  .rtc.data :
  {
    *(.rtc.data)
    *(.rtc.data.*)
  }

  .rela.text :
  {
    *(.rela.text)
    *(.rela.text.*)
  }

  .rela.rodata :
  {
    *(.rela.rodata)
    *(.rela.rodata.*)
  }
}
