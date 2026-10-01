#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*mpoll_callback)(int, int);

extern uint32_t mpoll_gpio_intr_mask;
extern uint8_t mpoll_uart_intr_mask;

void mpoll_ctl(int fd, mpoll_callback callback);
void mpoll_wait(int timeout);
void mpoll_wakeup(void);
void mpoll_intr(int gpio, int uart);

#ifdef __cplusplus
}
#endif
