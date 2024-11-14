#include "esp32c3.h"

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    TickType_t tick = (TickType_t)(rqtp->tv_sec * 1000 + rqtp->tv_nsec / 1000000);
    vTaskDelay(tick / portTICK_PERIOD_MS);
    return 0;
}
