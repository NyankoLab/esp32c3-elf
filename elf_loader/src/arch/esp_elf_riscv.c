/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <sys/errno.h>
#include "esp_elf.h"
#include "esp_log.h"
#include "private/elf_platform.h"

/** @brief RISC-V relocations defined by the ABIs */

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

static const char *TAG = "elf_arch";

static intptr_t SignExtend(uintptr_t X)
{
#if __LP64__
    return (X << (64 - B)) >> (64 - B);
#else
    return X;
#endif
}

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

#if __LP64__
static uint64_t read64le(const void *P)
{
    return *(uint64_t*)P;
}
#endif

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

#if __LP64__
static void write64le(void *P, uint64_t V)
{
    *(uint64_t*)P = V;
}
#endif

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

/**
 * @brief Relocates target architecture symbol of ELF
 *
 * @param elf  - ELF object pointer
 * @param rela - Relocated symbol data
 * @param sym  - ELF symbol table
 * @param addr - Jumping target address
 * 
 * @return ESP_OK if sucess or other if failed.
 */
int esp_elf_arch_relocate(esp_elf_t *elf, const elf32_rela_t *rela, uint32_t addr)
{
    uint32_t val;
    uint32_t *where;

    assert(elf && rela);

    switch (ELF_R_TYPE(rela->info)) {
    case R_RISCV_NONE:
    case R_RISCV_ALIGN:
    case R_RISCV_RELAX:
        return 0;
    }

    val = addr + rela->addend;
    where = (uint32_t *)esp_elf_map_sym(elf, rela->offset);

    switch (ELF_R_TYPE(rela->info)) {
    case R_RISCV_JAL:
    case R_RISCV_BRANCH:
    case R_RISCV_PCREL_HI20:
    case R_RISCV_PCREL_LO12_I:
    case R_RISCV_PCREL_LO12_S:
    case R_RISCV_RVC_BRANCH:
    case R_RISCV_RVC_JUMP:
//  case R_RISCV_32_PCREL:
    case R_RISCV_CALL:
    case R_RISCV_CALL_PLT:
//  case R_RISCV_PLT32:
        val -= (uint32_t)where;
        break;
    }

    ESP_LOGD(TAG, "where=%p addr=%x offset=%x val=%x", where, (int)addr, (int)rela->offset, (int)val);

    switch (ELF_R_TYPE(rela->info)) {
    case R_RISCV_32:
        write32le(where, val);
        break;
#if __LP64__
    case R_RISCV_64:
        write64le(where, val);
        break;
#endif
    case R_RISCV_RVC_BRANCH: {
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
    case R_RISCV_RVC_JUMP: {
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
    case R_RISCV_RVC_LUI: {
        intptr_t imm = SignExtend(val + 0x800) >> 12;

        if (imm == 0) { // `c.lui rd, 0` is illegal, convert to `c.li rd, 0`
            write16le(where, (read16le(where) & 0x0F83) | 0x4000);
        } else {
            uint16_t imm17 = extractBits(val + 0x800, 17, 17) << 12;
            uint16_t imm16_12 = extractBits(val + 0x800, 16, 12) << 2;
            write16le(where, (read16le(where) & 0xEF83) | imm17 | imm16_12);
        }
        break;
    }
    case R_RISCV_JAL: {
        uint32_t insn = read32le(where) & 0xFFF;
        uint32_t imm20 = extractBits(val, 20, 20) << 31;
        uint32_t imm10_1 = extractBits(val, 10, 1) << 21;
        uint32_t imm11 = extractBits(val, 11, 11) << 20;
        uint32_t imm19_12 = extractBits(val, 19, 12) << 12;
        insn |= imm20 | imm10_1 | imm11 | imm19_12;

        write32le(where, insn);
        break;
    }
    case R_RISCV_BRANCH: {
        uint32_t insn = read32le(where) & 0x1FFF07F;
        uint32_t imm12 = extractBits(val, 12, 12) << 31;
        uint32_t imm10_5 = extractBits(val, 10, 5) << 25;
        uint32_t imm4_1 = extractBits(val, 4, 1) << 8;
        uint32_t imm11 = extractBits(val, 11, 11) << 7;
        insn |= imm12 | imm10_5 | imm4_1 | imm11;

        write32le(where, insn);
        break;
    }
    case R_RISCV_CALL:
    case R_RISCV_CALL_PLT: {
        esp_elf_arch_relocate(elf, &(elf32_rela_t){rela->offset, R_RISCV_PCREL_HI20}, addr);
        esp_elf_arch_relocate(elf, &(elf32_rela_t){rela->offset + 4, R_RISCV_PCREL_LO12_I}, addr + 4);
        break;
    }
    case R_RISCV_GOT_HI20:
    case R_RISCV_PCREL_HI20:
//  case R_RISCV_TLSDESC_HI20:
    case R_RISCV_TLS_GD_HI20:
    case R_RISCV_TLS_GOT_HI20:
    case R_RISCV_TPREL_HI20:
    case R_RISCV_HI20: {
        uintptr_t hi = val + 0x800;

        write32le(where, (read32le(where) & 0xFFF) | (hi & 0xFFFFF000));
        break;
    }
    case R_RISCV_PCREL_LO12_I:
//  case R_RISCV_TLSDESC_LOAD_LO12:
//  case R_RISCV_TLSDESC_ADD_LO12:
    case R_RISCV_TPREL_LO12_I:
    case R_RISCV_LO12_I: {
        uintptr_t hi = (val + 0x800) >> 12;
        uintptr_t lo = val - (hi << 12);

        write32le(where, setLO12_I(read32le(where), lo & 0xFFF));
        break;
    }
    case R_RISCV_PCREL_LO12_S:
    case R_RISCV_TPREL_LO12_S:
    case R_RISCV_LO12_S: {
        uintptr_t hi = (val + 0x800) >> 12;
        uintptr_t lo = val - (hi << 12);

        write32le(where, setLO12_S(read32le(where), lo));
        break;
    }
    case R_RISCV_ADD8:
        write8(where, read8(where) + val);
        break;
    case R_RISCV_ADD16:
        write16le(where, read16le(where) + val);
        break;
    case R_RISCV_ADD32:
        write32le(where, read32le(where) + val);
        break;
#if __LP64__
    case R_RISCV_ADD64:
        write64le(where, read64le(where) + val);
        break;
#endif
//  case R_RISCV_SUB6:
//      write8(where, (read8(where) & 0xC0) | (((read8(where) & 0x3F) - val) & 0x3F));
//      break;
    case R_RISCV_SUB8:
        write8(where, read8(where) - val);
        break;
    case R_RISCV_SUB16:
        write16le(where, read16le(where) - val);
        break;
    case R_RISCV_SUB32:
        write32le(where, read32le(where) - val);
        break;
#if __LP64__
    case R_RISCV_SUB64:
        write64le(where, read64le(where) - val);
        break;
#endif
//  case R_RISCV_SET6:
//      write8(where, (read8(where) & 0xC0) | (val & 0x3F));
//      break;
//  case R_RISCV_SET8:
//      write8(where, val);
//      break;
//  case R_RISCV_SET16:
//      write16le(where, val);
//      break;
//  case R_RISCV_SET32:
//  case R_RISCV_32_PCREL:
//  case R_RISCV_PLT32:
    case R_RISCV_GOT32_PCREL:
        write32le(where, val);
        break;
    default:
        ESP_LOGE(TAG, "info=%d is not supported", ELF_R_TYPE(rela->info));
        return -EINVAL;
    }

    return 0;
}
