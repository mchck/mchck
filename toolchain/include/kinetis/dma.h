// 2014-01-08 @stg, http://www.github.com/stg

#define DMAMEM __attribute__ ((section(".dmabuffers"), used))

struct DMAMUX_t {
	struct DMAMUX_CHCFG_t {
		UNION_STRUCT_START(8);
		enum DMAMUX_SOURCE_e {
			DMAMUX_SOURCE_DISABLED =  0,
			DMAMUX_SOURCE_UART0_RX =  2,
			DMAMUX_SOURCE_UART0_TX =  3,
			DMAMUX_SOURCE_UART1_RX =  4,
			DMAMUX_SOURCE_UART1_TX =  5,
			DMAMUX_SOURCE_UART2_RX =  6,
			DMAMUX_SOURCE_UART2_TX =  7,
			DMAMUX_SOURCE_I2S0_RX  = 14,
			DMAMUX_SOURCE_I2S0_TX  = 15,
			DMAMUX_SOURCE_SPI0_RX  = 16,
			DMAMUX_SOURCE_SPI0_TX  = 17,
			DMAMUX_SOURCE_I2C0     = 22,
			DMAMUX_SOURCE_FTM0_CH0 = 24,
			DMAMUX_SOURCE_FTM0_CH1 = 25,
			DMAMUX_SOURCE_FTM0_CH2 = 26,
			DMAMUX_SOURCE_FTM0_CH3 = 27,
			DMAMUX_SOURCE_FTM0_CH4 = 28,
			DMAMUX_SOURCE_FTM0_CH5 = 29,
			DMAMUX_SOURCE_FTM0_CH6 = 30,
			DMAMUX_SOURCE_FTM0_CH7 = 31,
			DMAMUX_SOURCE_FTM1_CH0 = 32,
			DMAMUX_SOURCE_FTM1_CH1 = 33,
			DMAMUX_SOURCE_ADC0     = 40,
		} source : 6;
		uint8_t trig   : 1;
		uint8_t enbl   : 1;
		UNION_STRUCT_END;
	} chcfg[16];
};

CTASSERT_SIZE_BYTE(struct DMAMUX_t, 0x10);

extern volatile	struct DMAMUX_t DMAMUX;

struct DMA_t {
	struct DMA_CR_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 :  1;
		uint32_t edbg   :  1;
		uint32_t erca   :  1;
		uint32_t _rsvd1 :  1;
		uint32_t hoe    :  1;
		uint32_t halt   :  1;
		uint32_t clm    :  1;
		uint32_t emlm   :  1;
		uint32_t _rsvd2 :  8;
		uint32_t ecx    :  1;
		uint32_t cx     :  1;
		uint32_t _rsvd3 : 14;
		UNION_STRUCT_END;
	} cr;               // 0x40008000
	struct DMA_ES_t {
		UNION_STRUCT_START(32);
		uint32_t dbe    :  1;
		uint32_t sbe    :  1;
		uint32_t sge    :  1;
		uint32_t nce    :  1;
		uint32_t doe    :  1;
		uint32_t dae    :  1;
		uint32_t soe    :  1;
		uint32_t sae    :  1;
		uint32_t errchn :  2;
		uint32_t _rsvd0 :  4;
		uint32_t cpe    :  1;
		uint32_t _rsvd1 :  1;
		uint32_t ecx    :  1;
		uint32_t _rsvd2 : 14;
		uint32_t vld    :  1;
		UNION_STRUCT_END;
	}	es;			          // 0x40008004
	uint32_t _pad0;     // 0x40008008
	struct DMA_ERQ_t {
		UNION_STRUCT_START(32);
		uint32_t erq0   :  1;
		uint32_t erq1   :  1;
		uint32_t erq2   :  1;
		uint32_t erq3   :  1;
		uint32_t _rsvd0 : 28;
		UNION_STRUCT_END;
	}	erq;			        // 0x4000800C
	uint32_t _pad1;		  // 0x40008010
	struct DMA_EEI_t {
		UNION_STRUCT_START(32);
		uint32_t eei0   :  1;
		uint32_t eei1   :  1;
		uint32_t eei2   :  1;
		uint32_t eei3   :  1;
		uint32_t _rsvd0 : 28;
		UNION_STRUCT_END;
	}	eei;			        // 0x40008014
	struct DMA_CEEI_t {
		UNION_STRUCT_START(8);
		uint8_t ceei   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t caee   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	ceei;		          // 0x40008018
		struct DMA_SEEI_t {
		UNION_STRUCT_START(8);
		uint8_t seei   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t saee   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	seei;             // 0x40008019
		struct DMA_CERQ_t {
		UNION_STRUCT_START(8);
		uint8_t cerq   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t caer   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	cerq;             // 0x4000801A
		struct DMA_SERQ_t {
		UNION_STRUCT_START(8);
		uint8_t serq   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t saer   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	serq;             // 0x4000801B
		struct DMA_CDNE_t {
		UNION_STRUCT_START(8);
		uint8_t cdne   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t cadn   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	cdne;             // 0x4000801C
		struct DMA_SSRT_t {
		UNION_STRUCT_START(8);
		uint8_t ssrt   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t sast   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	ssrt;		          // 0x4000801D
		struct DMA_CERR_t {
		UNION_STRUCT_START(8);
		uint8_t cerr   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t caei   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	cerr;             // 0x4000801E
		struct DMA_CINT_t {
		UNION_STRUCT_START(8);
		uint8_t cint   :  2;
		uint8_t _rsvd0 :  4;
		uint8_t cair   :  1;
		uint8_t nop    :  1;
		UNION_STRUCT_END;
	}	cint;             // 0x4000801F
	uint32_t _pad2;		  // 0x40008020
	struct DMA_INTR_t {
		UNION_STRUCT_START(32);
		uint32_t int0   :  1;
		uint32_t int1   :  1;
		uint32_t int2   :  1;
		uint32_t int3   :  1;
		uint32_t _rsvd0 : 28;
		UNION_STRUCT_END;
	}	intr;		          // 0x40008024
	uint32_t _pad3;		  // 0x40008028
	struct DMA_ERR_t {
		UNION_STRUCT_START(32);
		uint32_t err0   :  1;
		uint32_t err1   :  1;
		uint32_t err2   :  1;
		uint32_t err3   :  1;
		uint32_t _rsvd0 : 28;
		UNION_STRUCT_END;
	} err;			        // 0x4000802C
	uint32_t _pad4;		  // 0x40008030
	struct DMA_HRS_t {
		UNION_STRUCT_START(32);
		uint32_t hrs0   :  1;
		uint32_t hrs1   :  1;
		uint32_t hrs2   :  1;
		uint32_t hrs3   :  1;
		uint32_t _rsvd0 : 28;
		UNION_STRUCT_END;
	} hrs;			        // 0x40008034
  uint32_t _pad5[50]; // 0x40008038
	struct DMA_DCHPRI_t {
		UNION_STRUCT_START(8);
		uint8_t chpri  :  2;
		uint8_t _rsvd0 :  4;
		uint8_t dpa    :  1;
		uint8_t ecp    :  1;
		UNION_STRUCT_END;
	} dchpri[4];	       // 0x400080B8
	uint32_t _pad6[959];
	struct DMA_TCD_t {
		const volatile void * saddr;
		uint16_t  soff;
	  struct DMA_ATTR_t {
		  UNION_STRUCT_START(16);
		  enum DMA_SIZE_e {
				DMA_SIZE_8BIT   = 0, // 8-bit
			  DMA_SIZE_16BIT  = 1, // 16-bit
			  DMA_SIZE_32BIT  = 2, // 32-bit
			  DMA_SIZE_3      = 3, // Reserved
			  DMA_SIZE_16BYTE = 4, // 16-byte
			  DMA_SIZE_32BYTE = 5, // 32-byte
			  DMA_SIZE_6      = 6, // Reserved
			  DMA_SIZE_7      = 7, // Reserved
			}        dsize  :  3;
			uint16_t dmod   :  5;
			enum DMA_SIZE_e
			         ssize  :   3;
			uint16_t smod   :   5;
		  UNION_STRUCT_END;
	  } attr;
		union {
			uint32_t mlno;
			struct DMA_MLOFFNO_t {
				UNION_STRUCT_START(32);
				uint32_t nbytes : 30;
				uint32_t dmloe  :  1;
				uint32_t smloe  :  1;
				UNION_STRUCT_END;
			} mloffno;
			struct DMA_MLOFFYES_t {
				UNION_STRUCT_START(32);
				uint32_t nbytes : 10;
				uint32_t mloff  : 20;
				uint32_t dmloe  :  1;
				uint32_t smloe  :  1;
				UNION_STRUCT_END;
			} mloffyes;
		} nbytes;
		const volatile void * slast;
		const volatile void * daddr;
		uint16_t  doff;
		union DMA_ITER_u {
			struct DMA_ELINKYES_t {
				UNION_STRUCT_START(16);
				uint16_t iter  : 9;
				uint16_t linkch : 2;
				uint16_t _rsvd0 : 4;
				uint16_t elink  : 1;
				UNION_STRUCT_END;
			} elinkyes;
			struct DMA_ELINKNO_t {
				UNION_STRUCT_START(16);
				uint16_t iter : 15;
				uint16_t elink :  1;
				UNION_STRUCT_END;
			} elinkno;
		} citer;
		uint32_t  dlastsga;
		struct DMA_CSR_t {
			UNION_STRUCT_START(16);
			uint16_t start       : 1;
			uint16_t intmajor    : 1;
			uint16_t inthalf     : 1;
			uint16_t dreq        : 1;
			uint16_t esg         : 1;
			uint16_t majorlink   : 1;
			uint16_t active      : 1;
			uint16_t done        : 1;
			uint16_t majorlinkch : 2;
			uint16_t _rsvd0      : 4;
			uint16_t bwc         : 2;
			UNION_STRUCT_END;
		} csr;
		union DMA_ITER_u biter;
	} tcd[4];						// 0x40009000
};

CTASSERT_SIZE_BYTE(struct DMA_t, 0x1080);

extern volatile	struct DMA_t DMA;