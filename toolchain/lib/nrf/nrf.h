#ifndef __NRF_H
#define __NRF_H

#include <mchck.h>
#include <sys/types.h>

enum nrf_tx_output_power_t {
	NRF_TX_POWER_N18DBM = 0,
	NRF_TX_POWER_N12DBM = 1,
	NRF_TX_POWER_N6DBM = 2,
	NRF_TX_POWER_0DBM = 3 // default
};

enum nrf_data_rate_t {
	NRF_DATA_RATE_1MBPS = 0x0,
	NRF_DATA_RATE_250KBPS = 0x1,
	NRF_DATA_RATE_2MBPS = 0x4 // default
};

enum nrf_crc_encoding_scheme_t {
	NRF_CRC_ENC_1_BYTE = 0, // default
	NRF_CRC_ENC_2_BYTES = 1
};

struct nrf_addr_t {
	uint64_t value; // 40 bits max
	uint8_t size;
};

typedef void (nrf_data_callback)(void *, uint8_t);


/* Initialize nRF library, this MUST be called before using other "nrf_" functions. */
void nrf_init(void);

/* Set the radio channel (range [1 .. 126]). */
void nrf_set_channel(uint8_t ch);

/*
 * Set both output power and data rate.
 * Note: Setting data rate to 250kbps will automatically set auto retransmission delay to 500us.
 */
void nrf_set_power_datarate(enum nrf_tx_output_power_t power, enum nrf_data_rate_t data_rate);

/*
 * "delay" 0 = 250us, 1 = 500us ... (max) 31 = 4000us.
 * "count" 0 = disables auto retransmission (max 31).
 */
void nrf_set_autoretransmit(uint8_t delay, uint8_t count); // delay in 250uS's

/*
 * Puts the nRF in power down mode after each send/receive.
 * While powered down the nRF consumes less than 1mA but may take up to 4.5ms to wake up.
 */
void nrf_enable_powersave();

/*
 * Puts the nRF in standby-1 mode after each send/receive.
 * While in standby-1 the nRF is always ready to send/receive (i.e. no extra wakeup delay).
 */
void nrf_disable_powersave();

/*
 * Enables dynamic payload for both send and receive.
 * For receives the "len" argument is ignored, the caller should use a 32 byte buffer to avoid overflows.
 */
void nrf_enable_dynamic_payload();

/* Disables dynamic payload for both send and receive. */
void nrf_disable_dynamic_payload();

/* Sets CRC length (1 or 2 bytes). */
void nrf_set_crc_length(enum nrf_crc_encoding_scheme_t len);

/*
 * The nRF will listen for data on "addr". This is a one-shot operation.
 * If dynamic payload is enabled (@see nrf_enable_dynamic_payload) the "len" argument is ignored.
 * Once data is made available the user callback "cb" is invoked with "data" and "len".
 * In case of failure the data argument in the callback will be NULL and length will be larger than 32.
 */
void nrf_receive(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb);

/*
 * Send "data" with size "len" to a certain address "addr".
 * Once the receiver ACK's or the transmitter fails all retransmission attempts the user callback "cb" is invoked.
 * In case of failure the data argument in the callback will be NULL and length will be zero.
 */
void nrf_send(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb);

#endif

/* vim: set ts=8 sw=8 noexpandtab: */
