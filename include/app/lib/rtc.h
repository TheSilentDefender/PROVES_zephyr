#pragma once

#include <zephyr/drivers/rtc.h>

/**
 * @brief Initialize the RTC device and set the default time.
 *
 * This function initializes the RTC hardware and sets it to a predefined
 * start time. It should be called once during system initialization.
 *
 * @return 0 on success, negative error code on failure.
 */
int rtc_init(void);

/**
 * @brief Get the current RTC time.
 *
 * Retrieves the current time from the RTC and stores it in a standard
 * Zephyr `struct rtc_time`.
 *
 * @param[out] time Pointer to a `struct rtc_time` where the current time will be stored.
 * @return 0 on success, negative error code on failure.
 */
int get_rtc(struct rtc_time *time);

/**
 * @brief Set the RTC time.
 *
 * Sets the RTC to a specific time provided in a Zephyr `struct rtc_time`.
 *
 * @param[in] time Pointer to a `struct rtc_time` containing the time to set.
 * @return 0 on success, negative error code on failure.
 */
int set_rtc(const struct rtc_time *time);
