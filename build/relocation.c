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

static uint8_t read8(const void *P)
{
  return *(uint8_t*)P;
}

static uint16_t read16le(const void *P)
{
  return *(uint16_t*)P;
}

static uint32_t read32le(const void *P)
{
  return *(uint32_t*)P;
}

static void write8(void *P, uint8_t V)
{
  *(uint8_t*)P = V;
}

static void write16le(void *P, uint16_t V)
{
  *(uint16_t*)P = V;
}

static void write32le(void *P, uint32_t V)
{
  *(uint32_t*)P = V;
}

static uint32_t extractBits(uintptr_t v, uint32_t begin, uint32_t end)
{
  return (uint32_t)((v & (((uintptr_t)1 << (begin + 1)) - 1)) >> end);
}

static uint32_t setLO12_I(uint32_t insn, uint32_t imm)
{
  return (insn & 0xfffff) | (imm << 20);
}

static uint32_t setLO12_S(uint32_t insn, uint32_t imm)
{
  return (insn & 0x1fff07f) | (extractBits(imm, 11, 5) << 25) | (extractBits(imm, 4, 0) << 7);
}

static bool relocate(char* text, struct elf32_rela* rela, uintptr_t addr)
{
  uintptr_t val = addr + rela->addend;
  unsigned int reloctype = ELF_R_TYPE(rela->info);
  char* where = text + rela->offset;

  switch (reloctype)
  {
  case R_RISCV_CALL:
  case R_RISCV_CALL_PLT:
  case R_RISCV_BRANCH:
  case R_RISCV_JAL:
  case R_RISCV_RVC_JUMP:
  case R_RISCV_RVC_BRANCH:
    val -= rela->offset;
  }

  switch (reloctype)
  {
  case R_RISCV_CALL:
  case R_RISCV_CALL_PLT:
    relocate(text, &(elf32_rela_t){rela->offset, R_RISCV_HI20}, val);
    relocate(text, &(elf32_rela_t){rela->offset + 4, R_RISCV_LO12_I}, val);
    break;
  case R_RISCV_BRANCH:
  {
    uint32_t insn = read32le(where) & 0x1FFF07F;
    uint32_t imm12 = extractBits(val, 12, 12) << 31;
    uint32_t imm10_5 = extractBits(val, 10, 5) << 25;
    uint32_t imm4_1 = extractBits(val, 4, 1) << 8;
    uint32_t imm11 = extractBits(val, 11, 11) << 7;
    insn |= imm12 | imm10_5 | imm4_1 | imm11;

    write32le(where, insn);
    break;
  }
  case R_RISCV_JAL:
  {
    uint32_t insn = read32le(where) & 0xFFF;
    uint32_t imm20 = extractBits(val, 20, 20) << 31;
    uint32_t imm10_1 = extractBits(val, 10, 1) << 21;
    uint32_t imm11 = extractBits(val, 11, 11) << 20;
    uint32_t imm19_12 = extractBits(val, 19, 12) << 12;
    insn |= imm20 | imm10_1 | imm11 | imm19_12;

    write32le(where, insn);
    break;
  }
  case R_RISCV_HI20:
  {
    uintptr_t hi = val + 0x800;

    write32le(where, (read32le(where) & 0xFFF) | (hi & 0xFFFFF000));
    break;
  }
  case R_RISCV_LO12_I:
  {
    uintptr_t hi = (val + 0x800) >> 12;
    uintptr_t lo = val - (hi << 12);

    write32le(where, setLO12_I(read32le(where), lo & 0xFFF));
    break;
  }
  case R_RISCV_RVC_JUMP:
  {
    uint16_t insn = read16le(where) & 0xE003;
    uint16_t imm11 = extractBits(val, 11, 11) << 12;
    uint16_t imm4 = extractBits(val, 4, 4) << 11;
    uint16_t imm9_8 = extractBits(val, 9, 8) << 9;
    uint16_t imm10 = extractBits(val, 10, 10) << 8;
    uint16_t imm6 = extractBits(val, 6, 6) << 7;
    uint16_t imm7 = extractBits(val, 7, 7) << 6;
    uint16_t imm3_1 = extractBits(val, 3, 1) << 3;
    uint16_t imm5 = extractBits(val, 5, 5) << 2;
    insn |= imm11 | imm4 | imm9_8 | imm10 | imm6 | imm7 | imm3_1 | imm5;

    write16le(where, insn);
    break;
  }
  case R_RISCV_RVC_BRANCH:
  {
    uint16_t insn = read16le(where) & 0xE383;
    uint16_t imm8 = extractBits(val, 8, 8) << 12;
    uint16_t imm4_3 = extractBits(val, 4, 3) << 10;
    uint16_t imm7_6 = extractBits(val, 7, 6) << 5;
    uint16_t imm2_1 = extractBits(val, 2, 1) << 3;
    uint16_t imm5 = extractBits(val, 5, 5) << 2;
    insn |= imm8 | imm4_3 | imm7_6 | imm2_1 | imm5;

    write16le(where, insn);
    break;
  }
  default:
    return false;
    break;
  }
  return true;
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
    for (struct elf32_rela* rela = begin; rela != end; ++rela)
    {
      unsigned int addr = rela->offset;
      unsigned int reloctype = ELF_R_TYPE(rela->info);
      unsigned int relocsym = ELF_R_SYM(rela->info);
      const struct elf32_sym* sym = &symtab[relocsym];

      if (reloctype == R_RISCV_ALIGN || reloctype == R_RISCV_RELAX)
      {
        continue;
      }

#if 0
      if (reloctype == R_RISCV_LO12_I && p != begin)
      {
        struct elf32_rela* prev = (p - 1);
        struct elf32_rela* curr = rela;
        if (prev->offset + 4 == curr->offset && ELF_R_TYPE(prev->info) == R_RISCV_HI20 && ELF_R_SYM(prev->info) == relocsym)
        {
          (*rela) = (*prev);
          rela->info &= ~0xFF;
          rela->info |= R_RISCV_CALL_PLT;
          p--;
        }
      }
#endif

      if (sym->shndx == 0)
      {
        (*p++) = (*rela);
        continue;
      }

      if (sym->shndx != ndx)
      {
        (*p++) = (*rela);
        continue;
      }

#if 1
      if (reloctype == R_RISCV_HI20 || reloctype == R_RISCV_LO12_I)
      {
        (*p++) = (*rela);
        printf("%08x %08x %-16s %s\n", sym->value, sym->size, riscv_type_name[reloctype], strtab + sym->name);
        continue;
      }
#endif

      if (relocate(text, rela, sym->value) == false)
      {
        (*p++) = (*rela);
        printf("%08x %08x %-16s %s\n", sym->value, sym->size, riscv_type_name[reloctype], strtab + sym->name);
        continue;
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
