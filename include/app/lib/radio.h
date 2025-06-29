#pragma once
#include <zephyr/kernel.h>

/**
 * @brief Start the LoRa radio thread.
 * @return 0 on success, negative error code on failure.
 */
int lora_radio_start(void);

/**
 * @brief Send data over the LoRa radio.
 * @param data Pointer to the data to send.
 * @param len Length of the data in bytes.
 * @return 0 on success, negative error code on failure.
 */
int lora_radio_send(const uint8_t *data, size_t len);

/**
 * @brief Structure to hold radio packet data.
 *
 * payload: The data payload of the packet.
 * length: The length of the payload.
 * rssi: Received Signal Strength Indicator (RSSI) of the packet.
 * snr: Signal-to-Noise Ratio (SNR) of the packet.
 * timestamp_ms: Timestamp in milliseconds since boot when the packet was received.
 */
struct lora_packet {
    uint8_t payload[256];
    uint16_t length;
    int16_t rssi;
    int16_t snr;
    uint64_t timestamp;
};