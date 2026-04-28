#include "esp32c3.h"
#include "esp_private/esp_crypto_lock_internal.h"
#include "esp_private/periph_ctrl.h"
#include "hal/aes_hal.h"
#include "hal/aes_ll.h"

static portMUX_TYPE aes_spinlock = portMUX_INITIALIZER_UNLOCKED;

void esp_aes_acquire_hardware(void) __attribute__((weak));
void esp_aes_acquire_hardware(void)
{
    portENTER_CRITICAL(&aes_spinlock);

    /* Enable AES hardware */
    AES_RCC_ATOMIC() {
        aes_ll_enable_bus_clock(true);
        aes_ll_reset_register();
    }
}

void esp_aes_release_hardware(void) __attribute__((weak));
void esp_aes_release_hardware(void)
{
    /* Disable AES hardware */
    AES_RCC_ATOMIC() {
        aes_ll_enable_bus_clock(false);
    }

    portEXIT_CRITICAL(&aes_spinlock);
}

#if SOC_AES_SUPPORTED
void esp_crypto_aes_enable_periph_clk(bool enable)
{
    AES_RCC_ATOMIC() {
        aes_ll_enable_bus_clock(enable);
        if (enable) {
            aes_ll_reset_register();
        }
#if SOC_AES_CRYPTO_DMA
        crypto_dma_ll_enable_bus_clock(enable);
        if (enable) {
            crypto_dma_ll_reset_register();
        }
#endif
    }
}
#endif
