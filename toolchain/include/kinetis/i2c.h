struct I2C {
	struct I2C_A1 {
		UNION_STRUCT_START(8);
		uint8_t _rsvd0   : 1;
		uint8_t ad       : 7;
		UNION_STRUCT_END;
	} a1;
	struct I2C_F {
		UNION_STRUCT_START(8);
		uint8_t icr      : 6;
		enum {
			I2C_MULT_1 = 0b00,
			I2C_MULT_2 = 0b01,
			I2C_MULT_4 = 0b10
		} mult            : 2;

		UNION_STRUCT_END;
	} f;
	struct I2C_C1 {
		UNION_STRUCT_START(8);
		uint8_t dmaen    : 1;
		uint8_t wuen     : 1;
		uint8_t rsta     : 1;
		uint8_t txak     : 1;
		uint8_t tx       : 1;
		uint8_t mst      : 1;
		uint8_t iicie    : 1;
		uint8_t iicen    : 1;
		UNION_STRUCT_END;
	} c1;
	struct I2C_S {
		UNION_STRUCT_START(8);
		uint8_t rxak     : 1;
		uint8_t iicif    : 1;
		uint8_t srw      : 1;
		uint8_t ram      : 1;
		uint8_t arbl     : 1;
		uint8_t busy     : 1;
		uint8_t iaas     : 1;
		uint8_t tcf      : 1;
		UNION_STRUCT_END;
	} s;
	uint8_t d;
	struct I2C_C2 {
		UNION_STRUCT_START(8);
		uint8_t ad       : 3;
		uint8_t rmen     : 1;
		uint8_t sbrc     : 1;
		uint8_t hdrs     : 1;
		uint8_t adext    : 1;
		uint8_t gcaen    : 1;
		UNION_STRUCT_END;
	} c2;
	struct I2C_FLT {
		UNION_STRUCT_START(8);
		uint8_t flt      : 5;
		uint8_t _rsvd0   : 3;
		UNION_STRUCT_END;
	} flt;
	struct I2C_RA {
		UNION_STRUCT_START(8);
		uint8_t _rsvd0   : 1;
		uint8_t rad      : 7;
		UNION_STRUCT_END;
	} ra;
	struct I2C_SMB {
		UNION_STRUCT_START(8);
		uint8_t shtf2ie  : 1;
		uint8_t shtf2    : 1;
		uint8_t shtf1    : 1;
		uint8_t sltf     : 1;
		uint8_t tcksel   : 1;
		uint8_t siicaen  : 1;
		uint8_t alerten  : 1;
		uint8_t fack     : 1;
		UNION_STRUCT_END;
	} smb;
	struct I2C_A2 {
		UNION_STRUCT_START(8);
		uint8_t _rsvd0  : 1;
		uint8_t sad     : 7;
		UNION_STRUCT_END;
	} a2;
	uint8_t slth;
	uint8_t sltl;
};

CTASSERT_SIZE_BYTE(struct I2C, 0x0B+1);

extern volatile struct I2C I2C0;
