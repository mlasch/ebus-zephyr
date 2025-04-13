/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ebus_serial, CONFIG_EBUS_LOG_LEVEL);

#include "ebus_internal.h"

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

int ebus_init_client(int client_iface)
{
    LOG_INF("ebus_client_init %d", client_iface);
    return 0;
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
