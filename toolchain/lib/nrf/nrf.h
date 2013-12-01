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

struct nrf_addr_t {
	uint64_t value; // 40 bits max
	uint8_t size;
};

typedef void (nrf_data_callback)(void *, uint8_t);
typedef void (nrf_callback)(void *);

void nrf_init(void);
void nrf_set_channel(uint8_t ch);
void nrf_set_power_datarate(enum nrf_tx_output_power_t power, enum nrf_data_rate_t data_rate);
void nrf_set_autoretransmit(uint8_t delay, uint8_t count); // delay in 250uS's
void nrf_receive(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb);
void nrf_send(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb);

void nrf_read_register(uint8_t reg, nrf_callback cb);
void nrf_write_register(uint8_t reg, void *data, size_t len, nrf_callback cb);

#endif
