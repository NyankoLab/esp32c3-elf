SECTIONS
{
  .stl :
  {
    *(.text.*St3__112basic_string*)
  }

  .text :
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

  .bss (NOLOAD) :
  {
    *(.bss)
    *(.bss.*)
    *(.sbss)
    *(.sbss.*)
  }

  .rtc (NOLOAD) :
  {
    *(.rtc.bss)
    *(.rtc.bss.*)
    *(.rtc.data)
    *(.rtc.data.*)
  }

  .rela.stl :
  {
    *(.rela.text.*St3__112basic_string*)
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
