#include <unistd.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include <esp_log.h>
#include <driver/gpio.h>

#include "mpoll.h"

static int mpollfd_count = 0;
static struct pollfd mpollfd[CONFIG_LWIP_MAX_SOCKETS];
static mpoll_callback mpollfd_callback[CONFIG_LWIP_MAX_SOCKETS];

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
    if (lwip_poll(mpollfd, mpollfd_count, timeout) <= 0)
        return;

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
        }
    }
}

uint32_t mpoll_gpio_mask = 0;
static TaskHandle_t mpoll_isr_task_handle = NULL;
static struct sockaddr_in const sockaddr_udp =
{
    .sin_len = sizeof(struct sockaddr_in),
    .sin_family = AF_INET,
    .sin_port = htons(65535),
    .sin_addr = { .s_addr = htonl(INADDR_LOOPBACK) },
};

static void IRAM_ATTR mpoll_isr_trigger(void* arg)
{
    gpio_ll_intr_disable_mask(mpoll_gpio_mask);
    vTaskNotifyGiveFromISR(mpoll_isr_task_handle, NULL);
}

static void mpoll_isr_task(void* arg)
{
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        lwip_sendto((int)arg, "", 1, MSG_DONTWAIT, (struct sockaddr*)&sockaddr_udp, sizeof(sockaddr_udp));
    }
}

static int mpoll_isr_recv(int fd, int revents)
{
    struct sockaddr_in sockaddr;
    socklen_t len = sizeof(sockaddr);
    lwip_recvfrom(fd, &sockaddr, sizeof(sockaddr), MSG_DONTWAIT, (struct sockaddr*)&sockaddr, &len);
    return revents;
}

void mpoll_isr(int pin)
{
    if (mpoll_isr_task_handle == NULL)
    {
        int wakeup_socket = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        lwip_bind(wakeup_socket, (struct sockaddr*)&sockaddr_udp, sizeof(sockaddr_udp));
        mpoll_ctl(wakeup_socket, mpoll_isr_recv);
        xTaskCreate(mpoll_isr_task, "mpoll_isr_task", 2048, (void*)wakeup_socket, tskIDLE_PRIORITY, &mpoll_isr_task_handle);
        gpio_isr_register(mpoll_isr_trigger, NULL, 0, NULL);
    }
    mpoll_gpio_mask |= BIT(pin);
    gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_ANYEDGE);
}
