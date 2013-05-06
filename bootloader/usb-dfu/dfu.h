enum dfu_status {
        DFU_STATUS_OK = 0x00,
        DFU_STATUS_errTARGET = 0x01,
        DFU_STATUS_errFILE = 0x02,
        DFU_STATUS_errWRITE = 0x03,
        DFU_STATUS_errERASE = 0x04,
        DFU_STATUS_errCHECK_ERASED = 0x05,
        DFU_STATUS_errPROG = 0x06,
        DFU_STATUS_errVERIFY = 0x07,
        DFU_STATUS_errADDRESS = 0x08,
        DFU_STATUS_errNOTDONE = 0x09,
        DFU_STATUS_errFIRMWARE = 0x0a,
        DFU_STATUS_errVENDOR = 0x0b,
        DFU_STATUS_errUSBR = 0x0c,
        DFU_STATUS_errPOR = 0x0d,
        DFU_STATUS_errUNKNOWN = 0x0e,
        DFU_STATUS_errSTALLEDPKT = 0x0f
};

typedef enum dfu_status (*dfu_setup_write_t)(size_t off, size_t len, void **buf);
typedef enum dfu_status (*dfu_finish_write_t)(size_t off, size_t len);

void dfu_start(dfu_setup_write_t, dfu_finish_write_t);
int dfu_handle_control(struct usb_ctrl_req_t *);
void dfu_write_done(enum dfu_status);
