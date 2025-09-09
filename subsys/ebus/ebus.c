/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ebus_serial, CONFIG_EBUS_LOG_LEVEL);

#include "ebus_internal.h"

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#define DT_DRV_COMPAT zephyr_ebus_serial

#define EBUS_DT_GET_SERIAL_DEV(inst)                                                               \
    {                                                                                              \
        .dev = DEVICE_DT_GET(DT_INST_PARENT(inst)),                                                \
    },

static struct ebus_serial_config ebus_serial_cfg[] = {
    DT_INST_FOREACH_STATUS_OKAY(EBUS_DT_GET_SERIAL_DEV)};

#define EBUS_DT_GET_DEV(inst)                                                                      \
    {                                                                                              \
        .iface_name = DEVICE_DT_NAME(DT_DRV_INST(inst)),                                           \
        .cfg = &ebus_serial_cfg[inst],                                                             \
    },

static struct ebus_context ebus_ctx_tbl[] = {DT_INST_FOREACH_STATUS_OKAY(EBUS_DT_GET_DEV)};

static void cb_handler_rx(struct ebus_context *ctx)
{
    struct ebus_serial_config *cfg = ctx->cfg;
    uint8_t rx_byte;
    if (!uart_irq_update(cfg->dev)) {
        LOG_ERR("uart_irq_update");
        return;
    }

    if (!uart_irq_rx_ready(cfg->dev)) {
        LOG_ERR("uart_irq_rx_ready");
        return;
    }
    if (uart_fifo_read(cfg->dev, &rx_byte, 1) != 1) {
        LOG_ERR("Failed to read UART");
        return;
    }

    if (ctx->cfg->uart_buf_idx >= CONFIG_EBUS_UART_BUFFER_SIZE) {
        LOG_ERR("UART buffer overflow");
        ctx->cfg->uart_buf_idx = 0;
        return;
    }

    if (rx_byte == 0xaa) {
        /* Start of new frame */
        ctx->cfg->uart_buf_idx = 0;
        return;
    }

    if (ctx->cfg->prev_rx_byte == 0xa9) {
        if (rx_byte == 0x00) {
            ctx->cfg->uart_buf[ctx->cfg->uart_buf_idx++] = 0xa9;
            return;
        }
        if (rx_byte == 0x01) {
            ctx->cfg->uart_buf[ctx->cfg->uart_buf_idx++] = 0xaa;
            return;
        }
    }

    ctx->cfg->uart_buf[ctx->cfg->uart_buf_idx++] = rx_byte;

    ctx->cfg->prev_rx_byte = rx_byte;
}

static void uart_cb_handler(const struct device *dev, void *app_data)
{
    struct ebus_context *ctx = (struct ebus_context *)app_data;
    if (ctx == NULL) {
        LOG_ERR("eBUS UART is not properly initialized");
        return;
    }

    if (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
        if (uart_irq_rx_ready(dev)) {
            cb_handler_rx(ctx);
        }

        if (uart_irq_tx_ready(dev)) {
            LOG_ERR("eBUS UART TX is not implemented");
        }
    }
}

static int ebus_serial_init(struct ebus_context *ctx)
{
    int ret;
    struct ebus_serial_config *cfg = ctx->cfg;
    if (!device_is_ready(cfg->dev)) {
        LOG_ERR("Bus device %s is not ready", cfg->dev->name);
        return -ENODEV;
    }

    if (IS_ENABLED(CONFIG_UART_USE_RUNTIME_CONFIGURE)) {
        struct uart_config uart_cfg = {
            .baudrate = 2400,
            .parity = UART_CFG_PARITY_NONE,
            .stop_bits = UART_CFG_STOP_BITS_1,
            .data_bits = UART_CFG_DATA_BITS_8,
            .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
        };
        if (uart_configure(cfg->dev, &uart_cfg) != 0) {
            LOG_ERR("Failed to configure UART");
            return -EINVAL;
        }
    }
    cfg->uart_buf_ctr = 0;
    cfg->uart_buf_ptr = &cfg->uart_buf[0];
    ret = uart_irq_callback_user_data_set(cfg->dev, uart_cb_handler, ctx);
    if (ret < 0) {
        LOG_ERR("Failed to set UART callback: %d", ret);
        return ret;
    };
    uart_irq_rx_enable(cfg->dev);
    LOG_INF("UART configured and enabled");
    return 0;
}

static struct ebus_context *ebus_init_iface(const uint8_t iface)
{
    struct ebus_context *ctx;
    if (iface >= ARRAY_SIZE(ebus_ctx_tbl)) {
        LOG_ERR("Interface %u not available", iface);
        return NULL;
    }

    ctx = &ebus_ctx_tbl[iface];

    if (atomic_test_and_set_bit(&ctx->state, EBUS_STATE_CONFIGURED)) {
        LOG_ERR("Interface already used");
        return NULL;
    }

    k_mutex_init(&ctx->iface_lock);

    return ctx;
}

int ebus_init_client(const int iface)
{
    int ret;
    struct ebus_context *ctx = NULL;

    ctx = ebus_init_iface(iface);
    if (ctx == NULL) {
        ret = -EINVAL;
        goto init_client_error;
    }
    ret = ebus_serial_init(ctx);
    if (ret < 0) {
        LOG_ERR("Failed to init UART");
        ret = -EINVAL;
        goto init_client_error;
    }

    return 0;

init_client_error:
    return ret;
}

int ebus_iface_get_by_name(const char *iface_name)
{
    for (int i = 0; i < ARRAY_SIZE(ebus_ctx_tbl); i++) {
        if (strcmp(iface_name, ebus_ctx_tbl[i].iface_name) == 0) {
            return i;
        }
    }

    return -ENODEV;
}
