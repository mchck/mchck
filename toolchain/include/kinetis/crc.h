#include <mchck.h>

struct CRC_t {
	union {
		struct {
			uint8_t crc_ll;
			uint8_t crc_lu;
			uint8_t crc_hl;
			uint8_t crc_hu;
		};
		uint32_t crc;
	};
	union {
		struct {
			uint16_t poly_low;
			uint16_t poly_high;
		};
		uint32_t poly;
	};
	struct {
		UNION_STRUCT_START(32);
		uint32_t _pad : 24;
		uint32_t tcrc : 1;
		uint32_t was  : 1;
		uint32_t fxor : 1;
		uint32_t _res : 1;
		uint32_t totr : 2;
		uint32_t tot  : 2;
		UNION_STRUCT_END;
	} ctrl;
};

CTASSERT_SIZE_BYTE(struct CRC_t, 3 * 4);

extern volatile struct CRC_t CRC;
