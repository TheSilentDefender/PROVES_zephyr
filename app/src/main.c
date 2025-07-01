/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>
#include <app/lib/sensors.h>
#include <app/lib/radio.h>
#include <app/lib/rtc.h>
#include <app/lib/power.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);
static const struct device *usb_device;

ZBUS_CHAN_DECLARE(sensor_data_chan);
ZBUS_CHAN_DECLARE(power_data_chan);

int get_sensor_data(struct sensor_data *data_out) {
    struct sensor_data data;

    const int ret = zbus_chan_read(&sensor_data_chan, &data, K_MSEC(500));
    if (ret == 0) {
        memcpy(data_out, &data, sizeof(struct sensor_data));
        return 0;
    }

    LOG_WRN("Failed to read sensor data from Zbus: %d", ret);
    return ret;
}

int get_power_data(struct power_data *data_out) {
    struct power_data data;

    const int ret = zbus_chan_read(&power_data_chan, &data, K_MSEC(500));
    if (ret == 0) {
        memcpy(data_out, &data, sizeof(struct power_data));
        return 0;
    }

    LOG_WRN("Failed to read power data from Zbus: %d", ret);
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

    if (lora_radio_start() != 0) {
        LOG_ERR("Failed to start LoRa radio\n");
        return -1;
    }

    if (rtc_init() != 0) {
        LOG_ERR("Failed to initialize RTC\n");
        return -1;
    }

    if (power_manager_start() != 0) {
        LOG_ERR("Failed to start power manager\n");
        return -1;
    }

    struct sensor_data sensor_reading;
    struct rtc_time current_time;
    struct power_data power_reading;
    while (1) {
        if (get_sensor_data(&sensor_reading) == 0) {
            LOG_INF("Temp: %.1f°C, Accel Z: %.2f",
                    (double) sensor_reading.temp, (double) sensor_reading.accel_z);
            lora_radio_send((uint8_t *) &sensor_reading, sizeof(sensor_reading));
        }
        get_rtc(&current_time);
        if (get_power_data(&power_reading) == 0) {
            LOG_INF("Voltage: %.2fV, Current: %.2fA, Power: %.2fW",
                    (double)power_reading.voltage, (double)power_reading.current, (double)power_reading.power);
        } else {
            LOG_ERR("Failed to read power data");
        }
        LOG_INF("Current RTC time: %04d-%02d-%02d %02d:%02d:%02d",
                current_time.tm_year + 1900, current_time.tm_mon + 1,
                current_time.tm_mday, current_time.tm_hour,
                current_time.tm_min, current_time.tm_sec);
        k_sleep(K_SECONDS(3));
    }

    return 0;
}
