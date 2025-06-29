#include <time.h>
#include <app/lib/rtc.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/rtc.h>
LOG_MODULE_REGISTER(rtc, LOG_LEVEL_INF);

static const struct device *rtc_dev;
struct rtc_time start_time = {
    .tm_sec = 0,
    .tm_min = 0,
    .tm_hour = 0,
    .tm_mday = 1,
    .tm_mon = 0,
    .tm_year = 2025-1900,
    .tm_wday = 0,
    .tm_yday = 0,
    .tm_isdst = 0
};
int rtc_init(void) {
    rtc_dev = DEVICE_DT_GET_ONE(microcrystal_rv3028);
    if (!device_is_ready(rtc_dev)) {
        LOG_ERR("RTC device not ready\n");
        return -1;
    }
    LOG_INF("RTC initialized successfully\n");

    return 0;
}

int get_rtc(struct rtc_time *time) {
    if (rtc_get_time(rtc_dev, time) != 0) {
        LOG_ERR("Failed to get RTC time\n");
        return -1;
    }
    LOG_INF("Current RTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
            time->tm_year + 1900, time->tm_mon + 1,
            time->tm_mday, time->tm_hour,
            time->tm_min, time->tm_sec);
    return 0;
}

int set_rtc(const struct rtc_time *time) {
    if (rtc_set_time(rtc_dev, time) != 0) {
        LOG_ERR("Failed to set RTC time\n");
        return -1;
    }
    LOG_INF("RTC time set to %04d-%02d-%02d %02d:%02d:%02d\n",
            start_time.tm_year + 1900, start_time.tm_mon + 1,
            start_time.tm_mday, start_time.tm_hour,
            start_time.tm_min, start_time.tm_sec);
    return 0;
}