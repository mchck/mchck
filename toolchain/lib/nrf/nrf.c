#include <nrf/nrf.h>

#define NRF_REG_MASK 0x1f // 0001 1111

/* nrf spi commands */
enum NRF_CMD {
	NRF_CMD_R_REGISTER		= 0x00, // 5 lower bits masked
	NRF_CMD_W_REGISTER		= 0x20, // 5 lower bits masked
	NRF_CMD_R_RX_PAYLOAD		= 0x61,
	NRF_CMD_W_TX_PAYLOAD		= 0xa0,
	NRF_CMD_FLUSH_TX		= 0xe1,
	NRF_CMD_FLUSH_RX		= 0xe2,
	NRF_CMD_REUSE_TX_PL		= 0xe3,
	NRF_CMD_R_RX_PL_WID		= 0x60,
	NRF_CMD_W_ACK_PAYLOAD		= 0xa8, // 3 lower bits masked
	NRF_CMD_W_TX_PAYLOAD_NO_ACK	= 0xb0,
	NRF_CMD_NOP			= 0xff
};

/* nrf registers */
enum NRF_REG_ADDR {
	NRF_REG_ADDR_CONFIG		= 0x00,
	NRF_REG_ADDR_EN_AA		= 0x01,
	NRF_REG_ADDR_EN_RXADDR		= 0x02,
	NRF_REG_ADDR_SETUP_AW		= 0x03,
	NRF_REG_ADDR_SETUP_RETR		= 0x04,
	NRF_REG_ADDR_RF_CH		= 0x05,
	NRF_REG_ADDR_RF_SETUP		= 0x06,
	NRF_REG_ADDR_STATUS		= 0x07,
	NRF_REG_ADDR_OBSERVE_TX		= 0x08,
	NRF_REG_ADDR_RPD		= 0x09,
	NRF_REG_ADDR_RX_ADDR_P0		= 0x0a,
	NRF_REG_ADDR_RX_ADDR_P1		= 0x0b,
	NRF_REG_ADDR_RX_ADDR_P2		= 0x0c,
	NRF_REG_ADDR_RX_ADDR_P3		= 0x0d,
	NRF_REG_ADDR_RX_ADDR_P4		= 0x0e,
	NRF_REG_ADDR_RX_ADDR_P5		= 0x0f,
	NRF_REG_ADDR_TX_ADDR		= 0x10,
	NRF_REG_ADDR_RX_PW_P0		= 0x11,
	NRF_REG_ADDR_RX_PW_P1		= 0x12,
	NRF_REG_ADDR_RX_PW_P2		= 0x13,
	NRF_REG_ADDR_RX_PW_P3		= 0x14,
	NRF_REG_ADDR_RX_PW_P4		= 0x15,
	NRF_REG_ADDR_RX_PW_P5		= 0x16,
	NRF_REG_ADDR_FIFO_STATUS	= 0x17,
	NRF_REG_ADDR_DYNPD		= 0x1c,
	NRF_REG_ADDR_FEATURE		= 0x1d
};

/* pin mapping */
enum {
	NRF_CE		= PIN_PTC3,
	NRF_CSN		= PIN_PTC2,
	NRF_SCK		= PIN_PTC5,
	NRF_MOSI	= PIN_PTC6,
	NRF_MISO	= PIN_PTC7,
	NRF_IRQ		= PIN_PTC4
};

/* SPI */
enum {
	NRF_SPI_CS = SPI_PCS2
};

/* nrf lib states (key) */
enum {
	NRF_STATE_SETUP	= 0,
	NRF_STATE_RECV	= 0x20,
	NRF_STATE_SEND	= 0x40
};

/* nrf lib states (all) */
enum nrf_state_t {
	// setup
	NRF_STATE_SETUP_SET_POWER_RATE	= NRF_STATE_SETUP,
	NRF_STATE_SETUP_SET_CHANNEL,
	NRF_STATE_SETUP_DPL,
	NRF_STATE_SETUP_SET_AUTORETR,
	NRF_STATE_SETUP_DONE,
	// recv
	NRF_STATE_RECV_SET_RX_HIGH	= NRF_STATE_RECV,
	NRF_STATE_RECV_SET_PAYLOAD_SIZE,
	NRF_STATE_RECV_SET_DPL,
	NRF_STATE_RECV_SET_RX_ADDRESS,
	NRF_STATE_RECV_SET_CE_HIGH,
	NRF_STATE_RECV_WAITING,
	NRF_STATE_RECV_GET_PAYLOAD_SIZE,
	NRF_STATE_RECV_GET_PAYLOAD,
	NRF_STATE_RECV_POWER_DOWN,
	NRF_STATE_RECV_USER_DATA,
	// send
	NRF_STATE_SEND_SET_RX_LOW	= NRF_STATE_SEND,
	NRF_STATE_SEND_SET_TX_ADDR,
	NRF_STATE_SEND_SET_RX_ADDR_P0,
	NRF_STATE_SEND_SET_DPL,
	NRF_STATE_SEND_TX_PAYLOAD,
	NRF_STATE_SEND_SET_CE_HIGH,
	NRF_STATE_SEND_WAITING,
	NRF_STATE_SEND_MAX_RT,
	NRF_STATE_SEND_POWER_DOWN,
	NRF_STATE_SEND_USER_DATA
};

/* BEGIN nrf registers */
struct nrf_reg_config_t {
	enum nrf_rxtx_control {
		NRF_PRIM_RX_PTX = 0, // reset value
		NRF_PRIM_RX_PRX = 1
	} PRIM_RX : 1;
	uint8_t PWR_UP : 1;
	enum nrf_crc_encoding_scheme_t CRCO : 1; // reset value 0
	uint8_t EN_CRC : 1;  // reset value 1
	uint8_t MASK_MAX_RT : 1;
	uint8_t MASK_TX_DS : 1;
	uint8_t MASK_RX_DR : 1;
	uint8_t pad: 1; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_reg_config_t, 1);

struct nrf_datapipe_payload_size_t {
	uint8_t size : 6; // 1...32, 0 pipe not used
	uint8_t pad : 2; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_datapipe_payload_size_t, 1);

struct nrf_status_t {
	uint8_t TX_FULL : 1;
	uint8_t RX_P_NO : 3; // reset value 111
	uint8_t MAX_RT : 1;
	uint8_t TX_DS : 1;
	uint8_t RX_DR : 1;
	uint8_t pad : 1; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_status_t, 1);

struct nrf_rf_ch_t {
	uint8_t RF_CH : 7; // reset value 0000010
	uint8_t pad : 1; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_rf_ch_t, 1);

struct nrf_rf_setup_t {
	uint8_t pad2 : 1; // 0
	enum nrf_tx_output_power_t power : 2;
	enum nrf_data_rate_t rate : 3; // PLL_LOCK is always 0
	uint8_t pad1 : 1; // 0
	uint8_t CONT_WAVE : 1;
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_rf_setup_t, 1);

struct nrf_addr_width_t {
	enum nrf_rxtx_addr_field_width {
		NRF_ADDR_FIELD_WIDTH_ILLEGAL = 0,
		NRF_ADDR_FIELD_WIDTH_3_BYTES = 1,
		NRF_ADDR_FIELD_WIDTH_4_BYTES = 2,
		NRF_ADDR_FIELD_WIDTH_5_BYTES = 3 // reset value
	} width : 2;
	uint8_t pad : 6; // 000000
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_addr_width_t, 1);

struct nrf_retries_t {
	uint8_t ARC : 4; // number of retries, reset value 3
	uint8_t ARD : 4; // in 250us steps, reset value 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_retries_t, 1);

struct nrf_feature_t {
	uint8_t EN_DYN_ACK : 1;
	uint8_t EN_ACK_PAY : 1;
	uint8_t EN_DPL : 1;
	uint8_t pad : 5; // 00000
};
CTASSERT_SIZE_BYTE(struct nrf_feature_t, 1);
/* END nrf registers */

struct nrf_transaction_t {
	struct spi_ctx_bare spi_ctx;
	struct sg tx_sg[2];
	struct sg rx_sg[2];
	uint8_t cmd;
	uint8_t tx_len;
	void *tx_data;
	uint8_t rx_len;
	struct nrf_status_t status;
	void *rx_data;
};

struct nrf_context_t {
	struct nrf_transaction_t trans;
	struct nrf_addr_t *addr;
	//struct nrf_addr_t *addr1;
	//uint8_t addrN[4];
	uint8_t rx_pipe;
	size_t payload_size;
	void *payload;
	nrf_data_callback *user_cb;
	enum nrf_state_t state;
	uint8_t channel;
	enum nrf_data_rate_t data_rate;
	enum nrf_tx_output_power_t power;
	enum nrf_crc_encoding_scheme_t crc_len;
	uint8_t ard;
	uint8_t arc;
	union {
		struct {
			uint8_t power_save : 1;
			uint8_t dynamic_payload : 1;
		};
		uint8_t flags;
	};
	union {
		struct {
			uint8_t addr_dirty : 1;
			uint8_t payload_size_dirty : 1;
			uint8_t channel_dirty : 1;
			uint8_t power_data_rate_dirty : 1;
			uint8_t autoretransmit_dirty : 1;
			uint8_t dynamic_payload_dirty : 1;
		};
		uint8_t dirty;
	};
};

static struct nrf_context_t nrf_ctx;

static void nrf_handle_receive(void*);
static void nrf_handle_send(void*);

#define NRF_SET_CTX(_cmd, _tx_len, _tx_data, _rx_len, _rx_data, _state)	\
	nrf_ctx.trans.cmd = _cmd;					\
	nrf_ctx.trans.tx_len = _tx_len;					\
	nrf_ctx.trans.tx_data = _tx_data;				\
	nrf_ctx.trans.rx_len = _rx_len;					\
	nrf_ctx.trans.rx_data = _rx_data;				\
	nrf_ctx.state = _state;						\

static void
send_command(struct nrf_transaction_t *trans, spi_cb *cb, void *data)
{
	struct sg *tx = trans->tx_len ?
		sg_init(trans->tx_sg, &trans->cmd, 1, trans->tx_data, trans->tx_len) :
		sg_init(trans->tx_sg, &trans->cmd, 1);
	struct sg *rx = trans->rx_len ?
		sg_init(trans->rx_sg, &trans->status, 1, trans->rx_data, trans->rx_len) :
		sg_init(trans->rx_sg, &trans->status, 1);

	spi_queue_xfer_sg(&trans->spi_ctx, NRF_SPI_CS, tx, rx, cb, data);
}

static void
handle_status(void *data)
{
	struct nrf_transaction_t *trans = data;

	if (trans->status.RX_DR && nrf_ctx.state == NRF_STATE_RECV_WAITING) {
		nrf_ctx.state = nrf_ctx.dynamic_payload ? NRF_STATE_RECV_GET_PAYLOAD_SIZE : NRF_STATE_RECV_GET_PAYLOAD;
		// nrf_ctx.rx_pipe = trans->status.RX_P_NO;
		nrf_handle_receive(NULL);
	}

	if (trans->status.TX_DS && nrf_ctx.state == NRF_STATE_SEND_WAITING) {
		nrf_ctx.state = nrf_ctx.power_save ? NRF_STATE_SEND_POWER_DOWN : NRF_STATE_SEND_USER_DATA;
		nrf_handle_send(NULL);
	}

	if (trans->status.MAX_RT && nrf_ctx.state == NRF_STATE_SEND_WAITING) {
		nrf_ctx.state = NRF_STATE_SEND_MAX_RT;
		nrf_ctx.payload = NULL;
		nrf_ctx.payload_size = 0;
		nrf_handle_send(NULL);
	}

	// clear NRF interrupt
	trans->cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_STATUS);
	trans->tx_len = 1;
	trans->tx_data = &trans->status;
	trans->rx_len = 0;
	send_command(trans, NULL, NULL);
}

void
nrf_interrupt(void *cbdata)
{
	gpio_write(NRF_CE, 0); // go to standby-1
	static struct nrf_transaction_t trans = {
		.cmd = NRF_CMD_NOP,
		.tx_len = 0,
		.rx_len = 0
	};
	send_command(&trans, handle_status, &trans); // nop it to get the status register
}

PIN_DEFINE_CALLBACK(NRF_IRQ, PIN_CHANGE_FALLING, nrf_interrupt, NULL);

void
nrf_init(void)
{
	nrf_ctx.state = 0;
	nrf_ctx.channel = 2;
	nrf_ctx.data_rate = NRF_DATA_RATE_2MBPS;
	nrf_ctx.power = NRF_TX_POWER_0DBM;
	nrf_ctx.crc_len = NRF_CRC_ENC_1_BYTE;
	nrf_ctx.ard = 0;
	nrf_ctx.arc = 3;
	nrf_ctx.power_save = 0;
	nrf_ctx.dynamic_payload = 0;
	nrf_ctx.dirty = 1;

	spi_init();

	pin_mode(NRF_CSN, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_SCK, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_MOSI, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_MISO, PIN_MODE_MUX_ALT2);

	gpio_dir(NRF_CE, GPIO_OUTPUT);
	gpio_write(NRF_CE, 0);

	gpio_dir(NRF_IRQ, GPIO_INPUT);

	pin_change_init();
}

void
nrf_set_channel(uint8_t ch)
{
	nrf_ctx.channel = ch;
	nrf_ctx.channel_dirty = 1;
}

void
nrf_set_power_datarate(enum nrf_tx_output_power_t power, enum nrf_data_rate_t data_rate)
{
	nrf_ctx.power = power;
	nrf_ctx.data_rate = data_rate;
	nrf_ctx.power_data_rate_dirty = 1;
}

void
nrf_set_autoretransmit(uint8_t delay, uint8_t count)
{
	nrf_ctx.ard = delay;
	nrf_ctx.arc = count;
	nrf_ctx.autoretransmit_dirty = 1;
}

void
nrf_enable_powersave()
{
	nrf_ctx.power_save = 1;
}

void
nrf_disable_powersave()
{
	nrf_ctx.power_save = 0;
}

void
nrf_enable_dynamic_payload()
{
	nrf_ctx.dynamic_payload = 1;
	nrf_ctx.dynamic_payload_dirty = 1;
}

void
nrf_disable_dynamic_payload()
{
	nrf_ctx.dynamic_payload = 0;
	nrf_ctx.dynamic_payload_dirty = 1;
}

void
nrf_set_crc_length(enum nrf_crc_encoding_scheme_t len)
{
	nrf_ctx.crc_len = len;
}

void
nrf_receive(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
	nrf_ctx.addr_dirty = nrf_ctx.addr ? nrf_ctx.addr->value != addr->value : 1;
	nrf_ctx.payload_size_dirty = nrf_ctx.dynamic_payload ? 0 : nrf_ctx.payload_size != len;
	nrf_ctx.addr = addr;
	nrf_ctx.rx_pipe = 1;
	nrf_ctx.payload_size = len;
	nrf_ctx.payload = data;
	nrf_ctx.user_cb = cb;
	nrf_ctx.dynamic_payload_dirty |= ((nrf_ctx.state & NRF_STATE_RECV) != NRF_STATE_RECV);
	nrf_ctx.state = NRF_STATE_SETUP_SET_POWER_RATE;
	nrf_handle_receive(NULL);
}

void
nrf_send(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
	nrf_ctx.addr_dirty = nrf_ctx.addr ? nrf_ctx.addr->value != addr->value : 1;
	nrf_ctx.addr = addr;
	nrf_ctx.payload_size = len;
	nrf_ctx.payload = data;
	nrf_ctx.user_cb = cb;
	nrf_ctx.dynamic_payload_dirty |= ((nrf_ctx.state & NRF_STATE_SEND) != NRF_STATE_SEND);
	nrf_ctx.state = NRF_STATE_SETUP_SET_POWER_RATE;
	nrf_handle_send(NULL);
}

static void
nrf_prepare_config(uint8_t power_up, uint8_t prim_rx, enum nrf_state_t next)
{
	static struct nrf_reg_config_t config = {
		.pad = 0, .MASK_MAX_RT = 0, .MASK_TX_DS = 0, .MASK_RX_DR = 0, .EN_CRC = 1
	};
	config.PRIM_RX = prim_rx;
	config.PWR_UP = power_up;
	config.CRCO = nrf_ctx.crc_len;
	NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_CONFIG),
		1, &config,
		0, NULL,
		next);
}

static void
nrf_handle_setup(void *data)
{
	switch (nrf_ctx.state) {
	default:
	case NRF_STATE_SETUP_SET_POWER_RATE:
		if (nrf_ctx.power_data_rate_dirty) {
			// ARD must be at least 500uS for 250kbps (nRF datasheet p34)
			if (nrf_ctx.data_rate == NRF_DATA_RATE_250KBPS && nrf_ctx.ard == 0) {
				nrf_ctx.ard = 1; // 500 uS
				nrf_ctx.autoretransmit_dirty = 1; // force set ARD
			}
			nrf_ctx.power_data_rate_dirty = 0;
			static struct nrf_rf_setup_t rfsetup = {
				.pad2 = 0, .pad1 = 0,
				.CONT_WAVE = 0
			};
			rfsetup.power = nrf_ctx.power;
			rfsetup.rate = nrf_ctx.data_rate;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_SETUP),
				1, &rfsetup,
				0, NULL,
				NRF_STATE_SETUP_SET_CHANNEL);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_SETUP_SET_CHANNEL:
		if (nrf_ctx.channel_dirty) {
			nrf_ctx.channel_dirty = 0;
			static struct nrf_rf_ch_t ch = {
				.pad = 0
			};
			ch.RF_CH = nrf_ctx.channel;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_CH),
				1, &ch,
				0, NULL,
				NRF_STATE_SETUP_DPL);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_SETUP_DPL:
		if  (nrf_ctx.dynamic_payload_dirty) {
			// nrf_ctx.dynamic_payload_dirty is cleared by send/recv
			static struct nrf_feature_t feat = {
				.pad = 0,
				.EN_DYN_ACK = 0,
				.EN_ACK_PAY = 0
			};
			feat.EN_DPL = nrf_ctx.dynamic_payload;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_FEATURE),
				1, &feat,
				0, NULL,
				NRF_STATE_SETUP_SET_AUTORETR);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_SETUP_SET_AUTORETR:
		if (nrf_ctx.autoretransmit_dirty) {
			nrf_ctx.autoretransmit_dirty = 0;
			static struct nrf_retries_t retr;
			retr.ARD = nrf_ctx.ard;
			retr.ARC = nrf_ctx.arc;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_SETUP_RETR),
				1, &retr,
				0, NULL,
				NRF_STATE_SETUP_DONE);
			send_command(&nrf_ctx.trans, (spi_cb*)data, NULL);
			return;
		}
		nrf_ctx.state = NRF_STATE_SETUP_DONE;
		((spi_cb*)data)(NULL); // XXX McShady
		return;
	}
	send_command(&nrf_ctx.trans, nrf_handle_setup, data);
}

static void
nrf_handle_receive(void *data)
{
	switch (nrf_ctx.state) {
	default: {
		nrf_handle_setup(nrf_handle_receive);
		return;
	}
	case NRF_STATE_SETUP_DONE:
		// FALLTHROUGH
	case NRF_STATE_RECV_SET_RX_HIGH:
		nrf_prepare_config(1, NRF_PRIM_RX_PRX, NRF_STATE_RECV_SET_PAYLOAD_SIZE);
		break;
	case NRF_STATE_RECV_SET_PAYLOAD_SIZE:
		if (nrf_ctx.payload_size_dirty) {
			nrf_ctx.payload_size_dirty = 0;
			static struct nrf_datapipe_payload_size_t dps = {
				.pad = 0
			};
			dps.size = nrf_ctx.payload_size;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & (NRF_REG_ADDR_RX_PW_P0 + nrf_ctx.rx_pipe)),
				1, &dps,
				0, NULL,
				NRF_STATE_RECV_SET_DPL);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_RECV_SET_DPL:
		if (nrf_ctx.dynamic_payload_dirty) {
			nrf_ctx.dynamic_payload_dirty = 0;
			static uint8_t dynpd;
			dynpd = nrf_ctx.dynamic_payload << nrf_ctx.rx_pipe;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_DYNPD),
				1, &dynpd,
				0, NULL,
				NRF_STATE_RECV_SET_RX_ADDRESS);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_RECV_SET_RX_ADDRESS:
		if (nrf_ctx.addr_dirty) {
			nrf_ctx.addr_dirty = 0;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & (NRF_REG_ADDR_RX_ADDR_P0 + nrf_ctx.rx_pipe)),
				nrf_ctx.addr->size, &nrf_ctx.addr->value,
				0, NULL,
				NRF_STATE_RECV_SET_CE_HIGH);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_RECV_SET_CE_HIGH:
		gpio_write(NRF_CE, 1);
		nrf_ctx.state = NRF_STATE_RECV_WAITING;
		// FALLTHROUGH
	case NRF_STATE_RECV_WAITING:
		return; // wait for interrupt
	case NRF_STATE_RECV_GET_PAYLOAD_SIZE:
		NRF_SET_CTX(NRF_CMD_R_RX_PL_WID,
			0, NULL,
			1, &nrf_ctx.payload_size,
			NRF_STATE_RECV_GET_PAYLOAD);
		break;
	case NRF_STATE_RECV_GET_PAYLOAD:
		if (nrf_ctx.dynamic_payload && nrf_ctx.payload_size > 32) {
			// something is wrong with this packet, discard it. (see nrf datasheet p30)
			nrf_ctx.payload = NULL;
			NRF_SET_CTX(NRF_CMD_FLUSH_RX,
				0, NULL,
				0, NULL,
				nrf_ctx.power_save ? NRF_STATE_RECV_POWER_DOWN : NRF_STATE_RECV_USER_DATA);
		} else {
			NRF_SET_CTX(NRF_CMD_R_RX_PAYLOAD,
				0, NULL,
				nrf_ctx.payload_size, nrf_ctx.payload,
				nrf_ctx.power_save ? NRF_STATE_RECV_POWER_DOWN : NRF_STATE_RECV_USER_DATA);
		}
		break;
	case NRF_STATE_RECV_POWER_DOWN:
		nrf_prepare_config(0, NRF_PRIM_RX_PRX, NRF_STATE_RECV_USER_DATA);
		break;
	case NRF_STATE_RECV_USER_DATA:
		nrf_ctx.user_cb(nrf_ctx.payload, nrf_ctx.payload_size);
		return;
	}
	send_command(&nrf_ctx.trans, nrf_handle_receive, NULL);
}

static void
nrf_handle_send(void *data)
{
	switch (nrf_ctx.state) {
	default: {
		nrf_handle_setup(nrf_handle_send);
		return;
	}
	case NRF_STATE_SETUP_DONE:
		// FALLTHROUGH
	case NRF_STATE_SEND_SET_RX_LOW:
		nrf_prepare_config(1, NRF_PRIM_RX_PTX, NRF_STATE_SEND_SET_TX_ADDR);
		break;
	case NRF_STATE_SEND_SET_TX_ADDR:
		if (nrf_ctx.addr_dirty) {
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_TX_ADDR),
				nrf_ctx.addr->size, &nrf_ctx.addr->value,
				0, NULL,
				NRF_STATE_SEND_SET_RX_ADDR_P0);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_SEND_SET_RX_ADDR_P0:
		if (nrf_ctx.addr_dirty) {
			nrf_ctx.addr_dirty = 0;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RX_ADDR_P0),
				nrf_ctx.addr->size, &nrf_ctx.addr->value,
				0, NULL,
				NRF_STATE_SEND_SET_DPL);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_SEND_SET_DPL:
		if (nrf_ctx.dynamic_payload_dirty) {
			nrf_ctx.dynamic_payload_dirty = 0;
			static uint8_t dynpd;
			dynpd = nrf_ctx.dynamic_payload;
			NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_DYNPD),
				1, &dynpd,
				0, NULL,
				NRF_STATE_SEND_TX_PAYLOAD);
			break;
		}
		// (else) FALLTHROUGH
	case NRF_STATE_SEND_TX_PAYLOAD:
		/* TODO if TX_FULL is 1 we could fail instantly or FLUSH_TX. */
		NRF_SET_CTX(NRF_CMD_W_TX_PAYLOAD,
			nrf_ctx.payload_size, nrf_ctx.payload,
			0, NULL,
			NRF_STATE_SEND_SET_CE_HIGH);
		break;
	case NRF_STATE_SEND_SET_CE_HIGH:
		gpio_write(NRF_CE, 1);
		nrf_ctx.state = NRF_STATE_SEND_WAITING;
		// FALLTHROUGH
	case NRF_STATE_SEND_WAITING:
		return;
	case NRF_STATE_SEND_MAX_RT:
		NRF_SET_CTX(NRF_CMD_FLUSH_TX,
			0, NULL,
			0, NULL,
			nrf_ctx.power_save ? NRF_STATE_SEND_POWER_DOWN : NRF_STATE_SEND_USER_DATA);
		break;
	case NRF_STATE_SEND_POWER_DOWN:
		nrf_prepare_config(0, NRF_PRIM_RX_PTX, NRF_STATE_SEND_USER_DATA);
		break;
	case NRF_STATE_SEND_USER_DATA:
		nrf_ctx.user_cb(nrf_ctx.payload, nrf_ctx.payload_size);
		return;
	}
	send_command(&nrf_ctx.trans, nrf_handle_send, NULL);
}

/* vim: set ts=8 sw=8 noexpandtab: */
