#include <app/lib/sensors.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/zbus/zbus.h>

#include "zephyr/logging/log.h"
LOG_MODULE_REGISTER(sensors, LOG_LEVEL_INF);

static const struct device *lsm6dso_dev;
static const struct device *lis2mdl_dev;

// Thread control
static k_tid_t sensor_thread_id;
static K_THREAD_STACK_DEFINE(sensor_stack, 2048);
static struct k_thread sensor_thread_data;
static volatile bool thread_should_stop = false;

ZBUS_CHAN_DEFINE(sensor_data_chan, struct sensor_data, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

static float out_ev(struct sensor_value *val) {
    return val->val1 + (float)val->val2 / 1000000.0f;
}

static int lsm6dso_init(void) {
    lsm6dso_dev = DEVICE_DT_GET_ONE(st_lsm6dso);
    if (!device_is_ready(lsm6dso_dev)) {
        LOG_ERR("LSM6DSO not ready\n");
        return -1;
    }

    struct sensor_value odr = { .val1 = 12, .val2 = 500000 }; // 12.5 Hz

    if (sensor_attr_set(lsm6dso_dev, SENSOR_CHAN_ACCEL_XYZ,
                        SENSOR_ATTR_SAMPLING_FREQUENCY, &odr) ||
        sensor_attr_set(lsm6dso_dev, SENSOR_CHAN_GYRO_XYZ,
                        SENSOR_ATTR_SAMPLING_FREQUENCY, &odr)) {
        LOG_ERR("Failed to set LSM6DSO ODR\n");
        return -1;
    }

    LOG_INF("LSM6DSO initialized successfully\n");
    return 0;
}

static int lis2mdl_init(void) {
    lis2mdl_dev = device_get_binding("LIS2MDL");
    if (!lis2mdl_dev || !device_is_ready(lis2mdl_dev)) {
        LOG_ERR("LIS2MDL not ready\n");
        return -1;
    }

    LOG_INF("LIS2MDL initialized successfully\n");
    return 0;
}

static void sensor_loop(void *p1, void *p2, void *p3) {
    struct sensor_value x, y, z, temp;
    struct sensor_data payload;

    LOG_INF("Sensor thread started - both sensors ready\n");

    while (!thread_should_stop) {
        sensor_sample_fetch_chan(lsm6dso_dev, SENSOR_CHAN_ACCEL_XYZ);
        sensor_channel_get(lsm6dso_dev, SENSOR_CHAN_ACCEL_X, &x);
        sensor_channel_get(lsm6dso_dev, SENSOR_CHAN_ACCEL_Y, &y);
        sensor_channel_get(lsm6dso_dev, SENSOR_CHAN_ACCEL_Z, &z);

        payload.accel_x = out_ev(&x);
        payload.accel_y = out_ev(&y);
        payload.accel_z = out_ev(&z);

        sensor_sample_fetch_chan(lsm6dso_dev, SENSOR_CHAN_GYRO_XYZ);
        sensor_channel_get(lsm6dso_dev, SENSOR_CHAN_GYRO_X, &x);
        sensor_channel_get(lsm6dso_dev, SENSOR_CHAN_GYRO_Y, &y);
        sensor_channel_get(lsm6dso_dev, SENSOR_CHAN_GYRO_Z, &z);

        payload.gyro_x = out_ev(&x);
        payload.gyro_y = out_ev(&y);
        payload.gyro_z = out_ev(&z);

        sensor_sample_fetch_chan(lis2mdl_dev, SENSOR_CHAN_MAGN_XYZ);
        sensor_channel_get(lis2mdl_dev, SENSOR_CHAN_MAGN_X, &x);
        sensor_channel_get(lis2mdl_dev, SENSOR_CHAN_MAGN_Y, &y);
        sensor_channel_get(lis2mdl_dev, SENSOR_CHAN_MAGN_Z, &z);

        payload.mag_x = out_ev(&x);
        payload.mag_y = out_ev(&y);
        payload.mag_z = out_ev(&z);

        sensor_sample_fetch_chan(lsm6dso_dev, SENSOR_CHAN_DIE_TEMP);
        sensor_channel_get(lsm6dso_dev, SENSOR_CHAN_DIE_TEMP, &temp);
        payload.temp = out_ev(&temp);

        if (zbus_chan_pub(&sensor_data_chan, &payload, K_NO_WAIT) != 0) {
            LOG_ERR("Zbus publish failed\n");
        }

        k_sleep(K_SECONDS(1));
    }

    LOG_INF("Sensor thread stopped\n");
}

int sensor_manager_start(void) {
    // Try to initialize both sensors
    if (lsm6dso_init() != 0) {
        LOG_ERR("Failed to initialize LSM6DSO - sensor thread will not start\n");
        return -1;
    }

    if (lis2mdl_init() != 0) {
        LOG_ERR("Failed to initialize LIS2MDL - sensor thread will not start\n");
        return -1;
    }

    sensor_thread_id = k_thread_create(&sensor_thread_data,
                                      sensor_stack,
                                      K_THREAD_STACK_SIZEOF(sensor_stack),
                                      sensor_loop,
                                      NULL, NULL, NULL,
                                      5,
                                      0,
                                      K_MSEC(500));

    if (sensor_thread_id == NULL) {
        LOG_ERR("Failed to create sensor thread\n");
        return -1;
    }

    LOG_INF("Sensor manager started successfully\n");
    return 0;
}