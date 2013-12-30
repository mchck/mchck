enum crc_transpose_t {
	CRC_TRANSPOSE_NONE      = 0x0,
	CRC_TRANSPOSE_BITS      = 0x1,
	CRC_TRANSPOSE_BITSBYTES = 0x2,
	CRC_TRANSPOSE_BYTES     = 0x3
};

void crc_init(
	uint32_t seed,
	uint32_t poly,
	uint8_t width,
	enum crc_transpose_t totr,
	enum crc_transpose_t tot,
	uint8_t compl_xor);
void crc_update(uint32_t value);
uint32_t crc_value();

inline void
crc_init_CRC32()
{
	crc_init(0xffffffff, 0x04C11DB7, 1, CRC_TRANSPOSE_NONE, CRC_TRANSPOSE_NONE, 0);
}
