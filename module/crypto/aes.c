#include "esp32c3.h"
#include "esp_private/periph_ctrl.h"
#include "hal/aes_hal.h"

static portMUX_TYPE aes_spinlock = portMUX_INITIALIZER_UNLOCKED;

void esp_aes_acquire_hardware(void) __attribute__((weak));
void esp_aes_acquire_hardware(void)
{
    portENTER_CRITICAL(&aes_spinlock);

    /* Enable AES hardware */
    periph_module_enable(PERIPH_AES_MODULE);
}

void esp_aes_release_hardware(void) __attribute__((weak));
void esp_aes_release_hardware(void)
{
    /* Disable AES hardware */
    periph_module_disable(PERIPH_AES_MODULE);
    
    portEXIT_CRITICAL(&aes_spinlock);
}
