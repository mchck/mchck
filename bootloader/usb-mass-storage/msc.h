#ifndef MSC_H
#define MSC_H

#include <sys/types.h>
#include "param.h"
#include <stdint.h>
#include "scsi.h"
#include "usb.h"

enum msc_ep_size {
	MSC_EP_SIZE = 64,
};

enum msc_req {
	MSC_REQ_RESET = 255,
	MSC_REQ_GET_MAX_LUN = 254,
};


enum msc_cbw_signature {
        MSC_CBW_SIGNATURE = 0x43425355,
};

enum msc_cbw_length {
        MSC_CBW_LENGTH = 31,
};

enum msc_cbw_cdb_length {
        MSC_CBW_CDB_LENGTH_MIN = 1,
        MSC_CBW_CDB_LENGTH_MAX = 16,
};

struct msc_cbw {
        uint32_t        signature;
        uint32_t        tag;
        uint32_t        data_length;
        uint8_t         reserved1       : 7;
        uint8_t         in              : 1;
        uint8_t         reserved2       : 4;
        uint8_t         lun             : 4;
        uint8_t         reserved3       : 3;
        uint8_t         length          : 5;
        uint8_t         cdb[MSC_CBW_CDB_LENGTH_MAX];
} __packed;

enum msc_csw_signature {
        MSC_CSW_SIGNATURE = 0x53425355,
};

enum msc_csw_status {
        MSC_STATUS_PASSED = 0,
        MSC_STATUS_FAILED = 1,
        MSC_STATUS_PHASE_ERROR = 2,
};

struct msc_csw {
        uint32_t        signature;
        uint32_t        tag;
        uint32_t        residue;
        uint8_t         status;
} __packed;

struct msc_state {
	struct usb_endpoint *control;
	struct usb_endpoint *in;
	struct usb_endpoint *out;

	struct scsi_state scsi_state;
	
        enum {
                MSC_STATE_ALL_GOOD = 0,
                MSC_STATE_IDLE,
                MSC_STATE_RECV,
                MSC_STATE_SEND,
                MSC_STATE_FAULT,
        } state;
        struct msc_cbw cbw;
        struct msc_csw csw;
        uint8_t cbw_length;
};

int msc_bulk_out(const void *data, size_t len, struct msc_state *s);
int msc_bulk_in(struct msc_state *s);
int msc_control(const struct usb_request *req, struct msc_state *s);
int msc_init(struct msc_state *s);

#endif
