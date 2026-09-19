#pragma once

#include <hal/gpio_ll.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*mpoll_callback)(int, int);

extern uint32_t mpoll_intr_gpio_mask;
extern uint8_t mpoll_intr_uart_mask;

void mpoll_ctl(int fd, mpoll_callback callback);
void mpoll_wait(int timeout);
void mpoll_intr(int gpio, int uart);

#ifdef __cplusplus
}
#endif
