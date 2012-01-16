#include "util.h"

#define betoh(s)						\
	uint ## s ## _t						\
	betoh ## s(uint ## s ## _t *in)				\
	{							\
		uint8_t *ina = (uint8_t *)in;			\
		uint ## s ## _t r = 0;				\
								\
		for (int i = 0; i < sizeof(r); ++i, ++ina)	\
			r = (r << 8) | *ina;			\
		return (r);					\
	}

betoh(32)
betoh(16)

#define htobe(s)							\
	void								\
	htobe ## s(uint ## s ## _t v, uint ## s ## _t *out)		\
	{								\
		uint8_t *p = (uint8_t *)out;				\
									\
		for (int i = sizeof(v); i >= 0; --i, v >>= 8, ++p)	\
			*p = v & 0xff;					\
	}

htobe(32)
htobe(16)
