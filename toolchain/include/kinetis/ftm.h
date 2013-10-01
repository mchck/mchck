/* there are quite a few things that are a 32-bit word with the first 8 bits
 * representing a bit per channel, so here's a generic struct to represent it.
 */
struct FTM_PERCHANNEL_t {
	UNION_STRUCT_START(32);
	uint32_t ch0	: 1;
	uint32_t ch1	: 1;
	uint32_t ch2	: 1;
	uint32_t ch3	: 1;
	uint32_t ch4	: 1;
	uint32_t ch5	: 1;
	uint32_t ch6	: 1;
	uint32_t ch7	: 1;
	uint32_t _rsvd0 : 24;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct FTM_PERCHANNEL_t, 32);

struct FTM_t {
	struct FTM_SC_t {
		UNION_STRUCT_START(32);
		enum FTM_PS {
			FTM_PS_DIV1   = 0,
			FTM_PS_DIV2   = 1,
			FTM_PS_DIV4   = 2,
			FTM_PS_DIV8   = 3,
			FTM_PS_DIV16  = 4,
			FTM_PS_DIV32  = 5,
			FTM_PS_DIV64  = 6,
			FTM_PS_DIV128 = 7
		} ps		: 3;
		enum FTM_CLKS {
			FTM_CLKS_NONE	= 0,
			FTM_CLKS_SYSTEM = 1,
			FTM_CLKS_FIXED	= 2,
			FTM_CLKS_EXT	= 3
		} clks		: 2;
		uint32_t cpwms	: 1;
		uint32_t toie	: 1;
		uint32_t tof	: 1;
		uint32_t _rsvd0 : 24;
		UNION_STRUCT_END;
	} sc;
	uint16_t cnt;
	uint16_t _pad0;
	uint16_t mod;
	uint16_t _pad1;
	struct {
		struct FTM_CSC_t {
			UNION_STRUCT_START(32);
			uint32_t dma	: 1;
			uint32_t _rsvd0 : 1;
			uint32_t elsa	: 1;
			uint32_t elsb	: 1;
			uint32_t msa	: 1;
			uint32_t msb	: 1;
			uint32_t chie	: 1;
			uint32_t chf	: 1;
			uint32_t _rsvd1 : 24;
			UNION_STRUCT_END;
		} csc;
		struct {
			uint32_t cv     : 16;
			uint32_t _rsvd0 : 16;
		};
	} channel[8];
	struct {
		uint32_t cntin : 16;
		uint32_t _pad2 : 16;
	};
	struct FTM_PERCHANNEL_t status;
	struct FTM_MODE_t {
		UNION_STRUCT_START(32);
		uint32_t ftmen	 : 1;
		uint32_t init	 : 1;
		uint32_t wpdis	 : 1;
		uint32_t pwmsync : 1;
		uint32_t captest : 1;
		enum FTM_FAULTM_t {
			FTM_FAULTM_DISABLED    = 0,
			FTM_FAULTM_EVEN_MANUAL = 1,
			FTM_FAULTM_ALL_MANUAL  = 2,
			FTM_FAULTM_ALL_AUTO    = 1
		} faultm;
		uint32_t faultie : 1;
		UNION_STRUCT_END;
	} mode;
	struct FTM_SYNC_t {
		UNION_STRUCT_START(32);
		uint32_t cntmin	 : 1;
		uint32_t cntmax	 : 1;
		uint32_t reinit	 : 1;
		uint32_t synchom : 1;
		uint32_t trig0	 : 1;
		uint32_t trig1	 : 1;
		uint32_t trig2	 : 1;
		uint32_t swsync	 : 1;
		uint32_t _rsvd0	 : 24;
		UNION_STRUCT_END;
	} sync;
	struct FTM_PERCHANNEL_t outinit;
	struct FTM_PERCHANNEL_t outmask;
	struct FTM_COMBINE_t {
		UNION_STRUCT_START(8);
		uint8_t combine : 1;
		uint8_t comp	: 1;
		uint8_t decapen : 1;
		uint8_t decap	: 1;
		uint8_t dten	: 1;
		uint8_t syncen	: 1;
		uint8_t faulten : 1;
		uint8_t _rsvd0	: 1;
		UNION_STRUCT_END;
	} combine[4];
	struct FTM_DEADTIME_t {
		UNION_STRUCT_START(32);
		uint32_t dtval	: 6;
		enum FTM_DTPS_t {
			FTM_DTPS_DIV1  = 0, /* both 0 and 1 cause a divisor of one */
			FTM_DTPS_DIV4  = 2,
			FTM_DTPS_DIV16 = 3
		} dtps		: 2;
		uint32_t _rsvd0 : 24;
		UNION_STRUCT_END;
	} deadtime;
	struct FTM_EXTTRIG_t {
		UNION_STRUCT_START(32);
		uint32_t ch2trig    : 1;
		uint32_t ch3trig    : 1;
		uint32_t ch4trig    : 1;
		uint32_t ch0trig    : 1;
		uint32_t ch1trig    : 1;
		uint32_t inittrigen : 1;
		uint32_t trigf	    : 1;
		uint32_t _rsvd0	    : 24;
		UNION_STRUCT_END;
	} exttrig;
	struct FTM_PERCHANNEL_t pol;
	struct FTM_FMS_t {
		UNION_STRUCT_START(32);
		uint32_t faultf0 : 1;
		uint32_t faultf1 : 1;
		uint32_t faultf2 : 1;
		uint32_t faultf3 : 1;
		uint32_t _rsvd0	 : 1;
		uint32_t faultin : 1;
		uint32_t wpen	 : 1;
		uint32_t faultf	 : 1;
		uint32_t _rsvd1	 : 16;
		UNION_STRUCT_END;
	} fms;
	struct FTM_FILTER_t {
		UNION_STRUCT_START(32);
		uint32_t ch0fval : 4;
		uint32_t ch1fval : 4;
		uint32_t ch2fval : 4;
		uint32_t ch3fval : 4;
		uint32_t _rsvd0	 : 16;
		UNION_STRUCT_END;
	} filter;
	struct FTM_FLTCTRL_t {
		UNION_STRUCT_START(32);
		uint32_t fault0en : 1;
		uint32_t fault1en : 1;
		uint32_t fault2en : 1;
		uint32_t fault3en : 1;
		uint32_t ffltr0en : 1;
		uint32_t ffltr1en : 1;
		uint32_t ffltr2en : 1;
		uint32_t ffltr3en : 1;
		uint32_t ffval	  : 4;
		uint32_t _rsvd0	  : 20;
		UNION_STRUCT_END;
	} fltctrl;
	struct FTM_QDCTRL_t {
		UNION_STRUCT_START(32);
		uint32_t quaden	   : 1;
		uint32_t tofdir	   : 1;
		uint32_t quadir	   : 1;
		uint32_t quadmode  : 1;
		uint32_t phbpol	   : 1;
		uint32_t phapol	   : 1;
		uint32_t phbfltren : 1;
		uint32_t phafltren : 1;
		uint32_t _rsvd0	   : 24;
		UNION_STRUCT_END;
	} qdctrl;
	struct FTM_CONF_t {
		UNION_STRUCT_START(32);
		uint32_t numtof	 : 5;
		uint32_t _rsvd0	 : 1;
		enum FTM_BDMMODE_t {
			/* XXX better names */
			FTM_BDMMODE_00,
			FTM_BDMMODE_01,
			FTM_BDMMODE_10,
			FTM_BDMMODE_11
		} bdmmode : 2;
		uint32_t _rsvd1	 : 1;
		uint32_t gtbeen	 : 1;
		uint32_t gtbeout : 1;
		uint32_t _rsvd2	 : 21;
		UNION_STRUCT_END;
	} conf;
	struct FTM_FTLPOL_t {
		UNION_STRUCT_START(32);
		uint32_t ftl0pol : 1;
		uint32_t ftl1pol : 1;
		uint32_t ftl2pol : 1;
		uint32_t ftl3pol : 1;
		uint32_t _rsvd0	 : 28;
		UNION_STRUCT_END;
	} fltpol;
	struct FTM_SYNCONF_t {
		UNION_STRUCT_START(32);
		uint32_t hwtrigmode : 1;
		uint32_t _rsvd0	    : 1;
		uint32_t cntinc	    : 1;
		uint32_t _rsvd1	    : 1;
		uint32_t invc	    : 1;
		uint32_t swoc	    : 1;
		uint32_t _rsvd2	    : 1;
		uint32_t syncmode   : 1;
		uint32_t swrstcnt   : 1;
		uint32_t swwrbuf    : 1;
		uint32_t swom	    : 1;
		uint32_t swinvc	    : 1;
		uint32_t swsoc	    : 1;
		uint32_t _rsvd3	    : 3;
		uint32_t hwrstcnt   : 1;
		uint32_t hwwrbuf    : 1;
		uint32_t hwom	    : 1;
		uint32_t hwsoc	    : 1;
		uint32_t _rsvd4	    : 11;
		UNION_STRUCT_END;
	} synconf;
	struct STM_INVCTRL_t {
		UNION_STRUCT_START(32);
		uint32_t inv0en : 1;
		uint32_t inv1en : 1;
		uint32_t inv2en : 1;
		uint32_t inv3en : 1;
		uint32_t _rsvd0 : 28;
		UNION_STRUCT_END;
	} invctrl;
	struct STM_SWOCTRL_t {
		UNION_STRUCT_START(32);
		uint32_t oc	: 8;
		uint32_t ocv	: 8;
		uint32_t _rsvd0 : 16;
		UNION_STRUCT_END;
	} swoctrl;
	struct FTM_PWMLOAD_t {
		UNION_STRUCT_START(32);
		uint32_t ch0sel : 1;
		uint32_t ch1sel : 1;
		uint32_t ch2sel : 1;
		uint32_t ch3sel : 1;
		uint32_t ch4sel : 1;
		uint32_t ch5sel : 1;
		uint32_t ch6sel : 1;
		uint32_t ch7sel : 1;
		uint32_t _rsvd0 : 1;
		uint32_t ldok	: 1;
		uint32_t _rsvd1 : 22;
		UNION_STRUCT_END;
	} pwmload;
};
CTASSERT_SIZE_BYTE(struct FTM_t, 0x98+4);

extern volatile struct FTM_t FTM0;
extern volatile struct FTM_t FTM1;
