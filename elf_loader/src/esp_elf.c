/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/param.h>

#include "esp_elf.h"
#include "esp_log.h"

#include "private/elf_symbol.h"
#include "private/elf_platform.h"

#define stype(_s, _t)               ((_s)->type == (_t))
#define sflags(_s, _f)              (((_s)->flags & (_f)) == (_f))
#define TAG                         "ELF"

/**
 * @brief Map symbol's address of ELF to physic space.
 *
 * @param elf - ELF object pointer
 * @param sym - ELF symbol address
 * 
 * @return ESP_OK if sucess or other if failed.
 */
uintptr_t esp_elf_map_sym(esp_elf_t *elf, uintptr_t sym)
{
    for (int i = 0; i < ELF_SECS; i++) {
        if ((sym >= elf->sec[i].v_addr) &&
                (sym < (elf->sec[i].v_addr + elf->sec[i].size))) {
            return sym - elf->sec[i].v_addr + elf->sec[i].addr;
        }
    }

    return 0;
}

/**
 * @brief Initialize ELF object.
 *
 * @param elf - ELF object pointer
 * 
 * @return ESP_OK if sucess or other if failed.
 */
int esp_elf_init(esp_elf_t *elf)
{
    ESP_LOGI(TAG, "ELF loader version: %d.%d.%d", ELF_LOADER_VER_MAJOR, ELF_LOADER_VER_MINOR, ELF_LOADER_VER_PATCH);

    if (!elf) {
        return -EINVAL;
    }

    memset(elf, 0, sizeof(esp_elf_t));

    return 0;
}

/**
 * @brief Decode and relocate ELF data.
 *
 * @param elf - ELF object pointer
 * @param pbuf - ELF data buffer
 * 
 * @return ESP_OK if sucess or other if failed.
 */
int esp_elf_relocate(esp_elf_t *elf, bool(*read)(void *, size_t, bool, const void *, size_t), const void *data)
{
    if (!elf || !read || !data) {
        return -EINVAL;
    }

    elf32_hdr_t hdr;
    elf32_shdr_t shdr;
    if (!read(&hdr, sizeof(hdr), false, data, 0) ||
        !read(&shdr, sizeof(shdr), false, data, hdr.shoff + sizeof(elf32_shdr_t) * hdr.shstrndx)) {
        return -ENOEXEC;
    }
    Elf32_Off shstrab = shdr.offset;

    /* Calculate ELF image size */

    for (int i = 0; i < hdr.shnum; i++) {
        char name[128];
        if (!read(&shdr, sizeof(shdr), false, data, hdr.shoff + sizeof(elf32_shdr_t) * i) ||
            !read(name, sizeof(name), true, data, shstrab + shdr.name)) {
            return -ENOEXEC;
        }
        if (stype(&shdr, SHT_PROGBITS) && sflags(&shdr, SHF_ALLOC)) {
            if (sflags(&shdr, SHF_EXECINSTR) && !strcmp(ELF_TEXT, name)) {
                ESP_LOGI(TAG, ".text   sec addr=0x%08x size=0x%08x offset=0x%08x",
                         shdr.addr, shdr.size, shdr.offset);

                elf->sec[ELF_SEC_TEXT].v_addr  = shdr.addr;
                elf->sec[ELF_SEC_TEXT].size    = ELF_ALIGN(shdr.size);
                elf->sec[ELF_SEC_TEXT].offset  = shdr.offset;

                ESP_LOGI(TAG, ".text   offset is 0x%lx size is 0x%x",
                         elf->sec[ELF_SEC_TEXT].offset,
                         elf->sec[ELF_SEC_TEXT].size);
            } else if (sflags(&shdr, SHF_WRITE) && !strcmp(ELF_DATA, name)) {
                ESP_LOGI(TAG, ".data   sec addr=0x%08x size=0x%08x offset=0x%08x",
                         shdr.addr, shdr.size, shdr.offset);

                elf->sec[ELF_SEC_DATA].v_addr  = shdr.addr;
                elf->sec[ELF_SEC_DATA].size    = shdr.size;
                elf->sec[ELF_SEC_DATA].offset  = shdr.offset;

                ESP_LOGI(TAG, ".data   offset is 0x%lx size is 0x%x",
                         elf->sec[ELF_SEC_DATA].offset,
                         elf->sec[ELF_SEC_DATA].size);
            } else if (!strcmp(ELF_RODATA, name)) {
                ESP_LOGI(TAG, ".rodata sec addr=0x%08x size=0x%08x offset=0x%08x",
                         shdr.addr, shdr.size, shdr.offset);

                elf->sec[ELF_SEC_RODATA].v_addr  = shdr.addr;
                elf->sec[ELF_SEC_RODATA].size    = shdr.size;
                elf->sec[ELF_SEC_RODATA].offset  = shdr.offset;

                ESP_LOGI(TAG, ".rodata offset is 0x%lx size is 0x%x",
                         elf->sec[ELF_SEC_RODATA].offset,
                         elf->sec[ELF_SEC_RODATA].size);
            } else if (!strcmp(ELF_DATA_REL_RO, name)) {
                ESP_LOGI(TAG, ".data.rel.ro sec addr=0x%08x size=0x%08x offset=0x%08x",
                         shdr.addr, shdr.size, shdr.offset);

                elf->sec[ELF_SEC_DRLRO].v_addr  = shdr.addr;
                elf->sec[ELF_SEC_DRLRO].size    = shdr.size;
                elf->sec[ELF_SEC_DRLRO].offset  = shdr.offset;

                ESP_LOGI(TAG, ".data.rel.ro offset is 0x%lx size is 0x%x",
                         elf->sec[ELF_SEC_DRLRO].offset,
                         elf->sec[ELF_SEC_DRLRO].size);
            }
        } else if (stype(&shdr, SHT_NOBITS) && sflags(&shdr, SHF_ALLOC | SHF_WRITE) && !strcmp(ELF_BSS, name)) {
            ESP_LOGI(TAG, ".bss    sec addr=0x%08x size=0x%08x offset=0x%08x",
                     shdr.addr, shdr.size, shdr.offset);

            elf->sec[ELF_SEC_BSS].v_addr  = shdr.addr;
            elf->sec[ELF_SEC_BSS].size    = shdr.size;
            elf->sec[ELF_SEC_BSS].offset  = shdr.offset;

            ESP_LOGI(TAG, ".bss    offset is 0x%lx size is 0x%x",
                     elf->sec[ELF_SEC_BSS].offset,
                     elf->sec[ELF_SEC_BSS].size);
        }
    }

    /* No .text on image */

    if (!elf->sec[ELF_SEC_TEXT].size) {
        return -EINVAL;
    }

    elf->ptext = esp_elf_malloc(elf->sec[ELF_SEC_TEXT].size, true);
    if (!elf->ptext) {
        return -ENOMEM;
    }

    uint32_t size = elf->sec[ELF_SEC_DATA].size +
           elf->sec[ELF_SEC_RODATA].size +
           elf->sec[ELF_SEC_BSS].size +
           elf->sec[ELF_SEC_DRLRO].size;
    if (size) {
        elf->pdata = esp_elf_malloc(size, false);
        if (!elf->pdata) {
            return -ENOMEM;
        }
    }

    /* Dump ".text" from ELF to excutable space memory */

    elf->sec[ELF_SEC_TEXT].addr = (Elf32_Addr)elf->ptext;
    if (!read(elf->ptext, elf->sec[ELF_SEC_TEXT].size, false, data, elf->sec[ELF_SEC_TEXT].offset)) {
        return -ENOEXEC;
    }

#ifdef CONFIG_ELF_LOADER_SET_MMU
    if (esp_elf_arch_init_mmu(elf)) {
        return -EIO;
    }
#endif

    /**
     * Dump ".data", ".rodata" and ".bss" from ELF to R/W space memory.
     *
     * Todo: Dump ".rodata" to rodata section by MMU/MPU.
     */

    if (size) {
        uint8_t *pdata = elf->pdata;

        if (elf->sec[ELF_SEC_DATA].size) {
            elf->sec[ELF_SEC_DATA].addr = (uint32_t)pdata;

            if (!read(pdata, elf->sec[ELF_SEC_DATA].size, false, data, elf->sec[ELF_SEC_DATA].offset)) {
                return -ENOEXEC;
            }

            pdata += elf->sec[ELF_SEC_DATA].size;
        }

        if (elf->sec[ELF_SEC_RODATA].size) {
            elf->sec[ELF_SEC_RODATA].addr = (uint32_t)pdata;

            if (!read(pdata, elf->sec[ELF_SEC_RODATA].size, false, data, elf->sec[ELF_SEC_RODATA].offset)) {
                return -ENOEXEC;
            }

            pdata += elf->sec[ELF_SEC_RODATA].size;
        }

        if (elf->sec[ELF_SEC_DRLRO].size) {
            elf->sec[ELF_SEC_DRLRO].addr = (uint32_t)pdata;

            if (!read(pdata, elf->sec[ELF_SEC_DRLRO].size, false, data, elf->sec[ELF_SEC_DRLRO].offset)) {
                return -ENOEXEC;
            }

            pdata += elf->sec[ELF_SEC_DRLRO].size;
        }

        if (elf->sec[ELF_SEC_BSS].size) {
            elf->sec[ELF_SEC_BSS].addr = (uint32_t)pdata;
            memset(pdata, 0, elf->sec[ELF_SEC_BSS].size);
        }
    }

    /* Set ELF entry */

    uint32_t entry = hdr.entry + elf->sec[ELF_SEC_TEXT].addr -
                          elf->sec[ELF_SEC_TEXT].v_addr;

#ifdef CONFIG_ELF_LOADER_CACHE_OFFSET
    elf->entry = (void *)elf_remap_text(elf, (uintptr_t)entry);
#else
    elf->entry = (void *)entry;
#endif

    /* Relocation section data */

    for (int i = 0; i < hdr.shnum; i++) {
        if (!read(&shdr, sizeof(shdr), false, data, hdr.shoff + sizeof(elf32_shdr_t) * i)) {
            return -ENOEXEC;
        }
        if (stype(&shdr, SHT_RELA)) {
            char name[128];
            int nr_reloc = shdr.size / sizeof(elf32_rela_t);
            if (!read(name, sizeof(name), true, data, shstrab + shdr.name)) {
                return -ENOEXEC;
            }
            ESP_LOGI(TAG, "Section %s has %d symbol tables", name, nr_reloc);

            Elf32_Off offset = 0;
            if (strstr(name, ELF_TEXT)) {
                offset = elf->sec[ELF_SEC_TEXT].addr;
            } else if (strstr(name, ELF_DATA)) {
                offset = elf->sec[ELF_SEC_DATA].addr;
            } else if (strstr(name, ELF_RODATA)) {
                offset = elf->sec[ELF_SEC_RODATA].addr;
            } else if (strstr(name, ELF_DATA_REL_RO)) {
                offset = elf->sec[ELF_SEC_DRLRO].addr;
            } else if (strstr(name, ELF_BSS)) {
                offset = elf->sec[ELF_SEC_BSS].addr;
            } else {
                ESP_LOGE(TAG, "Unsupported section %s", name);
                return -ENOEXEC;
            }

            Elf32_Off rela = shdr.offset;
            if (!read(&shdr, sizeof(shdr), false, data, hdr.shoff + sizeof(elf32_shdr_t) * shdr.link)) {
                return -ENOEXEC;
            }
            Elf32_Off symtab = shdr.offset;
            if (!read(&shdr, sizeof(shdr), false, data, hdr.shoff + sizeof(elf32_shdr_t) * shdr.link)) {
                return -ENOEXEC;
            }
            Elf32_Off strtab = shdr.offset;

            for (int i = 0; i < nr_reloc; i++) {
                elf32_rela_t rela_buf;
                if (!read(&rela_buf, sizeof(rela_buf), false, data, rela + sizeof(elf32_rela_t) * i)) {
                    return -ENOEXEC;
                }
                rela_buf.offset += offset;

                elf32_sym_t sym;
                if (!read(&sym, sizeof(sym), false, data, symtab + sizeof(elf32_sym_t) * ELF_R_SYM(rela_buf.info))) {
                    return -ENOEXEC;
                }

                uintptr_t addr = 0;
                switch (sym.shndx) {
                case SHN_COMMON:
                    return -EINVAL;
                case SHN_ABS:
                    addr = sym.value;
                    break;
                case SHN_UNDEF:
                    if (!read(name, sizeof(name), true, data, strtab + sym.name)) {
                        return -ENOEXEC;
                    }
                    addr = elf_find_sym(name);
                    if (!addr) {
                        ESP_LOGE(TAG, "Can't find symbol %s", name);

                        for (int j = i + 1; j < nr_reloc; j++) {
                            if (read(&rela_buf, sizeof(rela_buf), false, data, rela + sizeof(elf32_rela_t) * j) &&
                                read(&sym, sizeof(sym), false, data, symtab + sizeof(elf32_sym_t) * ELF_R_SYM(rela_buf.info))) {
                                if (sym.shndx == SHN_UNDEF) {
                                    if (read(name, sizeof(name), true, data, strtab + sym.name)) {
                                        addr = elf_find_sym(name);
                                        if (!addr) {
                                            ESP_LOGE(TAG, "Can't find symbol %s", name);
                                        }
                                    }
                                }
                            }
                        }

                        return -ENOSYS;
                    }
                    ESP_LOGD(TAG, "Found function %s addr=%x", name, addr);
                    break;
                default:
                    if (sym.shndx >= hdr.shnum) {
                        ESP_LOGE(TAG, "Out of bound section %d", sym.shndx);
                        return -ENOEXEC;
                    }
                    if (!read(&shdr, sizeof(shdr), false, data, hdr.shoff + sizeof(elf32_shdr_t) * sym.shndx) ||
                        !read(name, sizeof(name), true, data, shstrab + shdr.name)) {
                        return -ENOEXEC;
                    }
                    if (!strcmp(ELF_TEXT, name)) {
                        addr = sym.value + elf->sec[ELF_SEC_TEXT].addr;
                    } else if (!strcmp(ELF_DATA, name)) {
                        addr = sym.value + elf->sec[ELF_SEC_DATA].addr;
                    } else if (!strcmp(ELF_RODATA, name)) {
                        addr = sym.value + elf->sec[ELF_SEC_RODATA].addr;
                    } else if (!strcmp(ELF_DATA_REL_RO, name)) {
                        addr = sym.value + elf->sec[ELF_SEC_DRLRO].addr;
                    } else if (!strcmp(ELF_BSS, name)) {
                        addr = sym.value + elf->sec[ELF_SEC_BSS].addr;
                } else {
                        ESP_LOGE(TAG, "Unsupported section %s %d", name, sym.value);
                        return -ENOEXEC;
                    }
                    ESP_LOGD(TAG, "Found value %d addr=%x", sym.value, addr);
                    break;
                }

                esp_elf_arch_relocate(elf, &rela_buf, addr);
            }
        }
    }

#ifdef CONFIG_ELF_LOADER_LOAD_PSRAM
    esp_elf_arch_flush();
#endif

    return 0;
}

/**
 * @brief Request running relocated ELF function.
 *
 * @param elf  - ELF object pointer
 * @param opt  - Request options
 * @param argc - Arguments number
 * @param argv - Arguments value array
 * 
 * @return ESP_OK if sucess or other if failed.
 */
int esp_elf_request(esp_elf_t *elf, int opt, int argc, char *argv[])
{
    elf->entry(argc, argv);

    return 0;
}

/**
 * @brief Deinitialize ELF object.
 *
 * @param elf - ELF object pointer
 * 
 * @return None
 */
void esp_elf_deinit(esp_elf_t *elf)
{
    if (elf->pdata) {
        esp_elf_free(elf->pdata);
        elf->pdata = NULL;
    }

    if (elf->ptext) {
        esp_elf_free(elf->ptext);
        elf->ptext = NULL;
    }

#ifdef CONFIG_ELF_LOADER_SET_MMU
    esp_elf_arch_deinit_mmu(elf);
#endif
}

/**
 * @brief Print header description information of ELF.
 *
 * @param pbuf - ELF data buffer
 * 
 * @return None
 */
void esp_elf_print_hdr(const uint8_t *pbuf)
{
    const char *s_bits, *s_endian;
    const elf32_hdr_t *hdr = (const elf32_hdr_t *)pbuf;

    switch (hdr->ident[4]) {
    case 1:
        s_bits = "32-bit";
        break;
    case 2:
        s_bits = "64-bit";
        break;
    default:
        s_bits = "invalid bits";
        break;
    }

    switch (hdr->ident[5]) {
    case 1:
        s_endian = "little-endian";
        break;
    case 2:
        s_endian = "big-endian";
        break;
    default:
        s_endian = "invalid endian";
        break;
    }

    if (hdr->ident[0] == 0x7f) {
        ESP_LOGI(TAG, "%-40s %c%c%c", "Class:",                     hdr->ident[1], hdr->ident[2], hdr->ident[3]);
    }

    ESP_LOGI(TAG, "%-40s %s, %s", "Format:",                        s_bits, s_endian);
    ESP_LOGI(TAG, "%-40s %x", "Version(current):",                  hdr->ident[6]);

    ESP_LOGI(TAG, "%-40s %d", "Type:",                              hdr->type);
    ESP_LOGI(TAG, "%-40s %d", "Machine:",                           hdr->machine);
    ESP_LOGI(TAG, "%-40s %x", "Version:",                           hdr->version);
    ESP_LOGI(TAG, "%-40s %x", "Entry point address:",               hdr->entry);
    ESP_LOGI(TAG, "%-40s %x", "Start of program headers:",          hdr->phoff);
    ESP_LOGI(TAG, "%-40s %d", "Start of section headers:",          hdr->shoff);
    ESP_LOGI(TAG, "%-40s 0x%x", "Flags:",                           hdr->flags);
    ESP_LOGI(TAG, "%-40s %d", "Size of this header(bytes):",        hdr->ehsize);
    ESP_LOGI(TAG, "%-40s %d", "Size of program headers(bytes):",    hdr->phentsize);
    ESP_LOGI(TAG, "%-40s %d", "Number of program headers:",         hdr->phnum);
    ESP_LOGI(TAG, "%-40s %d", "Size of section headers(bytes):",    hdr->shentsize);
    ESP_LOGI(TAG, "%-40s %d", "Number of section headers:",         hdr->shnum);
    ESP_LOGI(TAG, "%-40s %d", "Section header string table i:",     hdr->shstrndx);
}

/**
 * @brief Print section header description information of ELF.
 *
 * @param pbuf - ELF data buffer
 * 
 * @return None
 */
void esp_elf_print_shdr(const uint8_t *pbuf)
{
    const elf32_hdr_t *phdr = (const elf32_hdr_t *)pbuf;
    const elf32_shdr_t *pshdr = (const elf32_shdr_t *)((size_t)pbuf + phdr->shoff);

    for (int i = 0; i < phdr->shnum; i++) {
        ESP_LOGI(TAG, "%-40s %d", "name:",                          pshdr->name);
        ESP_LOGI(TAG, "%-40s %d", "type:",                          pshdr->type);
        ESP_LOGI(TAG, "%-40s 0x%x", "flags:",                       pshdr->flags);
        ESP_LOGI(TAG, "%-40s %x", "addr",                           pshdr->addr);
        ESP_LOGI(TAG, "%-40s %x", "offset:",                        pshdr->offset);
        ESP_LOGI(TAG, "%-40s %d", "size",                           pshdr->size);
        ESP_LOGI(TAG, "%-40s 0x%x", "link",                         pshdr->link);
        ESP_LOGI(TAG, "%-40s %d", "addralign",                      pshdr->addralign);
        ESP_LOGI(TAG, "%-40s %d", "entsize",                        pshdr->entsize);

        pshdr = (const elf32_shdr_t *)((size_t)pshdr + sizeof(elf32_shdr_t));
    }
}

/**
 * @brief Print section information of ELF.
 *
 * @param pbuf - ELF data buffer
 * 
 * @return None
 */
void esp_elf_print_sec(esp_elf_t *elf)
{
    const char *sec_names[ELF_SECS] = {
        "text", "bss", "data", "rodata"
    };

    for (int i = 0; i < ELF_SECS; i++) {
        ESP_LOGI(TAG, "%s:   0x%08x size 0x%08x", sec_names[i], elf->sec[i].addr, elf->sec[i].size);
    }

    ESP_LOGI(TAG, "entry:  %p", elf->entry);
}
