#include "esp32c3.h"
#include <sys/lock.h>
#include <esp_private/esp_clk.h>
#include <esp_rom_gpio.h>
#include <esp_vfs.h>
#include <esp32c3/rom/ets_sys.h>
#include <soc/uart_periph.h>
#include <hal/gpio_ll.h>
#include <hal/uart_ll.h>
#include <lwip/sockets.h>

static int udp_fd = -1;
static struct sockaddr_in udp_sockaddr;
static SemaphoreHandle_t g_mutex = NULL;

void init_udp_console(const char* ip)
{
    udp_fd = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_fd < 0)
        return;
    udp_sockaddr.sin_len = sizeof(udp_sockaddr);
    udp_sockaddr.sin_family = AF_INET;
    udp_sockaddr.sin_port = htons(8888);
    udp_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int mode = 1;
    lwip_ioctl(udp_fd, FIONBIO, &mode);
    lwip_bind(udp_fd, (struct sockaddr*)&udp_sockaddr, sizeof(udp_sockaddr));
    lwip_inet_pton(AF_INET, ip, &udp_sockaddr.sin_addr);
}

void vfs_init(void)
{
    g_mutex = xSemaphoreCreateRecursiveMutex();
}

ssize_t __wrap__read_r_console(struct _reent* r, int fd, const void* data, size_t size)
{
    return -1;
}

ssize_t __wrap__write_r_console(struct _reent* r, int fd, const void* data, size_t size)
{
    SemaphoreHandle_t mutex = g_mutex;
    if (fd >= 0) {
        lwip_sendto(udp_fd, data, size, 0, (struct sockaddr*)&udp_sockaddr, sizeof(udp_sockaddr));
    }
    uint32_t baudrate = uart_ll_get_baudrate(&UART0, esp_clk_apb_freq());
    if (mutex && uart0_tx != U0TXD_GPIO_NUM) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        while (uart_ll_get_txfifo_len(&UART0) < UART_LL_FIFO_DEF_LEN);
        ets_delay_us(1000);
        esp_rom_gpio_connect_out_signal(U0TXD_GPIO_NUM, UART_PERIPH_SIGNAL(0, SOC_UART_TX_PIN_IDX), 0, 0);
        uart_ll_set_baudrate(&UART0, 115200, esp_clk_apb_freq());
        uart_ll_txfifo_rst(&UART0);
    }
    const char* text = data;
    for (size_t i = 0; i < size; ++i) {
        uint8_t c = text[i];
        if (c == '\n') {
            c = '\r';
            while (uart_ll_get_txfifo_len(&UART0) < 2);
            uart_ll_write_txfifo(&UART0, &c, 1);
            c = '\n';
        }
        while (uart_ll_get_txfifo_len(&UART0) < 2);
        uart_ll_write_txfifo(&UART0, &c, 1);
    }
    if (mutex && uart0_tx != U0TXD_GPIO_NUM) {
        while (uart_ll_get_txfifo_len(&UART0) < UART_LL_FIFO_DEF_LEN);
        ets_delay_us(1000);
        esp_rom_gpio_connect_out_signal(uart0_tx, UART_PERIPH_SIGNAL(0, SOC_UART_TX_PIN_IDX), 0, 0);
        uart_ll_set_baudrate(&UART0, baudrate, esp_clk_apb_freq());
        uart_ll_txfifo_rst(&UART0);
        xSemaphoreGive(mutex);
    }
    return size;
}
