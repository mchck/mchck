#include <mchck.h>

#include "crc.desc.h"

static struct cdc_ctx cdc;

static void
new_data(uint8_t *data, size_t len)
{
		onboard_led(ONBOARD_LED_TOGGLE);
		cdc_write(data, len, &cdc);
		for (; len > 0; ++data, --len) {
			switch (data[0]) {
			case '\r':
				printf("\r\n%x\r\n", crc_value());
				crc_init_CRC32B();
				break;
			default:
				crc_update(data, 1);
			}
		}
		cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
		cdc_init(new_data, NULL, &cdc);
		cdc_set_stdout(&cdc);
		printf("Type something and press ENTER to get the CRC32B.\r\n");
}

void
main(void)
{
		crc_init_CRC32B();
		usb_init(&cdc_device);
		sys_yield_for_frogs();
}
