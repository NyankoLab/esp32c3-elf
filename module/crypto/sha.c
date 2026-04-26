#include "esp32c3.h"
#include "esp_private/esp_crypto_lock_internal.h"
#include "esp_private/periph_ctrl.h"
#include "hal/sha_hal.h"
#include "hal/sha_ll.h"

static portMUX_TYPE sha_spinlock = portMUX_INITIALIZER_UNLOCKED;

void esp_sha_acquire_hardware(void) __attribute__((weak));
void esp_sha_acquire_hardware(void)
{
    portENTER_CRITICAL(&sha_spinlock);

    /* Enable SHA hardware */
    periph_module_enable(PERIPH_SHA_MODULE);
}

void esp_sha_release_hardware(void) __attribute__((weak));
void esp_sha_release_hardware(void)
{
    /* Disable SHA hardware */
    periph_module_disable(PERIPH_SHA_MODULE);

    portEXIT_CRITICAL(&sha_spinlock);
}

#if SOC_SHA_SUPPORTED
void esp_crypto_sha_enable_periph_clk(bool enable)
{
    SHA_RCC_ATOMIC() {
        sha_ll_enable_bus_clock(enable);
        if (enable) {
            sha_ll_reset_register();
        }
#if SOC_SHA_CRYPTO_DMA
        crypto_dma_ll_enable_bus_clock(enable);
        if (enable) {
            crypto_dma_ll_reset_register();
        }
#endif
    }
}
#endif


#if defined(SOC_SHA_SUPPORTED) || defined(SOC_AES_SUPPORTED)
/* Single lock for SHA and AES, sharing a reserved GDMA channel */
static portMUX_TYPE s_crypto_sha_aes_lock = portMUX_INITIALIZER_UNLOCKED;

void esp_crypto_sha_aes_lock_acquire(void)
{
    portENTER_CRITICAL(&s_crypto_sha_aes_lock);
}

void esp_crypto_sha_aes_lock_release(void)
{
    portEXIT_CRITICAL(&s_crypto_sha_aes_lock);
}
#endif /* defined(SOC_SHA_SUPPORTED) || defined(SOC_AES_SUPPORTED) */
