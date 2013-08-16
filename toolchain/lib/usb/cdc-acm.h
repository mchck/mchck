#define CDC_NOTICE_EP	1
#define CDC_NOTICE_SIZE 8

#define CDC_TX_EP	2
#define CDC_TX_SIZE	64

#define CDC_RX_EP	1
#define CDC_RX_SIZE 32

struct cdc_ctx {
        struct usbd_function_ctx_header header;
        struct usbd_ep_pipe_state_t *notice_pipe;
        struct usbd_ep_pipe_state_t *tx_pipe;
        struct usbd_ep_pipe_state_t *rx_pipe;
        void (*data_sent_cb)(size_t);
        void (*data_ready_cb)(uint8_t *, size_t);
        size_t out_pos;
        size_t out_sent;
        int out_queued;
        uint8_t outbuf[CDC_TX_SIZE];
        uint8_t inbuf[CDC_RX_SIZE];
};

enum cdc_dev_class {
        USB_DEV_CLASS_CDC = 0x02,
        USB_DEV_CLASS_CDC_DCD = 0x0a
};

enum cdc_dev_subclass {
        USB_DEV_SUBCLASS_CDC_DLC = 0x01,
        USB_DEV_SUBCLASS_CDC_ACM = 0x02
};

struct cdc_desc_function_common_t {
        uint8_t bLength;
        struct usb_desc_type_t bDescriptorType; /* = class CDC/0x4 CS_INTERFACE */
        enum {
                USB_CDC_SUBTYPE_HEADER = 0x00,
                USB_CDC_SUBTYPE_UNION = 0x06
        } bDescriptorSubtype;
} __packed;

struct cdc_desc_function_header_t {
        struct cdc_desc_function_common_t;
        struct usb_bcd_t bcdCDC; /* 1.2 */
} __packed;
CTASSERT_SIZE_BYTE(struct cdc_desc_function_header_t, 5);

struct cdc_desc_function_union_t {
        struct cdc_desc_function_common_t;
        uint8_t bControlInterface;
        uint8_t bSubordinateInterface0;
};
CTASSERT_SIZE_BYTE(struct cdc_desc_function_header_t, 5);

struct cdc_function_desc {
        struct usb_desc_iface_t ctrl_iface;
        struct cdc_desc_function_header_t cdc_header;
        struct cdc_desc_function_union_t cdc_union;
        struct usb_desc_ep_t ctrl_ep;
        struct usb_desc_iface_t  data_iface;
        struct usb_desc_ep_t tx_ep;
        struct usb_desc_ep_t rx_ep;
};

#define USB_FUNCTION_CDC_IFACE_COUNT	2

#define USB_FUNCTION_DESC_CDC_DECL \
        struct cdc_function_desc

#define USB_FUNCTION_DESC_CDC(state...)                                 \
        {                                                               \
        .ctrl_iface = {                                                 \
                .bLength = sizeof(struct usb_desc_iface_t),             \
                .bDescriptorType = USB_DESC_IFACE,                      \
                .bInterfaceNumber = USB_FUNCTION_IFACE(0, state),       \
                .bAlternateSetting = 0,                                 \
                .bNumEndpoints = 1,                                     \
                .bInterfaceClass = USB_DEV_CLASS_CDC,                   \
                .bInterfaceSubClass = USB_DEV_SUBCLASS_CDC_ACM,         \
                .bInterfaceProtocol = 0,                                \
                .iInterface = 0                                         \
        },                                                              \
                .cdc_header = {                                         \
                .bLength = sizeof(struct cdc_desc_function_header_t),   \
                .bDescriptorType = {                                    \
                        .id = USB_DESC_IFACE,                           \
                        .type_type = USB_DESC_TYPE_CLASS                \
                },                                                      \
                .bDescriptorSubtype = USB_CDC_SUBTYPE_HEADER,           \
                .bcdCDC = { .maj = 1, .min = 1 }                        \
        },                                                              \
                .cdc_union = {                                          \
                .bLength = sizeof(struct cdc_desc_function_union_t),    \
                .bDescriptorType = {                                    \
                        .id = USB_DESC_IFACE,                           \
                        .type_type = USB_DESC_TYPE_CLASS                \
                },                                                      \
                .bDescriptorSubtype = USB_CDC_SUBTYPE_UNION,            \
                .bControlInterface = USB_FUNCTION_IFACE(0, state), /* this interface */ \
                .bSubordinateInterface0 = USB_FUNCTION_IFACE(1, state) /* next interface */ \
        },                                                              \
                .ctrl_ep = {                                            \
                .bLength = sizeof(struct usb_desc_ep_t),                \
                .bDescriptorType = USB_DESC_EP,                         \
                .ep_num = USB_FUNCTION_TX_EP(CDC_NOTICE_EP, state),     \
                .in = 1,                                                \
                .type = USB_EP_INTR,                                    \
                .wMaxPacketSize = CDC_NOTICE_SIZE,                      \
                .bInterval = 0xff                                       \
        },                                                              \
                .data_iface = {                                         \
                .bLength = sizeof(struct usb_desc_iface_t),             \
                .bDescriptorType = USB_DESC_IFACE,                      \
                .bInterfaceNumber = USB_FUNCTION_IFACE(1, state),       \
                .bAlternateSetting = 0,                                 \
                .bNumEndpoints = 2,                                     \
                .bInterfaceClass = USB_DEV_CLASS_CDC_DCD,               \
                .bInterfaceSubClass = 0,                                \
                .bInterfaceProtocol = 0, /* no specific protocol */     \
                .iInterface = 0                                         \
        },                                                              \
                .tx_ep = {                                              \
                .bLength = sizeof(struct usb_desc_ep_t),                \
                .bDescriptorType = USB_DESC_EP,                         \
                .ep_num = USB_FUNCTION_TX_EP(CDC_TX_EP, state),         \
                .in = 1,                                                \
                .type = USB_EP_BULK,                                    \
                .wMaxPacketSize = CDC_TX_SIZE,                          \
                .bInterval = 0xff                                       \
        },                                                              \
                .rx_ep = {                                              \
                .bLength = sizeof(struct usb_desc_ep_t),                \
                .bDescriptorType = USB_DESC_EP,                         \
                .ep_num = USB_FUNCTION_RX_EP(CDC_RX_EP, state),         \
                .in = 0,                                                \
                .type = USB_EP_BULK,                                    \
                .wMaxPacketSize = CDC_RX_SIZE,                          \
                .bInterval = 0xff                                       \
        }                                                               \
}

struct cdc_ctx;

const struct usbd_function cdc_function;

void cdc_read_more(struct cdc_ctx *ctx);
size_t cdc_write_space(struct cdc_ctx *ctx);
ssize_t cdc_write(const uint8_t *buf, size_t len, struct cdc_ctx *ctx);
void cdc_init(void (*data_ready_cb)(uint8_t *, size_t), void (*data_sent_cb)(size_t), struct cdc_ctx *ctx);
