#include <unistd.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include <esp_log.h>
#include <hal/gpio_ll.h>
#include <hal/uart_ll.h>

#include "mpoll.h"

#define HAVE_GPIO_INTR  0
#define HAVE_UART_INTR  1

#define gpio_ll_clear_intr_status_mask(mask) \
    gpio_ll_clear_intr_status(&GPIO, mask)
#define gpio_ll_intr_enable_mask(mask) \
{ \
    int status = mask; \
    while (status) \
    { \
        int pin = __builtin_ffs(status) - 1; \
        status &= ~BIT(pin); \
        gpio_ll_intr_enable_on_core(&GPIO, xPortGetCoreID(), pin); \
    } \
}
#define gpio_ll_intr_disable_mask(mask) \
{ \
    int status = mask; \
    while (status) \
    { \
        int pin = __builtin_ffs(status) - 1; \
        status &= ~BIT(pin); \
        gpio_ll_intr_disable(&GPIO, pin); \
    } \
}


#define uart_ll_clear_intr_status_mask(mask) \
{ \
    int status = mask; \
    while (status) \
    { \
        int uart = __builtin_ffs(status) - 1; \
        status &= ~BIT(uart); \
        uart_ll_clr_intsts_mask(UART[uart], UART_INTR_RXFIFO_TOUT); \
    } \
}
#define uart_ll_intr_enable_mask(mask) \
{ \
    int status = mask; \
    while (status) \
    { \
        int uart = __builtin_ffs(status) - 1; \
        status &= ~BIT(uart); \
        uart_ll_ena_intr_mask(UART[uart], UART_INTR_RXFIFO_TOUT); \
    } \
}
#define uart_ll_intr_disable_mask(mask) \
{ \
    int status = mask; \
    while (status) \
    { \
        int uart = __builtin_ffs(status) - 1; \
        status &= ~BIT(uart); \
        uart_ll_disable_intr_mask(UART[uart], UART_INTR_RXFIFO_TOUT); \
    } \
}

static int mpollfd_count = 0;
static sys_sem_t* mpollfd_sem = NULL;
static struct pollfd mpollfd[CONFIG_LWIP_MAX_SOCKETS];
static mpoll_callback mpollfd_callback[CONFIG_LWIP_MAX_SOCKETS];

uint32_t mpoll_gpio_intr_mask = 0;
uint8_t mpoll_uart_intr_mask = 0;

#if HAVE_GPIO_INTR
static intr_handle_t mpoll_gpio_intr_handle;
#endif

#if HAVE_UART_INTR
static uart_dev_t* const UART[SOC_UART_NUM] =
{
#if SOC_UART_NUM > 0
    &UART0,
#endif
#if SOC_UART_NUM > 1
    &UART1,
#endif
#if SOC_UART_NUM > 2
    &UART2,
#endif
#if SOC_UART_NUM > 3
    &UART3,
#endif
#if SOC_UART_NUM > 4
    &UART4,
#endif
};
static intr_handle_t mpoll_uart_intr_handle[SOC_UART_NUM];
#endif

void mpoll_ctl(int fd, mpoll_callback callback)
{
    if (callback)
    {
        for (int i = 0; i < mpollfd_count; ++i)
        {
            if (mpollfd[i].fd == fd)
            {
                mpollfd_callback[i] = callback;
                return;
            }
        }
        mpollfd[mpollfd_count].fd = fd;
        mpollfd[mpollfd_count].events = POLLIN | POLLERR | POLLHUP | POLLNVAL;
        mpollfd[mpollfd_count].revents = 0;
        mpollfd_callback[mpollfd_count] = callback;
        mpollfd_count++;
    }
    else
    {
        for (int i = 0; i < mpollfd_count; ++i)
        {
            if (mpollfd[i].fd == fd)
            {
                mpollfd_count--;
                mpollfd_callback[i] = mpollfd_callback[mpollfd_count];
                mpollfd[i].revents = mpollfd[mpollfd_count].revents;
                mpollfd[i].events = mpollfd[mpollfd_count].events;
                mpollfd[i].fd = mpollfd[mpollfd_count].fd;
                return;
            }
        }
    }
}

void mpoll_wait(int timeout)
{
    sys_sem_t* sem = sys_thread_sem_get();
#if HAVE_UART_INTR
    if (timeout)
    {
        for (int i = 0; i < SOC_UART_NUM; ++i)
        {
            if ((mpoll_uart_intr_mask & BIT(i)) && uart_ll_get_rxfifo_len(UART[i]))
            {
                timeout = 0;
                break;
            }
        }
    }
#endif
    if (timeout)
    {
        mpollfd_sem = sem;
#if HAVE_GPIO_INTR
        gpio_ll_clear_intr_status_mask(mpoll_gpio_intr_mask);
        gpio_ll_intr_enable_mask(mpoll_gpio_intr_mask);
#endif
        uart_ll_clear_intr_status_mask(mpoll_uart_intr_mask);
        uart_ll_intr_enable_mask(mpoll_uart_intr_mask);
    }
    int count = lwip_poll(mpollfd, mpollfd_count, timeout);
    if (timeout)
    {
#if HAVE_GPIO_INTR
        gpio_ll_intr_disable_mask(mpoll_gpio_intr_mask);
#endif
#if HAVE_UART_INTR
        uart_ll_intr_disable_mask(mpoll_uart_intr_mask);
#endif
        mpollfd_sem = NULL;

        xSemaphoreTake((QueueHandle_t)sem, 0);
    }
    if (count <= 0)
    {
        vTaskDelay(1);
        return;
    }

    for (int i = 0; i < mpollfd_count; ++i)
    {
        int fd = mpollfd[i].fd;
        int revents = mpollfd[i].revents;
        mpollfd[i].revents = 0;
        if (revents)
            revents = mpollfd_callback[i](fd, revents);
        if (revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            lwip_close(fd);
            mpoll_ctl(fd, NULL);
            ESP_LOGI("mpoll", "%d : %s", fd, "close");
            --i;
        }
    }
}

#if HAVE_GPIO_INTR
static void IRAM_ATTR mpoll_gpio_intr_isr(gpio_dev_t* gpio)
{
    gpio_ll_clear_intr_status_mask(mpoll_gpio_intr_mask);
    gpio_ll_intr_disable_mask(mpoll_gpio_intr_mask);
    sys_sem_t* sem = mpollfd_sem;
    if (sem)
    {
        BaseType_t taskWoken = pdFALSE;
        xSemaphoreGiveFromISR((QueueHandle_t)sem, &taskWoken);
        if (taskWoken)
        {
            portYIELD_FROM_ISR();
        }
    }
}

__attribute__((unused))
static void mpoll_gpio_intr_shutdown(void)
{
    gpio_ll_intr_disable_mask(mpoll_gpio_intr_mask);
    esp_intr_disable(mpoll_gpio_intr_handle);
    esp_intr_free(mpoll_gpio_intr_handle);
    sys_sem_t* sem = mpollfd_sem;
    if (sem)
    {
        xSemaphoreTake((QueueHandle_t)sem, 0);
    }
}

static mpoll_gpio_intr(int gpio)
{
    if (gpio >= 0 && gpio < SOC_GPIO_PIN_COUNT)
    {
        if (mpoll_gpio_intr_handle == NULL)
        {
            esp_intr_alloc(ETS_GPIO_INTR_SOURCE, 0, (intr_handler_t)mpoll_gpio_intr_isr, NULL, &mpoll_gpio_intr_handle);
//          esp_register_shutdown_handler(mpoll_gpio_intr_shutdown);
        }
        mpoll_gpio_intr_mask |= BIT(gpio);
        gpio_ll_set_intr_type(&GPIO, gpio, GPIO_INTR_ANYEDGE);
    }
}
#endif

#if HAVE_UART_INTR
static void IRAM_ATTR mpoll_uart_intr_isr(uart_dev_t* uart)
{
    uart_ll_clr_intsts_mask(uart, UART_INTR_RXFIFO_TOUT);
    uart_ll_disable_intr_mask(uart, UART_INTR_RXFIFO_TOUT);
    sys_sem_t* sem = mpollfd_sem;
    if (sem)
    {
        BaseType_t taskWoken = pdFALSE;
        xSemaphoreGiveFromISR((QueueHandle_t)sem, &taskWoken);
        if (taskWoken)
        {
            portYIELD_FROM_ISR();
        }
    }
}

static void mpoll_uart_intr(int uart)
{
//  if (uart >= 0 && uart < SOC_UART_NUM)   // TODO
    if (uart >= 1 && uart < SOC_UART_NUM)
    {
        if (mpoll_uart_intr_handle[uart] == NULL)
        {
            int source = 0;
            switch (uart)
            {
#if SOC_UART_NUM > 0
            case 0: source = ETS_UART0_INTR_SOURCE; break;
#endif
#if SOC_UART_NUM > 1
            case 1: source = ETS_UART1_INTR_SOURCE; break;
#endif
#if SOC_UART_NUM > 2
            case 2: source = ETS_UART2_INTR_SOURCE; break;
#endif
#if SOC_UART_NUM > 3
            case 3: source = ETS_UART3_INTR_SOURCE; break;
#endif
#if SOC_UART_NUM > 4
            case 4: source = ETS_UART4_INTR_SOURCE; break;
#endif
            }
            esp_intr_alloc(source, 0, (intr_handler_t)mpoll_uart_intr_isr, (void*)UART[uart], &mpoll_uart_intr_handle[uart]);
        }
        mpoll_uart_intr_mask |= BIT(uart);
        uart_ll_set_rx_tout(UART[uart], 10);
    }
}
#endif

void mpoll_intr(int gpio, int uart)
{
#if HAVE_GPIO_INTR
    mpoll_gpio_intr(gpio);
#endif
#if HAVE_UART_INTR
    mpoll_uart_intr(uart);
#endif
}
