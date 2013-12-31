#include <mchck.h>

#include "usb-serial-loopback.desc.h"

static struct cdc_ctx cdc;

#define NPOLY 3
static uint32_t polys[NPOLY] = { 0x04c11db7, 0xedb88320, 0x82608edb };

static uint32_t
calc_crc()
{
	crc_update("aaab", 4);
	return crc_value();
}

static void
find_crc(uint32_t target)
{
	int p;
	uint32_t poly;
	enum crc_transpose_t tot;
	enum crc_transpose_t totr;
	for (p = 0; p < NPOLY; p++) {
		poly = polys[p];
		for (totr = CRC_TRANSPOSE_NONE; totr <= CRC_TRANSPOSE_BYTES; totr++) {
			for (tot = CRC_TRANSPOSE_NONE; tot <= CRC_TRANSPOSE_BYTES; tot++) {
				crc_init(0xffffffff, poly, CRC_WIDTH_32_BITS, totr, tot, 0);
				if (calc_crc() == target)
					printf("%x,%x,%u,%u,0\r\n", target, poly, totr, tot);
				crc_init(0xffffffff, poly, CRC_WIDTH_32_BITS, totr, tot, 1);
				if (calc_crc() == target)
					printf("%x,%x,%u,%u,1\r\n", target, poly, totr, tot);
			}
		}
	}
}

static void
new_data(uint8_t *data, size_t len)
{
		onboard_led(-1);
		// find_crc(0xc581c07a);
		find_crc(0x3491b4ff);
		find_crc(0x7f155185);
		// cdc_write(data, len, &cdc);
		cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
		cdc_init(new_data, NULL, &cdc);
		cdc_set_stdout(&cdc);
}

void
main(void)
{
		usb_init(&cdc_device);
		sys_yield_for_frogs();
}
