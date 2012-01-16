/**
 * Minimal support for SCSI commands, required for USB MSC.
 *
 * Supported commands:
 * 0x00 TEST UNIT READY
 * 0x03 REQUEST SENSE
 * 0x12 INQUIRY
 * 0x25 READ CAPACITY(10)
 * 0x28 READ(10)
 * 0x2a WRITE(10)
 */

#include <stdint.h>
#include <string.h>
#include "scsi.h"
#include "util.h"

static void
scsi_queue_send_data(struct scsi_state *s, void *data, size_t len, size_t maxlen)
{
	if (len > maxlen)
		len = maxlen;

	s->send_buf = data;
	s->send_len = len;
	s->cmd_len = 0;
	s->dir = SCSI_STATE_DIR_SEND;
}

static int
scsi_upcall_read(struct scsi_state *s)
{
	size_t read;

	s->send_buf = s->read_cb(s->cmd_lba, s->cmd_len, &read, s->cb_data);
	if (!s->send_buf) {
		s->send_len = 0;
		s->sense.sense_key = SCSI_SENSE_MEDIUM_ERROR;
		return (-1);
	}
	s->send_len = read * SCSI_BLOCK_LENGTH;
	s->cmd_lba += read;
	s->cmd_len -= read;
	return (0);
}

int
scsi_read_data(struct scsi_state *s, size_t maxlen, void **buf, size_t *buflen)
{
        if (s->send_len == 0 && s->cmd_len != 0)
		scsi_upcall_read(s);

	*buf = s->send_buf;
	if (*buf == NULL)
		return (-1);

	if (maxlen > s->send_len)
		maxlen = s->send_len;

	*buflen = maxlen;
	s->send_buf += maxlen;
	s->send_len -= maxlen;
	return (0);
}

int
scsi_write_data(struct scsi_state *s, const void *data, size_t len)
{
	if (s->cmd_len == 0)
		return (-1);

	memcpy(s->recv_buf + s->recv_pos, data, len);
	s->recv_pos += len;
	if (s->recv_pos == SCSI_BLOCK_LENGTH) {
		int res;

		s->cmd_lba++;
		s->cmd_len--;
		s->recv_pos = 0;
		res = s->write_cb(s->cmd_lba, s->recv_buf, s->cb_data);
		if (res < 0) {
			s->sense.sense_key = SCSI_SENSE_MEDIUM_ERROR;
			return (-1);
		}
	}
	return (0);
}

int
scsi_cmd(struct scsi_state *s, void *cdb_p, size_t len)
{
	union scsi_cdb *cdb = cdb_p;
	enum scsi_cmd cmd;
	size_t alloc_len;

	if (len < 1)
		goto error;

	cmd = cdb->op;
	switch (cmd) {
	case SCSI_CMD_TEST_UNIT_READY:
		break;

	case SCSI_CMD_INQUIRY:
		if (len < sizeof(cdb->inquiry))
			goto error;
		if (cdb->inquiry.evpd || cdb->inquiry.page != 0)
			goto error;
		scsi_queue_send_data(s, s->inquiry, sizeof(*s->inquiry),
				     betoh16(&cdb->inquiry.alloc_len));
		break;

	case SCSI_CMD_REQUEST_SENSE:
		scsi_queue_send_data(s, &s->sense, sizeof(s->sense), cdb->request_sense.alloc_len);
		break;

	case SCSI_CMD_READ_CAPACITY_10:
                scsi_queue_send_data(s, &s->read_capacity, sizeof(s->read_capacity),
				     sizeof(s->read_capacity));
                break;

	case SCSI_CMD_READ_10:
        case SCSI_CMD_WRITE_10:
                s->cmd_lba = betoh32(&cdb->read_write_10.lba);
		s->cmd_len = betoh16(&cdb->read_write_10.length);
                if (s->cmd_lba > s->read_capacity.lba ||
		    s->cmd_lba + s->cmd_len > s->read_capacity.lba) {
			s->cmd_len = 0;
			goto error;
		}
		if (cmd == SCSI_CMD_READ_10)
			s->dir = SCSI_STATE_DIR_RECV;
		else
			s->dir = SCSI_STATE_DIR_SEND;
		break;
        }

	return (0);

error:
	s->sense.sense_key = SCSI_SENSE_ILLEGAL_REQUEST;
	return (-1);
}

void
scsi_reset(struct scsi_state *s)
{
	s->cmd_len = 0;
	s->send_len = 0;

	htobe32(SCSI_BLOCK_LENGTH, &s->read_capacity.block_length);
        htobe32(s->capacity_cb(s->cb_data), &s->read_capacity.lba);

	memset(&s->sense, 0, sizeof(s->sense));
	s->sense.response = SCSI_SENSE_RESP_CURRENT_ERROR;
	s->sense.valid = 1;
	s->sense.sense_key = SCSI_SENSE_NO_ERROR;
	s->sense.add_sense_len = sizeof(s->sense) - 7;
}

void
scsi_init(struct scsi_state *s)
{
	scsi_reset(s);
}
