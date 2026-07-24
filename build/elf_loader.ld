SECTIONS
{
  .stl :
  {
    *(.text._ZNKSt3__112basic_string*)
    *(.text._ZNKRSt3__112basic_string*)
    *(.text._ZNOSt3__112basic_string*)
    *(.text._ZNSt3__112basic_string*)
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
    *(.rela.text._ZNKSt3__112basic_string*)
    *(.rela.text._ZNKRSt3__112basic_string*)
    *(.rela.text._ZNOSt3__112basic_string*)
    *(.rela.text._ZNSt3__112basic_string*)
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
