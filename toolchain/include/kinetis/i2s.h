// 2014-01-02 @stg, http://www.github.com/stg

struct I2S {
	struct I2S_TCSR	{
		UNION_STRUCT_START(32);
		uint32_t frde   :  1;
		uint32_t fwde   :  1;
		uint32_t _rsvd0 :  6;
		uint32_t frie   :  1;
		uint32_t fwie   :  1;
		uint32_t feie   :  1;
		uint32_t seie   :  1;
		uint32_t wsie   :  1;
		uint32_t _rsvd1 :  3;
		uint32_t frf    :  1;
		uint32_t fwf    :  1;
		uint32_t fef    :  1;
		uint32_t sef    :  1;
		uint32_t wsf    :  1;
		uint32_t _rsvd2 :  3;
		uint32_t sr     :  1;
		uint32_t fr     :  1;
		uint32_t _rsvd3 :  2;
		uint32_t bce    :  1;
		uint32_t dbge   :  1;
		uint32_t stope  :  1;
		uint32_t te     :  1;
		UNION_STRUCT_END;
	} tcsr;
	struct I2S_TCR1  {
		UNION_STRUCT_START(32);
		uint32_t tfw    :  2;
		uint32_t _rsvd0 : 30;
		UNION_STRUCT_END;
	} tcr1;
	struct I2S_TCR2  {
		UNION_STRUCT_START(32);
		uint32_t div    :  8;
		uint32_t _rsvd0 : 16;
		uint32_t bcd    :  1;
		uint32_t bcp    :  1;
		enum I2S_MSEL_e {
			I2S_MSEL_BUSCLK  = 0,
			I2S_MSEL_I2SMCLK = 1,
			I2S_MSEL_2       = 2, // Unsupported
			I2S_MSEL_3       = 3  // Unsupported
		}        msel   :  2;
		uint32_t bci    :  1;
		uint32_t bcs    :  1;
		enum {
			I2S_TCR_SYNC_ASYNC = 0,
			I2S_TCR_SYNC_RX    = 1,
			I2S_TCR_SYNC_SAITX = 2,
			I2S_TCR_SYNC_SAIRX = 3
		}        sync   :  2;
		UNION_STRUCT_END;
	} tcr2;
	struct I2S_TCR3  {
		UNION_STRUCT_START(32);
		uint32_t wdfl   :  4;
		uint32_t _rsvd0 : 12;
		uint32_t tce    :  1;
		uint32_t _rsvd1 : 15;
		UNION_STRUCT_END;
	} tcr3;
	struct I2S_TCR4  {
		UNION_STRUCT_START(32);
		uint32_t fsd    :  1;
		uint32_t fsp    :  1;
		uint32_t _rsvd0 :  1;
		uint32_t fse    :  1;
		uint32_t mf     :  1;
		uint32_t _rsvd1 :  3;
		uint32_t sywd   :  5;
		uint32_t _rsvd2 :  3;
		uint32_t frsz   :  4;
		uint32_t _rsvd3 : 12;
		UNION_STRUCT_END;
	} tcr4;
	struct I2S_TCR5  {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 :  8;
		uint32_t fbt    :  5;
		uint32_t _rsvd1 :  3;
		uint32_t w0w    :  5;
		uint32_t _rsvd2 :  3;
		uint32_t wnw    :  5;
		uint32_t _rsvd3 :  3;
		UNION_STRUCT_END;
	} tcr5;
	uint32_t _pad0[2];
	uint32_t tdr;
	uint32_t _pad1[7];
	struct I2S_TFR {
		UNION_STRUCT_START(32);
		uint32_t rfp    :  3;
		uint32_t _rsvd0 : 13;
		uint32_t wfp    :  3;
		uint32_t _rsvd1 : 13;
		UNION_STRUCT_END;
	} tcr;
	uint32_t _pad2[7];
	struct I2S_TMR {
		UNION_STRUCT_START(32);
		uint32_t twm    : 16;
		uint32_t _rsvd0 : 16;
		UNION_STRUCT_END;
	} tmr;
	uint32_t _pad3[7];
	struct I2S_RCSR  {
		UNION_STRUCT_START(32);
		uint32_t frde   :  1;
		uint32_t fwde   :  1;
		uint32_t _rsvd0 :  6;
		uint32_t frie   :  1;
		uint32_t fwie   :  1;
		uint32_t feie   :  1;
		uint32_t seie   :  1;
		uint32_t wsie   :  1;
		uint32_t _rsvd1 :  3;
		uint32_t frf    :  1;
		uint32_t fwf    :  1;
		uint32_t fef    :  1;
		uint32_t sef    :  1;
		uint32_t wsf    :  1;
		uint32_t _rsvd2 :  3;
		uint32_t sr     :  1;
		uint32_t fr     :  1;
		uint32_t _rsvd3 :  2;
		uint32_t bce    :  1;
		uint32_t dbge   :  1;
		uint32_t stope  :  1;
		uint32_t re     :  1;
		UNION_STRUCT_END;
	} rcsr;
	struct I2S_RCR1  {
		UNION_STRUCT_START(32);
		uint32_t rfw    :  2;
		uint32_t _rsvd0 : 30;
		UNION_STRUCT_END;
	} rcr1;
	struct I2S_RCR2  {
		UNION_STRUCT_START(32);
		uint32_t div    :  8;
		uint32_t _rsvd0 : 16;
		uint32_t bcd    :  1;
		uint32_t bcp    :  1;
		enum I2S_MSEL_e
		         msel   :  2;
		uint32_t bci    :  1;
		uint32_t bcs    :  1;
		enum {
			I2S_RCR_SYNC_ASYNC = 0,
			I2S_RCR_SYNC_TX    = 1,
			I2S_RCR_SYNC_SAIRX = 2,
			I2S_RCR_SYNC_SAITX = 3
		}        sync   :  2;
		UNION_STRUCT_END;
	} rcr2;
	struct I2S_RCR3  {
		UNION_STRUCT_START(32);
		uint32_t wdfl   :  4;
		uint32_t _rsvd0 : 12;
		uint32_t rce    :  1;
		uint32_t _rsvd1 : 15;
		UNION_STRUCT_END;
	} rcr3;
	struct I2S_RCR4  {
		UNION_STRUCT_START(32);
		uint32_t fsd    :  1;
		uint32_t fsp    :  1;
		uint32_t _rsvd0 :  1;
		uint32_t fse    :  1;
		uint32_t mf     :  1;
		uint32_t _rsvd1 :  3;
		uint32_t sywd   :  5;
		uint32_t _rsvd2 :  3;
		uint32_t frsz   :  4;
		uint32_t _rsvd3 : 12;
		UNION_STRUCT_END;
	} rcr4;
	struct I2S_RCR5  {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 :  8;
		uint32_t fbt    :  5;
		uint32_t _rsvd1 :  3;
		uint32_t w0w    :  5;
		uint32_t _rsvd2 :  3;
		uint32_t wnw    :  5;
		uint32_t _rsvd3 :  3;
		UNION_STRUCT_END;
	} rcr5;
	uint32_t _pad4[2];
	uint32_t rdr;
	uint32_t _pad5[7];
	struct I2S_RFR {
		UNION_STRUCT_START(32);
		uint32_t rfp    :  3;
		uint32_t _rsvd0 : 13;
		uint32_t wfp    :  3;
		uint32_t _rsvd1 : 13;
		UNION_STRUCT_END;
	} rfr;
	uint32_t _pad6[7];
	struct I2S_RMR {
		UNION_STRUCT_START(32);
		uint32_t rwm    : 16;
		uint32_t _rsvd0 : 16;
		UNION_STRUCT_END;
	} rmr;
	uint32_t _pad7[7];
	struct I2S_MCR {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 24;
		enum	{
			I2S_MICS_SYSCLK       =  0,
			I2S_MICS_OSC0ERCLK    =  1,
			I2S_MICS_2            =  2, //  Unsupported
			I2S_MICS_MCGPLLFLLCLK =  3
		}        mics   :  2;
		uint32_t _rsvd1 :  4;
		uint32_t moe    :  1;
		uint32_t duf    :  1;
		UNION_STRUCT_END;
	} mcr;
	struct I2S_MDR {
		UNION_STRUCT_START(32);
		uint32_t div    : 12;
		uint32_t mul    :  8;
		uint32_t _rsvd0 : 12;
		UNION_STRUCT_END;
	} mdr;
};

CTASSERT_SIZE_BYTE(struct I2S, 0x108);

extern volatile	struct I2S I2S0;
