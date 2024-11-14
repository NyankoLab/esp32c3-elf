#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define int32_t int
#define uint32_t unsigned int
#include <elf_types.h>

#define R_RISCV_NONE                 0
#define R_RISCV_32                   1
#define R_RISCV_64                   2
#define R_RISCV_RELATIVE             3
#define R_RISCV_COPY                 4
#define R_RISCV_JUMP_SLOT            5
#define R_RISCV_TLS_DTPMOD32         6
#define R_RISCV_TLS_DTPMOD64         7
#define R_RISCV_TLS_DTPREL32         8
#define R_RISCV_TLS_DTPREL64         9
#define R_RISCV_TLS_TPREL32         10
#define R_RISCV_TLS_TPREL64         11
#define R_RISCV_TLSDESC             12

#define R_RISCV_BRANCH              16
#define R_RISCV_JAL                 17
#define R_RISCV_CALL                18
#define R_RISCV_CALL_PLT            19
#define R_RISCV_GOT_HI20            20
#define R_RISCV_TLS_GOT_HI20        21
#define R_RISCV_TLS_GD_HI20         22
#define R_RISCV_PCREL_HI20          23
#define R_RISCV_PCREL_LO12_I        24
#define R_RISCV_PCREL_LO12_S        25
#define R_RISCV_HI20                26
#define R_RISCV_LO12_I              27
#define R_RISCV_LO12_S              28
#define R_RISCV_TPREL_HI20          29
#define R_RISCV_TPREL_LO12_I        30
#define R_RISCV_TPREL_LO12_S        31
#define R_RISCV_TPREL_ADD           32
#define R_RISCV_ADD8                33
#define R_RISCV_ADD16               34
#define R_RISCV_ADD32               35
#define R_RISCV_ADD64               36
#define R_RISCV_SUB8                37
#define R_RISCV_SUB16               38
#define R_RISCV_SUB32               39
#define R_RISCV_SUB64               40
#define R_RISCV_GOT32_PCREL         41

#define R_RISCV_ALIGN               43
#define R_RISCV_RVC_BRANCH          44
#define R_RISCV_RVC_JUMP            45
#define R_RISCV_RVC_LUI             46
#define R_RISCV_RELAX               51
//#define R_RISCV_SUB6              52
//#define R_RISCV_SET6              53
//#define R_RISCV_SET8              54
//#define R_RISCV_SET16             55
//#define R_RISCV_SET32             56
//#define R_RISCV_32_PCREL          57
//#define R_RISCV_IRELATIVE         58
//#define R_RISCV_PLT32             59
//#define R_RISCV_SET_ULEB128       60
//#define R_RISCV_SUB_ULEB128       61
//#define R_RISCV_TLSDESC_HI20      62
//#define R_RISCV_TLSDESC_LOAD_LO12 63
//#define R_RISCV_TLSDESC_ADD_LO12  64
//#define R_RISCV_TLSDESC_CALL      65

char const* const riscv_type_name[128] =
{
  [R_RISCV_NONE         ]   = "NONE",
  [R_RISCV_32           ]   = "32",
  [R_RISCV_64           ]   = "64",
  [R_RISCV_RELATIVE     ]   = "RELATIVE",
  [R_RISCV_COPY         ]   = "COPY",
  [R_RISCV_JUMP_SLOT    ]   = "JUMP_SLOT",
  [R_RISCV_TLS_DTPMOD32 ]   = "TLS_DTPMOD32",
  [R_RISCV_TLS_DTPMOD64 ]   = "TLS_DTPMOD64",
  [R_RISCV_TLS_DTPREL32 ]   = "TLS_DTPREL32",
  [R_RISCV_TLS_DTPREL64 ]   = "TLS_DTPREL64",
  [R_RISCV_TLS_TPREL32  ]   = "TLS_TPREL32",
  [R_RISCV_TLS_TPREL64  ]   = "TLS_TPREL64",
  [R_RISCV_TLSDESC      ]   = "TLSDESC",

  [R_RISCV_BRANCH       ]   = "BRANCH",
  [R_RISCV_JAL          ]   = "JAL",
  [R_RISCV_CALL         ]   = "CALL",
  [R_RISCV_CALL_PLT     ]   = "CALL_PLT",
  [R_RISCV_GOT_HI20     ]   = "GOT_HI20",
  [R_RISCV_TLS_GOT_HI20 ]   = "TLS_GOT_HI20",
  [R_RISCV_TLS_GD_HI20  ]   = "TLS_GD_HI20",
  [R_RISCV_PCREL_HI20   ]   = "PCREL_HI20",
  [R_RISCV_PCREL_LO12_I ]   = "PCREL_LO12_I",
  [R_RISCV_PCREL_LO12_S ]   = "PCREL_LO12_S",
  [R_RISCV_HI20         ]   = "HI20",
  [R_RISCV_LO12_I       ]   = "LO12_I",
  [R_RISCV_LO12_S       ]   = "LO12_S",
  [R_RISCV_TPREL_HI20   ]   = "TPREL_HI20",
  [R_RISCV_TPREL_LO12_I ]   = "TPREL_LO12_I",
  [R_RISCV_TPREL_LO12_S ]   = "TPREL_LO12_S",
  [R_RISCV_TPREL_ADD    ]   = "TPREL_ADD",
  [R_RISCV_ADD8         ]   = "ADD8",
  [R_RISCV_ADD16        ]   = "ADD16",
  [R_RISCV_ADD32        ]   = "ADD32",
  [R_RISCV_ADD64        ]   = "ADD64",
  [R_RISCV_SUB8         ]   = "SUB8",
  [R_RISCV_SUB16        ]   = "SUB16",
  [R_RISCV_SUB32        ]   = "SUB32",
  [R_RISCV_SUB64        ]   = "SUB64",
  [R_RISCV_GOT32_PCREL  ]   = "GOT32_PCREL",

  [R_RISCV_ALIGN        ]   = "ALIGN",
  [R_RISCV_RVC_BRANCH   ]   = "RVC_BRANCH",
  [R_RISCV_RVC_JUMP     ]   = "RVC_JUMP",
  [R_RISCV_RVC_LUI      ]   = "RVC_LUI",
  [R_RISCV_RELAX        ]   = "RELAX",
};

static uint32_t _extract_bits(uint32_t num, int low, int size)
{
  return (num & (((1 << size) - 1) << low)) >> low;
}

static uint32_t _get_val(uint16_t* addr)
{
  uint32_t ret;
  ret = *addr | (*(addr + 1)) << 16;
  return ret;
}

static void _set_val(uint16_t* addr, uint32_t val)
{
  *addr       = (val & 0xffff);
  *(addr + 1) = (val >> 16);
}

static void _add_val(uint16_t* addr, uint32_t val)
{
  uint32_t cur = _get_val(addr);
  _set_val(addr, cur + val);
}

static void _calc_imm(int offset, int* imm_hi, int* imm_lo)
{
  int lo;
  int hi = offset / 4096;
  int r  = offset % 4096;

  if (2047 < r)
  {
    hi++;
  }
  else if (r < -2048)
  {
    hi--;
  }

  lo = offset - (hi * 4096);

  *imm_lo = lo;
  *imm_hi = hi;
}

typedef struct elf32_rela_link
{
  struct elf32_rela_link* next;
  struct elf32_rela* begin;
  struct elf32_rela* end;  
} elf32_rela_link;

int main(int argc, char const* argv[])
{
  if (argc <= 2)
  {
    printf("usage: relocation input output\n");
    return 0;
  }

  FILE* file = fopen(argv[1], "rb");
  if (file == NULL)
  {
    printf("%s is open failed\n", argv[1]);
    return 0;
  }

  // Memory
  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
  fseek(file, 0, SEEK_SET);
  char* memory = malloc(length);
  fread(memory, 1, length, file);
  fclose(file);

  // ELF Header
  struct elf32_hdr* elf = ((struct elf32_hdr*)memory);
//printf("%-16s: %s\n", "ident", elf->ident);
//printf("%-16s: %d\n", "type", elf->type);
//printf("%-16s: %d\n", "machine", elf->machine);
//printf("%-16s: %d\n", "version", elf->version);
//printf("%-16s: %d\n", "entry", elf->entry);
//printf("%-16s: %d\n", "phoff", elf->phoff);
//printf("%-16s: %d\n", "shoff", elf->shoff);
//printf("%-16s: %d\n", "flags", elf->flags);
//printf("%-16s: %d\n", "ehsize", elf->ehsize);
//printf("%-16s: %d\n", "phentsize", elf->phentsize);
//printf("%-16s: %d\n", "phnum", elf->phnum);
//printf("%-16s: %d\n", "shentsize", elf->shentsize);
//printf("%-16s: %d\n", "shnum", elf->shnum);
//printf("%-16s: %d\n", "shstrndx", elf->shstrndx);

  // Section
  struct elf32_rela_link* rela_link = NULL;
  struct elf32_sym* symtab = NULL;
  struct elf32_sym* symtab_end = NULL;
  char* strtab = NULL;

  // Section Header
  struct elf32_shdr* section = (struct elf32_shdr*)(memory + elf->shoff);
  char* strndx = memory + section[elf->shstrndx].offset;
  for (Elf32_Half i = 0; i < elf->shnum; ++i)
  {
    if (strstr(strndx + section[i].name, ".rela") == strndx + section[i].name)
    {
      struct elf32_rela* begin = (struct elf32_rela*)(memory + section[i].offset);
      struct elf32_rela* end = (struct elf32_rela*)(memory + section[i].offset + section[i].size);
      struct elf32_rela_link* link = malloc(sizeof(struct elf32_rela_link));
      link->next = rela_link;
      link->begin = begin;
      link->end = end;
      rela_link = link;
    }
    if (strstr(strndx + section[i].name, ".symtab") == strndx + section[i].name)
    {
      symtab = (struct elf32_sym*)(memory + section[i].offset);
      symtab_end = (struct elf32_sym*)(memory + section[i].offset + section[i].size);
    }
    if (strstr(strndx + section[i].name, ".strtab") == strndx + section[i].name)
    {
      strtab = (memory + section[i].offset);
    }
  } 

  // RELA
  for (struct elf32_rela_link* link = rela_link; link; link = link->next)
  {
    struct elf32_rela* begin = link->begin;
    struct elf32_rela* end = link->end;
    Elf32_Half ndx = 0;
    char* text = NULL;
    for (Elf32_Half i = 0; i < elf->shnum; ++i)
    {
      if ((struct elf32_rela*)(memory + section[i].offset) == begin)
      {
        char* name = strndx + section[i].name + sizeof(".rela") - 1;
        for (Elf32_Half i = 0; i < elf->shnum; ++i)
        {
          if (strcmp(strndx + section[i].name, name) == 0)
          {
            ndx = i;
            text = memory + section[i].offset;
            printf("name: %s (%d)\n", name, ndx);
          }
        }
      }
    }
    if (ndx == 0)
    {
      continue;
    }

    struct elf32_rela* p = begin;
    for (struct elf32_rela* rel = begin; rel != end; ++rel)
    {
      unsigned int addr = rel->offset;
      unsigned int relotype = ELF_R_TYPE(rel->info);
      unsigned int relosym = ELF_R_SYM(rel->info);
      const struct elf32_sym* sym = &symtab[relosym];

      if (relotype == R_RISCV_ALIGN || relotype == R_RISCV_RELAX)
      {
        continue;
      }

      if (sym->shndx == 0)
      {
        (*p++) = (*rel);
        continue;
      }

      if (sym->shndx != ndx)
      {
        (*p++) = (*rel);
        continue;
      }

      switch (relotype)
      {
      case R_RISCV_CALL:
      case R_RISCV_CALL_PLT:
      {
        int offset = sym->value + rel->addend - addr;
        int imm_hi;
        int imm_lo;
        _calc_imm(offset, &imm_hi, &imm_lo);

        _add_val((uint16_t*)(size_t)(text + addr + 0), imm_hi << 12);
        _add_val((uint16_t*)(size_t)(text + addr + 4), imm_lo << 20);
        break;
      }
      case R_RISCV_BRANCH:
      {
        int offset = sym->value + rel->addend - addr;
        uint32_t imm12 = _extract_bits(offset, 12, 1) << 31;
        uint32_t imm10_5 = _extract_bits(offset, 5, 6) << 25;
        uint32_t imm4_1 = _extract_bits(offset, 1, 4) << 8;
        uint32_t imm11 = _extract_bits(offset, 11, 1) << 7;

        _add_val((uint16_t*)(size_t)(text + addr), imm12 | imm10_5 | imm4_1 | imm11);
        break;
      }
      case R_RISCV_JAL:
      {
        int offset = sym->value + rel->addend - addr;
        uint32_t imm20 = _extract_bits(offset, 20, 1) << 31;
        uint32_t imm10_1 = _extract_bits(offset, 1, 10) << 21;
        uint32_t imm11 = _extract_bits(offset, 11, 1) << 20;
        uint32_t imm19_12 = _extract_bits(offset, 12, 8) << 12;

        _add_val((uint16_t*)(size_t)(text + addr), imm20 | imm10_1 | imm11 | imm19_12);
        break;
      }
#if 0
      case R_RISCV_HI20:
      {
        int offset = sym->value + rel->addend;
        int imm_hi;
        int imm_lo;
        _calc_imm(offset, &imm_hi, &imm_lo);

        uint32_t insn = _get_val((uint16_t*)(size_t)(text + addr));
        _set_val((uint16_t*)(size_t)(text + addr), (insn & 0x00000fff) | (imm_hi << 12));
        break;
      }
      case R_RISCV_LO12_I:
      {
        int offset = sym->value + rel->addend;
        int imm_hi;
        int imm_lo;
        _calc_imm(offset, &imm_hi, &imm_lo);

        uint32_t insn = _get_val((uint16_t*)(size_t)(text + addr));
        _set_val((uint16_t*)(size_t)(text + addr), (insn & 0x000fffff) | (imm_lo << 20));
        break;
      }
#endif
      case R_RISCV_RVC_JUMP:
      {
        int offset = sym->value + rel->addend - addr;
        uint16_t imm11 = _extract_bits(offset, 11, 1) << 12;
        uint16_t imm4 = _extract_bits(offset, 4, 1) << 11;
        uint16_t imm9_8 = _extract_bits(offset, 8, 2) << 9;
        uint16_t imm10 = _extract_bits(offset, 10, 1) << 8;
        uint16_t imm6 = _extract_bits(offset, 6, 1) << 7;
        uint16_t imm7 = _extract_bits(offset, 7, 1) << 6;
        uint16_t imm3_1 = _extract_bits(offset, 1, 3) << 3;
        uint16_t imm5 = _extract_bits(offset, 5, 1) << 2;

        _add_val((uint16_t*)(size_t)(text + addr), imm11 | imm4 | imm9_8 | imm10 | imm6 | imm7 | imm3_1 | imm5);
        break;
      }
      case R_RISCV_RVC_BRANCH:
      {
        int offset = sym->value + rel->addend - addr;
        uint16_t imm8 = _extract_bits(offset, 8, 1) << 12;
        uint16_t imm4_3 = _extract_bits(offset, 3, 2) << 10;
        uint16_t imm7_6 = _extract_bits(offset, 6, 2) << 5;
        uint16_t imm2_1 = _extract_bits(offset, 1, 2) << 3;
        uint16_t imm5 = _extract_bits(offset, 5, 1) << 2;

        _add_val((uint16_t*)(size_t)(text + addr), imm8 | imm4_3 | imm7_6 | imm2_1 | imm5);
        break;
      }
      default:
        (*p++) = (*rel);
        printf("%08x %08x %-16s %s\n", sym->value, sym->size, riscv_type_name[relotype], strtab + sym->name);
        break;
      }
    }

    for (Elf32_Half i = 0; i < elf->shnum; ++i)
    {
      if (strstr(strndx + section[i].name, ".rela") == strndx + section[i].name)
      {
        if ((struct elf32_rela*)(memory + section[i].offset) == begin)
        {
          section[i].size = (p - begin) * sizeof(struct elf32_rela);
        }
      }
    }
  }

  // Symbol
  for (struct elf32_sym* r = symtab; r != symtab_end; ++r)
  {
    if (r->shndx == 0)
    {
      continue;
    }
    r->name = 0;
    r->info = 0;
    r->other = 0;
  }

  file = fopen(argv[2], "wb");
  if (file == NULL)
  {
    printf("%s is open failed\n", argv[2]);
    return 0;
  }
  fwrite(memory, 1, length, file);
  fclose(file);
  return 0;
}
