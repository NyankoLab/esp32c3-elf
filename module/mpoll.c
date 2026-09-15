#include <unistd.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include <esp_log.h>

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
