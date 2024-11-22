/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "private/elf_types.h"

#define ELF_LOADER_VER_MAJOR        0
#define ELF_LOADER_VER_MINOR        1
#define ELF_LOADER_VER_PATCH        0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Map symbol's address of ELF to physic space.
 *
 * @param elf - ELF object pointer
 * @param sym - ELF symbol address
 * 
 * @return Mapped physic address.
 */
uintptr_t esp_elf_map_sym(esp_elf_t *elf, uintptr_t sym);

/**
 * @brief Initialize ELF object.
 *
 * @param elf - ELF object pointer
 * 
 * @return ESP_OK if sucess or other if failed.
 */
int esp_elf_init(esp_elf_t *elf);

/**
 * @brief Decode and relocate ELF data.
 *
 * @param elf - ELF object pointer
 * @param read - ELF read function
 * @param data - ELF data buffer
 *
 * @return ESP_OK if sucess or other if failed.
 */
int esp_elf_relocate(esp_elf_t *elf, bool(*read)(void *, size_t, bool, const void *, size_t), const void *data);

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
int esp_elf_request(esp_elf_t *elf, int opt, int argc, char *const argv[]);

/**
 * @brief Deinitialize ELF object.
 *
 * @param elf - ELF object pointer
 * 
 * @return None
 */
void esp_elf_deinit(esp_elf_t *elf);

/**
 * @brief Print header description information of ELF.
 *
 * @param pbuf - ELF data buffer
 * 
 * @return None
 */
void esp_elf_print_hdr(bool(*read)(void *, size_t, bool, const void *, size_t), const void *data);

/**
 * @brief Print section header description information of ELF.
 *
 * @param pbuf - ELF data buffer
 * 
 * @return None
 */
void esp_elf_print_shdr(bool(*read)(void *, size_t, bool, const void *, size_t), const void *data);

/**
 * @brief Print section information of ELF.
 *
 * @param pbuf - ELF data buffer
 * 
 * @return None
 */
void esp_elf_print_sec(esp_elf_t *elf);

#ifdef __cplusplus
}
#endif
