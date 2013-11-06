#include <mchck.h>

struct UART_t {
        struct UART_BDH_t {
                UNION_STRUCT_START(8);
                uint8_t sbrh:5;
                uint8_t _pad:1;
                uint8_t rxedgie:1;
                uint8_t lbkdie:1;
                UNION_STRUCT_END;
        } bdh;
        struct UART_BDL_t {
                UNION_STRUCT_START(8);
                uint8_t sbrl:8;
                UNION_STRUCT_END;
        } bdl;
        struct UART_C1_t {
                UNION_STRUCT_START(8);
                uint8_t pt:1;
                uint8_t pe:1;
                uint8_t ilt:1;
                uint8_t wake:1;
                uint8_t m:1;
                uint8_t rsrc:1;
                uint8_t uartswai:1;
                uint8_t loops:1;
                UNION_STRUCT_END;
        } c1;
        struct UART_C2_t {
                UNION_STRUCT_START(8);
                uint8_t sbk:1;
                uint8_t rwu:1;
                uint8_t re:1;
                uint8_t te:1;
                uint8_t ilie:1;
                uint8_t rie:1;
                uint8_t tcie:1;
                uint8_t tie:1;
                UNION_STRUCT_END;
        } c2;
        struct UART_S1_t {
                UNION_STRUCT_START(8);
                uint8_t pf:1;
                uint8_t fe:1;
                uint8_t nf:1;
                uint8_t overrun:1;
                uint8_t idle:1;
                uint8_t rdrf:1;
                uint8_t tc:1;
                uint8_t tdre:1;
                UNION_STRUCT_END;
        } s1;
        struct UART_S2_t {
                UNION_STRUCT_START(8);
                uint8_t raf:1;
                uint8_t lbkde:1;
                uint8_t brk13:1;
                uint8_t rwuid:1;
                uint8_t rxinv:1;
                uint8_t msbf:1;
                uint8_t rxedgif:1;
                uint8_t lbkdif:1;
                UNION_STRUCT_END;
        } s2;
        struct UART_C3_t {
                UNION_STRUCT_START(8);
                uint8_t peie:1;
                uint8_t feie:1;
                uint8_t neie:1;
                uint8_t orie:1;
                uint8_t txinv:1;
                uint8_t txdir:1;
                uint8_t t8:1;
                uint8_t r8:1;
                UNION_STRUCT_END;
        } c3;
        uint8_t d;
        struct UART_MA_t {
                UNION_STRUCT_START(16);
                uint8_t ma1;
                uint8_t ma2;
                UNION_STRUCT_END;
        } ma;
        struct UART_C4_t {
                UNION_STRUCT_START(8);
                uint8_t brfa:5;
                uint8_t m10:1;
                uint8_t maen2:1;
                uint8_t maen1:1;
                UNION_STRUCT_END;
        } c4;
        struct UART_C5_t {
                UNION_STRUCT_START(8);
                uint8_t _pad0:5;
                uint8_t rdmas:1;
                uint8_t _pad1:1;
                uint8_t tdmas:1;
                UNION_STRUCT_END;
        } c5;
        struct UART_ED_t {
                UNION_STRUCT_START(8);
                uint8_t _pad0:6;
                uint8_t paritye:1;
                uint8_t noisy:1;
                UNION_STRUCT_END;
        } ed;
        struct UART_MODEM_t {
                UNION_STRUCT_START(8);
                uint8_t txctse:1;
                uint8_t txrtse:1;
                uint8_t txrtspol:1;
                uint8_t rxrtse:1;
                uint8_t _pad0:4;
                UNION_STRUCT_END;
        } modem;
        struct UART_IR_t {
                UNION_STRUCT_START(8);
                uint8_t tnp:2;
                uint8_t iren:1;
                uint8_t _pad0:5;
                UNION_STRUCT_END;
        } ir;
        uint8_t _pad0;
        /* FIFO registers */
        struct UART_PFIFO_t {
                UNION_STRUCT_START(8);
                uint8_t rxfifosize:3;
                uint8_t rxfe:1;
                uint8_t txfifosize:3;
                uint8_t txfe:1;
                UNION_STRUCT_END;
        } pfifo;
        struct UART_CFIFO_t {
                UNION_STRUCT_START(8);
                uint8_t rxufe:1;
                uint8_t txofe:1;
                uint8_t rxofe:1;
                uint8_t _pad:3;
                uint8_t rxflush:1;
                uint8_t txflush:1;
                UNION_STRUCT_END;
        } cfifo;
        struct UART_SFIFO_t {
                UNION_STRUCT_START(8);
                uint8_t rxuf:1;
                uint8_t txof:1;
                uint8_t rxof:1;
                uint8_t _pad:3;
                uint8_t rxempt:1;
                uint8_t txempt:1;
                UNION_STRUCT_END;
        } sfifo;
        uint8_t twfifo;
        uint8_t tcfifo;
        uint8_t rwfifo;
        uint8_t rcfifo;
} __packed;

CTASSERT_SIZE_BYTE(struct UART_t, 23);

extern volatile struct UART_t UART0;
extern volatile struct UART_t UART1;
extern volatile struct UART_t UART2;
