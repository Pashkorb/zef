#include "binary_protocol.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#define PROTOCOL_UART_NODE DT_ALIAS(protocol_uart)

int run_binary_protocol(void)
{
    const struct device *const protocol_uart_device =
        DEVICE_DT_GET(PROTOCOL_UART_NODE);
    uint8_t received_value;

    if (false == device_is_ready(protocol_uart_device))
    {
        return -ENODEV;
    }

    while (true)
    {
        if (0 == uart_poll_in(protocol_uart_device, &received_value))
        {
            uart_poll_out(protocol_uart_device, (uint8_t)(received_value + 1U));
        }
        else
        {
            k_msleep(1);
        }
    }

    return 0;
}
