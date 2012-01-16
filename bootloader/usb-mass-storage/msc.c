#include <stdint.h>
#include <string.h>
#include "msc.h"


static enum msc_csw_status
msc_cbw_valid(const struct msc_cbw *cbw, size_t len)
{
	if (len != MSC_CBW_LENGTH ||
	    cbw->signature != MSC_CBW_SIGNATURE) {
		return (MSC_STATE_FAULT);
	} else {
		return (MSC_STATE_ALL_GOOD);
	}
}

static enum msc_csw_status
msc_cbw_meaningful(const struct msc_cbw *cbw)
{
	if (cbw->lun != 0 ||
	    cbw->length > MSC_CBW_CDB_LENGTH_MAX ||
	    cbw->length < MSC_CBW_CDB_LENGTH_MIN ||
	    cbw->reserved1 != 0) {
		return (MSC_STATE_FAULT);
	} else {
		return (MSC_STATE_ALL_GOOD);
	}
}

static void
msc_send_csw(struct msc_state *s)
{
        s->csw.signature = MSC_CSW_SIGNATURE;
        s->csw.tag = s->cbw.tag;

        if (s->state != MSC_STATE_FAULT) {
		if (scsi_success(&s->scsi_state))
			s->csw.status = MSC_STATUS_PASSED;
		else
			s->csw.status = MSC_STATUS_FAILED;

                s->state = MSC_STATE_IDLE;
        }

        usb_in(s->in, &s->csw, sizeof(s->csw), sizeof(s->csw));
}

static void
msc_send_bulk_data(struct msc_state *s)
{
        void *buf;
        size_t len;

	if (s->cbw.data_length == 0) {
		msc_send_csw(s);
		return;
	}

	scsi_read_data(&s->scsi_state, MSC_EP_SIZE, &buf, &len);

	if (len > s->cbw.data_length)
		len = s->cbw.data_length;

	s->cbw.data_length -= len;
	s->csw.residue -= len;

	usb_in(s->in, buf, len, MSC_EP_SIZE);
}

int
msc_bulk_out(const void *data, size_t len, struct msc_state *s)
{
        size_t minlen = len;

        switch (s->state) {
	case MSC_STATE_IDLE:
		/* We're idle and we got a new cbw */

		s->state = msc_cbw_valid(data, len);
		if (s->state == MSC_STATE_ALL_GOOD)
			s->state = msc_cbw_meaningful(data);
		if (s->state != MSC_STATE_ALL_GOOD)
			goto error;
		memcpy(&s->cbw, data, len);

		scsi_cmd(&s->scsi_state, &s->cbw.cdb, s->cbw.length);
		s->csw.residue = scsi_remaining_length(&s->scsi_state);

		if (s->cbw.in) {
                        s->state = MSC_STATE_SEND;
                        if (scsi_get_dir(&s->scsi_state) == SCSI_STATE_DIR_RECV ||
			    s->csw.residue > s->cbw.data_length)
				goto error;

			msc_send_bulk_data(s);
		} else {
                        s->state = MSC_STATE_RECV;
                        if (scsi_get_dir(&s->scsi_state) == SCSI_STATE_DIR_SEND ||
                            s->csw.residue > s->cbw.data_length)
                                goto out_error;

                        if (s->cbw.data_length == 0)
                                msc_send_csw(s);
                }
                break;

        case MSC_STATE_RECV:
		if (len != MSC_EP_SIZE)
			goto error;

		if (minlen > s->cbw.data_length)
			minlen = s->cbw.data_length;

		s->cbw.data_length -= minlen;
		scsi_write_data(&s->scsi_state, data, minlen);

		if (s->cbw.data_length == 0)
                        msc_send_csw(s);
		break;

	default:
		goto error;
	}

out:
	return (0);

error:
        usb_stall(s->in);
	if (0) {		/* only exit goto target */
out_error:
		usb_stall(s->out);
	}
	
        s->csw.status = MSC_STATUS_PHASE_ERROR;
        s->state = MSC_STATE_FAULT;
	return (-1);
}

/**
 * msc_bulk_in is called when the *previous* out packet has been
 * ACK'ed by the host, and more space is free in the EP.
 *
 * The most common case of this would be while we're in "in",
 * i.e. send mode.  We will also be called after we sent the csw; in
 * this case we'll already be in MSC_STATE_IDLE.  In all other cases
 * (RECV, FAULT) we are experiencing a phase error.
 */
int
msc_bulk_in(struct msc_state *s)
{
        switch (s->state) {
        case MSC_STATE_SEND:
		msc_send_bulk_data(s);
		break;

        case MSC_STATE_IDLE:
                break;

        default:
                goto error;
        }

        return (0);

error:
	s->csw.status = MSC_STATUS_PHASE_ERROR;
        s->state = MSC_STATE_FAULT;
        usb_stall(s->in);
        return (-1);
}

int
msc_control(const struct usb_request *req, struct msc_state *s)
{
        uint8_t zero = 0;

        switch ((enum msc_req)req->request) {
	case MSC_REQ_RESET:
		scsi_reset(&s->scsi_state);
		s->state = MSC_STATE_IDLE;
		break;

	case MSC_REQ_GET_MAX_LUN:
		usb_in(s->control, &zero, 1, 1);
		break;

	default:
		return (-1);
	}

	return (0);
}

int
msc_init(struct msc_state *s)
{
	s->state = MSC_STATE_IDLE;
	scsi_init(&s->scsi_state);
}
