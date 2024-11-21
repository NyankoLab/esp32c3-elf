#include "esp32c3.h"
#include "esp_private/periph_ctrl.h"
#include "hal/sha_hal.h"

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
