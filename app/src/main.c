/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);
static const struct device *usb_device;

int main(void)
{

	usb_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

	if (usb_enable(NULL) != 0)
	{
		printk("Failed to enable USB\n");
		return -1;
	}

	printk("USB Serial Initialized\n");

	while (1)
	{
		printk("Current uptime: %lld ms\n", k_uptime_get());
		k_sleep(K_MSEC(100));
	}

	return 0;
}
