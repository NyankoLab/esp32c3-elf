#include <unistd.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include <esp_log.h>
#include <hal/gpio_ll.h>
#include <hal/uart_ll.h>

#include "mpoll.h"

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

static int mpollfd_count = 0;
static struct pollfd mpollfd[CONFIG_LWIP_MAX_SOCKETS];
static mpoll_callback mpollfd_callback[CONFIG_LWIP_MAX_SOCKETS];

uint32_t mpoll_intr_gpio_mask = 0;
uint8_t mpoll_intr_uart_mask = 0;
static sys_sem_t* mpollfd_sem = NULL;
static intr_handle_t mpoll_intr_handle;

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
    if (timeout)
    {
        if ((mpoll_intr_uart_mask & BIT(0)) && uart_ll_get_rxfifo_len(&UART0))
            timeout = 0;
        else if ((mpoll_intr_uart_mask & BIT(1)) && uart_ll_get_rxfifo_len(&UART1))
            timeout = 0;
    }
    if (timeout)
    {
        mpollfd_sem = sem;
        gpio_ll_clear_intr_status(&GPIO, mpoll_intr_gpio_mask);
        gpio_ll_intr_enable_mask(mpoll_intr_gpio_mask);
    }
    int count = lwip_poll(mpollfd, mpollfd_count, timeout);
    if (timeout)
    {
        gpio_ll_intr_disable_mask(mpoll_intr_gpio_mask);
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

static void IRAM_ATTR mpoll_intr_isr(void* arg)
{
    gpio_ll_clear_intr_status(&GPIO, mpoll_intr_gpio_mask);
    gpio_ll_intr_disable_mask(mpoll_intr_gpio_mask);
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
static void mpoll_intr_shutdown(void)
{
    gpio_ll_intr_disable_mask(mpoll_intr_gpio_mask);
    esp_intr_disable(mpoll_intr_handle);
    esp_intr_free(mpoll_intr_handle);
    sys_sem_t* sem = mpollfd_sem;
    if (sem)
    {
        xSemaphoreTake((QueueHandle_t)sem, 0);
    }
}

void mpoll_intr(int gpio, int uart)
{
    if (mpoll_intr_handle == NULL)
    {
        esp_intr_alloc(ETS_GPIO_INTR_SOURCE, 0, mpoll_intr_isr, NULL, &mpoll_intr_handle);
//      esp_register_shutdown_handler(mpoll_intr_shutdown);
    }
    if (gpio >= 0 && gpio < SOC_GPIO_PIN_COUNT)
    {
        mpoll_intr_gpio_mask |= BIT(gpio);
        gpio_ll_set_intr_type(&GPIO, gpio, GPIO_INTR_ANYEDGE);
    }
    if (uart >= 0 && uart < SOC_UART_NUM)
    {
        mpoll_intr_uart_mask |= BIT(uart);
    }
}
