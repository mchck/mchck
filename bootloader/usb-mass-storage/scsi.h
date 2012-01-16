#ifndef SCSI_H
#define SCSI_H

#include <sys/types.h>
#include "param.h"
#include <stdint.h>

enum scsi_cmd {
        SCSI_CMD_INQUIRY          = 0x12,
        SCSI_CMD_READ_CAPACITY_10 = 0x25,
        SCSI_CMD_TEST_UNIT_READY  = 0x00,
        SCSI_CMD_REQUEST_SENSE    = 0x03,
        SCSI_CMD_READ_10          = 0x28,
        SCSI_CMD_WRITE_10         = 0x2a,
};

struct scsi_cdb_test_unit_ready {
        uint8_t op;
        uint16_t reserved;
        uint8_t control;
} __packed;

struct scsi_cdb_request_sense {
        uint8_t op;
        uint8_t desc : 1;
        uint8_t reserved1 : 7;
        uint16_t reserved2;
        uint8_t alloc_len;
        uint8_t control;
} __packed;

struct scsi_cdb_inquiry {
        uint8_t op;
        uint8_t evpd : 1;
        uint8_t reserved : 7;
        uint8_t page;
        uint16_t alloc_len;
        uint8_t control;
} __packed;

struct scsi_cdb_read_capacity {
        uint8_t op;
        uint8_t reladr : 1;
        uint8_t reserved1 : 7;
        uint32_t lba;
        uint8_t reserved2[2];
        uint8_t pmi : 1;
        uint8_t reserved3 : 7;
        uint8_t control;
} __packed;

struct scsi_cdb_read_write_10 {
        uint8_t op;
        uint8_t reladr : 1;
        uint8_t reserved1 : 1;
        uint8_t ebp : 1;
        uint8_t fua : 1;
        uint8_t dpo : 1;
        uint8_t reserved2 : 3;
        uint32_t lba;
        uint8_t reserved3;
        uint16_t length;
        uint8_t control;
} __packed;

union scsi_cdb {
        uint8_t op;
        struct scsi_cdb_test_unit_ready test_unit_ready;
        struct scsi_cdb_request_sense request_sense;
        struct scsi_cdb_inquiry inquiry;
        struct scsi_cdb_read_capacity read_capacity;
        struct scsi_cdb_read_write_10 read_write_10;
};

enum scsi_sense_code {
	SCSI_SENSE_RESP_CURRENT_ERROR = 0x70,
};

enum scsi_sense_key {
        SCSI_SENSE_NO_ERROR        = 0,
        SCSI_SENSE_NOT_READY       = 2,
        SCSI_SENSE_MEDIUM_ERROR    = 3,
        SCSI_SENSE_ILLEGAL_REQUEST = 5,
        SCSI_SENSE_UNIT_ATTENTION  = 6,
};

#define SCSI_SENSE_ADD_SENSE_LEN 10

struct scsi_resp_sense_data {
        uint8_t response        : 7;
        uint8_t valid           : 1;
        uint8_t reserved1;
        uint8_t sense_key       : 3;
        uint8_t reserved2       : 1;
        uint8_t ili             : 1;
        uint8_t eom             : 1;
        uint8_t filemark        : 1;
        uint8_t information[4];
        uint8_t add_sense_len;
        uint8_t cmd_specific[4];
        uint8_t asc;
        uint8_t ascq;
        uint8_t fruc;
        uint8_t sense_key_specific[3];
        uint8_t add_bytes[];
} __packed;


enum scsi_inquiry_pdt {
        SCSI_INQUIRY_DIRECT_ACCESS = 0,
        SCSI_INQUIRY_CD_DVD        = 5,
        SCSI_INQUIRY_OPTICAL       = 7,
        SCSI_INQUIRY_RBC           = 14,
};

struct scsi_resp_inquiry {
        uint8_t type            : 5;
        uint8_t qualifier       : 3;
        uint8_t reserved1       : 6;
        uint8_t rmb             : 1;
	uint8_t version;
        uint8_t data_format     : 4;
        uint8_t hisup           : 1;
        uint8_t normal_aca      : 1;
        uint8_t reserved2       : 2;
        uint8_t add_length;
        union {
                uint8_t         b;
                struct {
                        uint8_t protect         : 1;
                        uint8_t reserved3       : 2;
                        uint8_t third_party     : 1;
                        uint8_t tpgs            : 2;
                        uint8_t acc             : 1;
                        uint8_t sccs            : 1;
                } __packed      s;
        }       info1;
        union {
                uint8_t         b;
                struct {
                        uint8_t addr16          : 1;
                        uint8_t reserved4       : 2;
                        uint8_t mchngr          : 1;
                        uint8_t multip          : 1;
                        uint8_t vs1             : 1;
                        uint8_t encserv         : 1;
                        uint8_t bque            : 1;
                } __packed      s;
        }       info2;
        union {
                uint8_t         b;
                struct {
                        uint8_t vs2             : 1;
                        uint8_t cmdque          : 1;
                        uint8_t reserved5       : 1;
                        uint8_t linked          : 1;
                        uint8_t sync            : 1;
                        uint8_t wbus16          : 1;
                        uint8_t reserved6       : 2;
                } __packed      s;
        }       info3;
        char    vendor_id[8];
        char    product_id[16];
        char    product_rev[4];
} __packed;

#define SCSI_INQUIRY_INIT(_vendor_id, _product_id, _product_rev)	\
	{							\
		.type = SCSI_INQUIRY_RBC,			\
		.version = SCSI_INQUIRY_VERSION_3,		\
		.data_format = SCSI_INQURIY_DATA_FORMAT,	\
		.add_length = sizeof(struct scsi_resp_inquiry) - 4, \
		.vendor_id = (_vendor_id),			\
		.product_id = (_product_id),			\
		.product_rev = (_product_rev),			\
	}

struct scsi_resp_read_capacity {
	uint32_t lba;
	uint32_t block_length;
} __packed;

typedef void * (*scsi_read_cb_t)(uint32_t lba, size_t len, size_t *readlen, void *private);
typedef int (*scsi_write_cb_t)(uint32_t lba, void *buf, void *private);
typedef uint32_t (*scsi_capacity_cb_t)(void *private);

enum scsi_block_length {
	SCSI_BLOCK_LENGTH = 512,
};

enum scsi_state_dir {
	SCSI_STATE_DIR_SEND,
	SCSI_STATE_DIR_RECV,
};

struct scsi_state {
	enum scsi_state_dir dir;

	uint8_t		*send_buf;
	size_t		 send_len;

	uint8_t		*recv_buf;
	size_t		 recv_pos;

	uint32_t	 cmd_lba;
	size_t		 cmd_len;

	scsi_read_cb_t	 read_cb;
	scsi_write_cb_t	 write_cb;
	scsi_capacity_cb_t capacity_cb;
        void		*cb_data;

        struct scsi_resp_inquiry	*inquiry;
        struct scsi_resp_sense_data	 sense;
	struct scsi_resp_read_capacity	 read_capacity;
};

static inline int
scsi_success(struct scsi_state *s)
{
        return (s->sense.sense_key == 0);
}

static inline size_t
scsi_remaining_length(struct scsi_state *s)
{
	return (s->send_len + s->cmd_len * SCSI_BLOCK_LENGTH);
}

static inline enum scsi_state_dir
scsi_get_dir(struct scsi_state *s)
{
	return (s->dir);
}

int scsi_read_data(struct scsi_state *s, size_t maxlen, void **buf, size_t *buflen);
int scsi_write_data(struct scsi_state *s, const void *data, size_t len);
int scsi_cmd(struct scsi_state *s, void *cdb, size_t len);
void scsi_reset(struct scsi_state *s);
void scsi_init(struct scsi_state *s);

#endif
