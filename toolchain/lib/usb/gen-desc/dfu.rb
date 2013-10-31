class DFUDesc < FunctionDesc
  TypeName = "struct dfu_function_desc"
  FunctionVarName = "dfu_function"

  child_block :dfu

  def initialize
    super

    interface(:iface) {
      bInterfaceClass :USB_DEV_CLASS_APP
      bInterfaceSubClass :USB_DEV_SUBCLASS_APP_DFU
      bInterfaceProtocol :USB_DEV_PROTO_DFU_DFU
    }
  end

  def gen_desc_init
    <<_end_
.#@var_name = {
	#{@interface.first.gen_desc_init}
	.dfu = {
		.bLength = sizeof(struct dfu_desc_functional),
		.bDescriptorType = {
			.id = 0x1,
			.type_type = USB_DESC_TYPE_CLASS
		},
		.will_detach = 1,
		.manifestation_tolerant = 0,
		.can_upload = 0,
		.can_download = 1,
		.wDetachTimeOut = 0,
		.wTransferSize = USB_DFU_TRANSFER_SIZE,
		.bcdDFUVersion = { .maj = 1, .min = 1 }
	}
},
_end_
  end
end
