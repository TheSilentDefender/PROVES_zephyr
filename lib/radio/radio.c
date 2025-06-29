#include <app/lib/radio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <string.h>

LOG_MODULE_REGISTER(lora_radio, LOG_LEVEL_INF);

static const struct device *lora_dev;

ZBUS_CHAN_DEFINE(lora_data_chan, struct lora_packet, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

static int enable_rx(void) {
    struct lora_modem_config config = {
        .frequency = 437400000,
        .bandwidth = BW_125_KHZ,
        .datarate = SF_8,
        .preamble_len = 8,
        .coding_rate = CR_4_5,
        .tx_power = 14,
        .iq_inverted = false,
        .public_network = false,
        .tx = false,
    };
    return lora_config(lora_dev, &config);
}

static int enable_tx(void) {
    struct lora_modem_config config = {
        .frequency = 437400000,
        .bandwidth = BW_125_KHZ,
        .datarate = SF_8,
        .preamble_len = 8,
        .coding_rate = CR_4_5,
        .tx_power = 14,
        .iq_inverted = false,
        .public_network = false,
        .tx = true,
    };
    return lora_config(lora_dev, &config);
}

static void lora_receive_cb(const struct device *dev, uint8_t *data, uint16_t size,
                            int16_t rssi, int8_t snr, void *user_data)
{
    struct lora_packet packet;
    packet.length = size > sizeof(packet.payload) ? sizeof(packet.payload) : size;
    memcpy(packet.payload, data, packet.length);
    packet.rssi = rssi;
    packet.snr = snr;
    packet.timestamp = k_uptime_get();
    int err = zbus_chan_pub(&lora_data_chan, &packet, K_NO_WAIT);
    if (err) {
        LOG_ERR("Failed to publish LoRa packet to channel: %d", err);
    } else {
        zbus_chan_notify(&lora_data_chan, K_NO_WAIT);
        LOG_INF("Published LoRa packet to channel: %d bytes (RSSI: %d, SNR: %d)",
                packet.length, packet.rssi, packet.snr);
        LOG_HEXDUMP_INF(packet.payload, packet.length, "LoRa Packet Data");
    }
}


int lora_radio_start(void) {
    lora_dev = DEVICE_DT_GET_ONE(semtech_sx1276);
    if (!device_is_ready(lora_dev)) {
        LOG_ERR("LoRa device not ready");
        return -1;
    }

    int err = enable_rx();
    if (err) {
        LOG_ERR("Failed to enable RX mode: %d", err);
        return err;
    }

    err = lora_recv_async(lora_dev, lora_receive_cb, NULL);
    if (err) {
        LOG_ERR("Failed to start async RX: %d", err);
        return err;
    }

    LOG_INF("LoRa radio started");
    return 0;
}

int lora_radio_send(const uint8_t *data, size_t len) {
    if (!lora_dev) {
        LOG_ERR("LoRa device not initialized");
        return -1;
    }

    int err = lora_recv_async(lora_dev, NULL, NULL);
    if (err < 0) {
        LOG_ERR("Failed to disable async RX: %d", err);
        return err;
    }

    err = enable_tx();
    if (err < 0) {
        LOG_ERR("Failed to enable TX mode: %d", err);
        return err;
    }

    err = lora_send(lora_dev, (uint8_t *) data, len);
    if (err < 0) {
        LOG_ERR("Failed to send LoRa packet: %d", err);
    } else {
        LOG_INF("LoRa sent");
    }


    int err_rx = enable_rx();
    if (err_rx < 0) {
        LOG_ERR("Failed to re-enable RX mode: %d", err_rx);
        return err_rx;
    }

    err_rx = lora_recv_async(lora_dev, lora_receive_cb, NULL);
    if (err_rx < 0) {
        LOG_ERR("Failed to re-enable async RX: %d", err_rx);
        return err_rx;
    }

    return err < 0 ? err : 0;
}
