#include <app/lib/power.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(power, LOG_LEVEL_INF);

static const struct device *ina219_dev;
static k_tid_t power_thread_id;
static K_THREAD_STACK_DEFINE(power_stack, 2048);
static struct k_thread power_thread_data;
static volatile bool thread_should_stop = false;

ZBUS_CHAN_DEFINE(power_data_chan, struct power_data, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

static int power_init(void) {
    ina219_dev = DEVICE_DT_GET_ONE(ti_ina219);
    if (!device_is_ready(ina219_dev)) {
        LOG_ERR("INA219 not ready");
        return -1;
    }
    LOG_INF("INA219 initialized");
    return 0;
}

static void power_loop(void *p1, void *p2, void *p3) {
    struct power_data payload;
    struct sensor_value voltage, current, power;

    while (!thread_should_stop) {
        if (sensor_sample_fetch(ina219_dev) != 0) {
            LOG_ERR("Failed to fetch sensor data");
            k_sleep(K_SECONDS(1));
            continue;
        }

        if (sensor_channel_get(ina219_dev, SENSOR_CHAN_VOLTAGE, &voltage) != 0 ||
            sensor_channel_get(ina219_dev, SENSOR_CHAN_CURRENT, &current) != 0 ||
            sensor_channel_get(ina219_dev, SENSOR_CHAN_POWER, &power) != 0) {
            LOG_ERR("Failed to read sensor channels");
            k_sleep(K_SECONDS(1));
            continue;
        }

        payload.voltage = voltage.val1 + voltage.val2 / 1000000.0f;
        payload.current = current.val1 + current.val2 / 1000000.0f;
        payload.power = power.val1 + power.val2 / 1000000.0f;

        if (zbus_chan_pub(&power_data_chan, &payload, K_NO_WAIT) != 0) {
            LOG_ERR("Zbus publish failed");
        }

        k_sleep(K_SECONDS(1));
    }

    LOG_INF("Power thread exiting");
}

int power_manager_start(void) {
    if (power_init() != 0) {
        return -1;
    }

    power_thread_id = k_thread_create(&power_thread_data,
                                      power_stack,
                                      K_THREAD_STACK_SIZEOF(power_stack),
                                      power_loop,
                                      NULL, NULL, NULL,
                                      5, 0, K_MSEC(500));

    k_thread_name_set(power_thread_id, "power_thread");

    LOG_INF("Power manager started");
    return 0;
}

void power_manager_stop(void) {
    thread_should_stop = true;
    k_thread_join(power_thread_id, K_FOREVER);
    LOG_INF("Power manager stopped");
}
