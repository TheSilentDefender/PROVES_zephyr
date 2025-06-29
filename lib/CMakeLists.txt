# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_SENSORS sensors)
add_subdirectory_ifdef(CONFIG_RADIO radio)
add_subdirectory_ifdef(CONFIG_RTC_LIB rtc)
add_subdirectory_ifdef(CONFIG_POWER power)

