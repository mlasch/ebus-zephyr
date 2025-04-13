/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/sensor.h>
#include <zephyr/ebus/ebus.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

int main(void)
{
    int ret;
    LOG_INF("Zephyr Example Application %s", APP_VERSION_STRING);

    const char iface_name[] = {DEVICE_DT_NAME(DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_ebus_serial))};
    LOG_INF("ebus interface name: %s", iface_name);

    int iface = ebus_iface_get_by_name(iface_name);
    LOG_INF("ebus interface number: %d", iface);

    ret = ebus_init_client(iface);
    if (ret < 0) {
        LOG_ERR("Failed to initialize ebus client");
        return ret;
    }

    return 0;
}
