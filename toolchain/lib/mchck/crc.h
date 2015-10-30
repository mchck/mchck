enum crc_width_t {
	CRC_WIDTH_16_BITS = 0x0,
	CRC_WIDTH_32_BITS = 0x1
};

enum crc_transpose_t {
	CRC_TRANSPOSE_NONE      = 0x0,
	CRC_TRANSPOSE_BITS      = 0x1,
	CRC_TRANSPOSE_BITSBYTES = 0x2,
	CRC_TRANSPOSE_BYTES     = 0x3
};

void crc_init(
	uint32_t seed,
	uint32_t poly,
	enum crc_width_t width,
	enum crc_transpose_t totr,
	enum crc_transpose_t tot,
	uint8_t compl_xor);
void crc_update(const void *buf, size_t len);
uint32_t crc_value();

inline void
crc_init_CRC32B()
{
	crc_init(0xffffffff, 0x04c11db7, CRC_WIDTH_32_BITS, CRC_TRANSPOSE_BITSBYTES, CRC_TRANSPOSE_BITSBYTES, 1);
}

inline void
crc_init_CRC32()
{
	crc_init(0xffffffff, 0x04c11db7, CRC_WIDTH_32_BITS, CRC_TRANSPOSE_BYTES, CRC_TRANSPOSE_BYTES, 1);
}
