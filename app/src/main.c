/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <app/lib/sensors.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);
static const struct device *usb_device;

ZBUS_CHAN_DECLARE(sensor_data_chan);

int get_sensor_data(struct sensor_data *data_out) {
    struct sensor_data data;  // Local variable to hold value

    const int ret = zbus_chan_read(&sensor_data_chan, &data, K_MSEC(500));
    if (ret == 0) {
        memcpy(data_out, &data, sizeof(struct sensor_data));
        return 0;
    }

    LOG_WRN("Failed to read sensor data from Zbus: %d", ret);
    return ret;
}

int main(void) {
    usb_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

    if (usb_enable(NULL) != 0) {
        LOG_ERR("Failed to enable USB\n");
        return -1;
    }

    LOG_INF("USB Serial Initialized\n");

    if (sensor_manager_start() != 0) {
        LOG_ERR("Failed to start sensor manager\n");
        return -1;
    }

    struct sensor_data sensor_reading;

    while (1) {
        if (get_sensor_data(&sensor_reading) == 0) {
            LOG_INF("Temp: %.1f°C, Accel Z: %.2f",
                    sensor_reading.temp, sensor_reading.accel_z);
        }
        k_sleep(K_SECONDS(3));
    }

    return 0;
}
