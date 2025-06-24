#pragma once

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

/**
 * @brief Initialize and start the sensor manager
 *
 * This function initializes both LSM6DSO and LIS2MDL sensors.
 * If both sensors initialize successfully, it starts the sensor
 * reading thread. If either sensor fails to initialize, the
 * function returns an error and no thread is started.
 *
 * @return 0 on success, negative error code on failure
 */
int sensor_manager_start(void);

/**
 * @brief Stop the sensor manager thread
 *
 * This function stops the sensor reading thread if it is running.
 *
 * @return 0 on success, negative error code on failure
 */
int sensor_manager_stop(void);

/**
 * @brief Structure to hold sensor data
 *
 * This structure contains the readings from the sensors:
 * - Accelerometer (X, Y, Z)
 * - Gyroscope (X, Y, Z)
 * - Magnetometer (X, Y, Z)
 * - Temperature
 */
struct sensor_data {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float mag_x;
    float mag_y;
    float mag_z;
    float temp;
};