#include <mchck.h>

void
crc_init(
	uint32_t seed,
	uint32_t poly,
	uint8_t width,
	enum crc_transpose_t totr,
	enum crc_transpose_t tot)
{
	SIM.scgc6.crc = 1;
	CRC.ctrl._pad = 0;
	CRC.ctrl._res = 0;
	CRC.ctrl.fxor = 0;
	CRC.ctrl.tcrc = width;
	CRC.ctrl.totr = totr;
	CRC.ctrl.tot = tot;
	CRC.poly = poly;
	CRC.ctrl.was = 1;
	CRC.crc = seed;
	CRC.ctrl.was = 0;
}

void
crc_update(uint32_t value)
{
	CRC.crc = value;
}

uint32_t
crc_value()
{
	return CRC.crc;
}
