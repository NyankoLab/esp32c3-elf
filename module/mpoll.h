#pragma once

#include <hal/gpio_ll.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*mpoll_callback)(int, int);

void mpoll_ctl(int fd, mpoll_callback callback);
void mpoll_wait(int timeout);

extern uint32_t mpoll_gpio_mask;
#define gpio_ll_intr_enable_mask(mask) { \
  int status = mask; \
  while (status) { \
    int pin = __builtin_ffs(status) - 1; \
    status &= ~BIT(pin); \
    gpio_ll_intr_enable_on_core(&GPIO, 0, pin); \
  } \
}
#define gpio_ll_intr_disable_mask(mask) { \
  int status = mask; \
  while (status) { \
    int pin = __builtin_ffs(status) - 1; \
    status &= ~BIT(pin); \
    gpio_ll_intr_disable(&GPIO, pin); \
  } \
}

void mpoll_isr(int pin);

#ifdef __cplusplus
}
#endif
