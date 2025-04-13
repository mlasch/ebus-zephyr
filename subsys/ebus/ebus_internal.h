/*
 * SPDX-License-Identifier: Apache-2.0
 */

struct ebus_serial_config {
    /* UART device */
    const struct device *dev;
    /* RTU timeout (maximum inter-frame delay) */
    uint32_t rtu_timeout;
    /* Pointer to current position in buffer */
    uint8_t *uart_buf_ptr;
    /* RTU timer to detect frame end point */
    struct k_timer rtu_timer;
    /* Number of bytes received or to send */
    uint16_t uart_buf_ctr;
    /* Storage of received characters or characters to send */
    uint8_t uart_buf[CONFIG_EBUS_UART_BUFFER_SIZE];
};

struct ebus_context {
    /* eBUS UART interface name */
    const char *iface_name;

    /* Pointer to UART config */
    struct ebus_serial_config *cfg;
};
