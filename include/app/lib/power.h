#pragma once

struct power_data {
    float voltage;
    float current;
    float power;
};

/**
 * @brief Initialize and start the power monitoring thread.
 *
 * @return 0 on success, negative value on failure.
 */
int power_manager_start(void);

/**
 * @brief Stop the power monitoring thread gracefully.
 */
void power_manager_stop(void);