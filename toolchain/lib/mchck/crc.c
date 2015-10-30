#include <mchck.h>

void
crc_init(
	uint32_t seed,
	uint32_t poly,
	enum crc_width_t width,
	enum crc_transpose_t totr,
	enum crc_transpose_t tot,
	uint8_t compl_xor)
{
	SIM.scgc6.crc = 1;
	CRC.ctrl.fxor = compl_xor;
	CRC.ctrl.tcrc = width;
	CRC.ctrl.totr = totr;
	CRC.ctrl.tot = tot;
	CRC.poly = poly;
	CRC.ctrl.was = 1;
	CRC.crc = seed;
	CRC.ctrl.was = 0;
}

void
crc_update(const void *buf, size_t len)
{
	size_t n = len / 4;
	while (n-- > 0) {
		CRC.crc = *(uint32_t*)buf;
		buf += 4;
	}
	n = len % 4;
	while (n-- > 0)
		CRC.crc_ll = *(uint8_t*)buf++;
}

uint32_t
crc_value()
{
	return CRC.crc;
}
