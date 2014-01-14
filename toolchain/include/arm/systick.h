// 2014-01-06 @stg, http://www.github.com/stg

struct SYSTICK_t {
	struct SYSTICK_CTRL {
		UNION_STRUCT_START(32);
		uint32_t enable    :  1;
		uint32_t tickint   :  1;
		uint32_t clksource :  1;
		uint32_t _rsvd0    : 13;
		uint32_t countflag :  1;
		uint32_t _rsvd1    : 15;
		UNION_STRUCT_END;
	} ctrl;	
	struct SYSTICK_LOAD {
		UNION_STRUCT_START(32);
		uint32_t reload : 24;
		uint32_t _rsvd0 :  8;
		UNION_STRUCT_END;
	} load;	
	struct SYSTICK_VAL {
		UNION_STRUCT_START(32);
		uint32_t current : 24;
		uint32_t _rsvd0  :  8;
		UNION_STRUCT_END;
	} val;	
	struct SYSTICK_CALIB {
		UNION_STRUCT_START(32);
		uint32_t tenms  : 24;
		uint32_t _rsvd0 :  6;
		uint32_t skew   :  1;
		uint32_t nored  :  1;
		UNION_STRUCT_END;
	} calib;	
};

CTASSERT_SIZE_BYTE(struct SYSTICK_t, 0x10);

extern volatile struct SYSTICK_t SYSTICK;