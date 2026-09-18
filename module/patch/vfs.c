#include "esp32c3.h"
#include <sys/lock.h>
#include <sys/queue.h>
#include <esp_private/esp_clk.h>
#include <esp_rom_gpio.h>
#include <esp_rom_serial_output.h>
#include <esp_vfs.h>
#include <esp32c3/rom/ets_sys.h>
#include <hal/uart_periph.h>
#include <hal/gpio_ll.h>
#include <hal/uart_ll.h>
#include <hal/usb_serial_jtag_ll.h>
#include <lwip/sockets.h>

#define USE_ESP_ROM         0
#define USE_TASK_FOR_UDP    1

static int udp_fd IRAM_BSS_ATTR = -1;
static struct sockaddr_in udp_sockaddr IRAM_BSS_ATTR;
static SemaphoreHandle_t g_mutex IRAM_BSS_ATTR = NULL;

typedef struct udp_message_t {
    STAILQ_ENTRY(udp_message_t) next;
    int size;
    char data[1];
} udp_message_t;
STAILQ_HEAD(udp_message_list_t, udp_message_t);
static struct udp_message_list_t* udp_message;
static TaskHandle_t udp_task_handle;

#define USBSERIAL_TIMEOUT_MAX_US 50000
static int s_usbserial_timeout IRAM_BSS_ATTR = 0;

static void usb_serial_jtag_ll_write(const uint8_t c)
{
    while (!usb_serial_jtag_ll_txfifo_writable() && s_usbserial_timeout < (USBSERIAL_TIMEOUT_MAX_US / 100)) {
        esp_rom_delay_us(100);
        s_usbserial_timeout++;
    }
    if (usb_serial_jtag_ll_txfifo_writable()) {
        usb_serial_jtag_ll_write_txfifo(&c, 1);
        s_usbserial_timeout = 0;
    }
}

void udp_task(void* arg)
{
    for (;;) {
        udp_message_t* message = STAILQ_FIRST(udp_message);
        if (message == NULL) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            continue;
        }
        STAILQ_REMOVE_HEAD(udp_message, next);

        lwip_sendto(udp_fd, message->data, message->size, MSG_DONTWAIT, (struct sockaddr*)&udp_sockaddr, sizeof(udp_sockaddr));
        free(message);
    }
}

void init_udp_console(const char* ip)
{
    if (udp_fd >= 0)
        return;
    udp_fd = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_fd < 0)
        return;
    char temp[64];
    strcpy(temp, ip);
    char* step = temp;
    char* address = strsep(&step, ":");
    char* port = strsep(&step, ":");
    udp_sockaddr.sin_len = sizeof(struct sockaddr_in);
    udp_sockaddr.sin_family = AF_INET;
    udp_sockaddr.sin_port = htons(port ? atoi(port) : 8888);
    lwip_inet_pton(AF_INET, address, &udp_sockaddr.sin_addr);
#if USE_TASK_FOR_UDP
    udp_message = calloc(1, sizeof(struct udp_message_list_t));
    STAILQ_INIT(udp_message);
    xTaskCreate(udp_task, "udp_task", 2048, NULL, tskIDLE_PRIORITY, &udp_task_handle);
#endif
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
    if (udp_fd >= 0 && size) {
#if USE_TASK_FOR_UDP
        udp_message_t* message = malloc(sizeof(udp_message_t) + size - 1);
        if (message) {
            message->size = size;
            memcpy(message->data, data, size);
            STAILQ_INSERT_TAIL(udp_message, message, next);
            vTaskNotifyGiveFromISR(udp_task_handle, NULL);
        }
#else
        lwip_sendto(udp_fd, data, size, MSG_DONTWAIT, (struct sockaddr*)&udp_sockaddr, sizeof(udp_sockaddr));
#endif
    }
    uint32_t baudrate = 0;
    if (mutex && uart0_tx != U0TXD_GPIO_NUM) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        baudrate = uart_ll_get_baudrate(&UART0, esp_clk_apb_freq());
        baudrate = ((baudrate + 150) / 300) * 300;
        while (uart_ll_get_txfifo_len(&UART0) < UART_LL_FIFO_DEF_LEN);
        ets_delay_us(1000);
        esp_rom_gpio_connect_out_signal(U0TXD_GPIO_NUM, UART_PERIPH_SIGNAL(UART_NUM_0, SOC_UART_PERIPH_SIGNAL_TX), 0, 0);
        uart_ll_set_baudrate(&UART0, 115200, esp_clk_apb_freq());
        uart_ll_txfifo_rst(&UART0);
    }
    const char* text = data;
#if USE_ESP_ROM
    for (size_t i = 0; i < size; ++i) {
        uint8_t c = text[i];
        if (c == '\n') {
            esp_rom_output_tx_one_char('\r');
        }
        esp_rom_output_tx_one_char(c);
    }
    esp_rom_output_tx_wait_idle(0);
#else
    for (size_t i = 0; i < size; ++i) {
        uint8_t c = text[i];
        if (c == '\n') {
            c = '\r';
            while (uart_ll_get_txfifo_len(&UART0) < 2);
            uart_ll_write_txfifo(&UART0, &c, 1);
            usb_serial_jtag_ll_write(c);
            c = '\n';
        }
        while (uart_ll_get_txfifo_len(&UART0) < 2);
        uart_ll_write_txfifo(&UART0, &c, 1);
        usb_serial_jtag_ll_write(c);
    }
    usb_serial_jtag_ll_txfifo_flush();
#endif
    if (baudrate != 0) {
        while (uart_ll_get_txfifo_len(&UART0) < UART_LL_FIFO_DEF_LEN);
        ets_delay_us(1000);
        esp_rom_gpio_connect_out_signal(uart0_tx, UART_PERIPH_SIGNAL(UART_NUM_0, SOC_UART_PERIPH_SIGNAL_TX), 0, 0);
        uart_ll_set_baudrate(&UART0, baudrate, esp_clk_apb_freq());
        uart_ll_txfifo_rst(&UART0);
        xSemaphoreGive(mutex);
    }
    return size;
}
