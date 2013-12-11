#include <mchck.h>
#include "t3.h"

#define I2C_DEBUG_REGS    do{}while(0)
#define I2C_DEBUG_STR(x)  do{}while(0)
#define I2C_DEBUG_STR2(x) do{}while(0)
#define I2C_DEBUG_HEX(x)  do{}while(0)
#define I2C_DEBUG_HEX2(x) do{}while(0)
#define I2C_DEBUG_WAIT    do{}while(0)

#define I2C_TX_BUFFER_LENGTH 259
#define I2C_RX_BUFFER_LENGTH 259

static uint8_t rxBuffer[I2C_TX_BUFFER_LENGTH];
static size_t rxBufferIndex;
static size_t rxBufferLength;
static uint8_t txBuffer[I2C_TX_BUFFER_LENGTH];
static size_t txBufferIndex;
static size_t txBufferLength;
static enum i2c_mode currentMode;
//static enum i2c_pins currentPins;
static enum i2c_stop currentStop;
static volatile enum i2c_status currentStatus;
static uint8_t rxAddr;
static size_t reqCount;
static uint8_t irqCount;
static void (*user_onReceive)(size_t len);
static void (*user_onRequest)(void);

//
// Initialize I2C - if config isn't specified, default to master, pins18/19, external pullup, 100kHz
//
void t3_begin(enum i2c_mode mode, uint8_t address1, uint8_t address2, enum i2c_pins pins, enum i2c_pullup pullup, enum i2c_rate rate) {
	SIM.scgc4.i2c0 = 1; // Enable I2C internal clock

    currentMode = mode; // Set mode
    currentStatus = I2C_WAITING; // reset status

    // Set Master/Slave address (zeroed in Master to prevent accidental Rx when setup is changed dynamically)
    if(currentMode == I2C_MASTER)
    {
    	I2C0.c2.hdrs = 1; // Set high drive select
        I2C0.a1.raw = 0;
        I2C0.ra.raw = 0;
    }
    else
    {
//        I2C0_C2 = (address2) ? (I2C_C2_HDRS|I2C_C2_RMEN)    // Set high drive select and range-match enable
//                             : I2C_C2_HDRS;                 // Set high drive select
    	if (address2) {
    		I2C0.c2.raw = ((struct I2C_C2) { .hdrs = 1, .rmen = 1 }).raw;
    	} else {
    		I2C0.c2.raw = ((struct I2C_C2) { .hdrs = 1 }).raw;
    	}
        // set Slave address, if two addresses are given, setup range and put lower address in A1, higher in RA
        I2C0.a1.raw = (address2) ? ((address1 < address2) ? (address1<<1) : (address2<<1))
                             : (address1<<1);
        I2C0.ra.raw = (address2) ? ((address1 < address2) ? (address2<<1) : (address1<<1))
                             : 0;
    }

	pin_mode(PIN_PTB2, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);
	pin_mode(PIN_PTB3, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);

	I2C0.f = (struct I2C_F ) { .mult = I2C_MULT_1, .icr = 0x1B };

    // Set config registers
    if(currentMode == I2C_MASTER)
    	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw; // Master - enable I2C (hold in Rx mode, intr disabled)
    else
//    	I2C0_C1 = I2C_C1_IICEN|I2C_C1_IICIE; // Slave - enable I2C and interrupts
    	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1,.iicie=1} ).raw;

	int_enable(IRQ_I2C0);

    #ifdef I2C_DEBUG
        if(!Serial) Serial.begin(115200);
        i2cDebugTimer.begin(printI2CDebug, 500); // 2kHz timer
    #endif
}

//
// Interrupt service routine
//
void I2C0_Handler(void) {
	onboard_led(ONBOARD_LED_ON);
	struct I2C_S status;
	struct I2C_C1 c1;
    uint8_t data;
    static uint8_t timeoutRxNAK = 0;

    status = I2C0.s;
    c1 = I2C0.c1;
    I2C_DEBUG_STR("I"); I2C_DEBUG_REGS; // interrupt, reg dump
    if(I2C0.c1.mst)
    {
        //
        // Master Mode
        //
        if(c1.tx)
        {
            // Continue Master Transmit
            I2C_DEBUG_STR(" MT"); // master transmit
            // check if Master Tx or Rx
            if(currentStatus == I2C_SENDING)
            {
                // check if slave ACK'd
                if(status.rxak)
                {
                    I2C_DEBUG_STR(" N"); // NAK
                    if(txBufferIndex == 0)
                        currentStatus = I2C_ADDR_NAK; // NAK on Addr
                    else
                        currentStatus = I2C_DATA_NAK; // NAK on Data
                    // send STOP, change to Rx mode, intr disabled
                    I2C_DEBUG_STR(" STOP");
                	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
                }
                else
                {
                    I2C_DEBUG_STR(" A"); // ACK
                    // check if last byte transmitted
                    if(++txBufferIndex >= txBufferLength)
                    {
                        // Tx complete, change to waiting state
                        currentStatus = I2C_WAITING;
                        // send STOP if configured
                        if(currentStop == I2C_STOP)
                        {
                            // send STOP, change to Rx mode, intr disabled
                            I2C_DEBUG_STR(" STOP");
                        	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
                        }
                        else
                        {
                            // no STOP, stay in Tx mode, intr disabled
//                            I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
                    		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;
                        }
                    }
                    else
                    {
                        // transmit next byte
                        I2C0.d = txBuffer[txBufferIndex];
                        I2C_DEBUG_STR(" D:"); I2C_DEBUG_HEX(txBuffer[txBufferIndex]); // Tx data
                    }
                }
                I2C_DEBUG_STR("\n");
                I2C0.s.iicif = 1; // clear intr
                return;
            }
            else if(currentStatus == I2C_SEND_ADDR)
            {
                // Master Receive, addr sent
                if(status.arbl)
                {
                    // Arbitration Lost
                    I2C_DEBUG_STR(" ARBL\n"); // arb lost
                    currentStatus = I2C_ARB_LOST;
//                    I2C0_C1 = I2C_C1_IICEN; // change to Rx mode, intr disabled (does this send STOP if ARBL flagged?)
                	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
//                    I2C0_S = I2C_S_ARBL | I2C_S_IICIF; // clear arbl flag and intr
                	// clear the status flags
                	I2C0.s.iicif = 1;
                	I2C0.s.arbl = 1;
                    return;
                }
                if(status.rxak)
                {
                    // Slave addr NAK
                    I2C_DEBUG_STR(" N"); // NAK
                    currentStatus = I2C_ADDR_NAK; // NAK on Addr
                    // send STOP, change to Rx mode, intr disabled
                    I2C_DEBUG_STR(" STOP");
                	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
                }
                else
                {
                    // Slave addr ACK, change to Rx mode
                    I2C_DEBUG_STR(" A"); // ACK
                    currentStatus = I2C_RECEIVING;
                    if(reqCount == 1)
//                        I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TXAK; // no STOP, Rx, NAK on recv
                		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .mst=1, .txak=1} ).raw;

                    else
//                        I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST; // no STOP, change to Rx
                		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .mst=1} ).raw;
                    data = I2C0.d; // dummy read
                }
                I2C_DEBUG_STR("\n");
                I2C0.s.iicif = 1; // clear intr
                return;
            }
            else if(currentStatus == I2C_TIMEOUT)
            {
                // send STOP if configured
                if(currentStop == I2C_STOP)
                {
                    // send STOP, change to Rx mode, intr disabled
                    I2C_DEBUG_STR(" STOP\n");
                    I2C_DEBUG_STR("Timeout\n");
                	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
                }
                else
                {
                    // no STOP, stay in Tx mode, intr disabled
                    I2C_DEBUG_STR("Timeout\n");
//                    I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
            		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;
                }
                I2C0.s.iicif = 1; // clear intr
                return;
            }
            else
            {
                // Should not be in Tx mode if not sending
                // send STOP, change to Rx mode, intr disabled
                I2C_DEBUG_STR("WTF\n");
            	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
                I2C0.s.iicif = 1; // clear intr
                return;
            }
        }
        else
        {
            // Continue Master Receive
            I2C_DEBUG_STR(" MR"); // master receive
            // check if 2nd to last byte or timeout
            if((rxBufferLength+2) == reqCount ||
               (currentStatus == I2C_TIMEOUT && !timeoutRxNAK))
//                I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TXAK; // no STOP, Rx, NAK on recv
        		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .mst=1, .txak=1} ).raw;
            // if last byte or timeout send STOP
            if((rxBufferLength+1) >= reqCount ||
               (currentStatus == I2C_TIMEOUT && timeoutRxNAK))
            {
                timeoutRxNAK = 0; // clear flag
                if(currentStatus != I2C_TIMEOUT)
                    currentStatus = I2C_WAITING; // Rx complete, change to waiting state
                // change to Tx mode
//                I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
        		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;
                // grab last data
                data = I2C0.d;
                I2C_DEBUG_STR(" D:"); I2C_DEBUG_HEX(data); // Rx data
                if(rxBufferLength < I2C_RX_BUFFER_LENGTH)
                    rxBuffer[rxBufferLength++] = data;
                if(currentStop == I2C_STOP)
                {
                    I2C_DEBUG_STR(" N STOP\n"); // NAK and STOP
//                    I2C0_C1 = I2C_C1_IICEN; // send STOP, change to Rx mode, intr disabled
                	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
                }
                else
                    I2C_DEBUG_STR(" N\n"); // NAK no STOP
                if(currentStatus == I2C_TIMEOUT)
                    I2C_DEBUG_STR("Timeout\n"); // timeout
            }
            else
            {
                // grab next data
                data = I2C0.d;
                I2C_DEBUG_STR(" D:"); I2C_DEBUG_HEX(data); // Rx data
                if(rxBufferLength < I2C_RX_BUFFER_LENGTH)
                    rxBuffer[rxBufferLength++] = data;
                I2C_DEBUG_STR(" A\n"); // not last byte, mark as ACK
            }
            if(currentStatus == I2C_TIMEOUT && !timeoutRxNAK)
                timeoutRxNAK = 1; // set flag to indicate NAK sent
            I2C0.s.iicif = 1; // clear intr
            return;
        }
    }
    else
    {
        //
        // Slave Mode
        //
        if(status.arbl)
        {
            // Arbitration Lost
            I2C_DEBUG_STR(" ARBL"); // arb lost
            I2C0.s.arbl = 1;    // clear arbl flag
            if(!(status.iaas))
            {
                I2C_DEBUG_STR("\n");
                I2C0.s.iicif = 1; // clear intr
                return;
            }
        }
        if(status.iaas)
        {
            // If in Slave Rx already, then RepSTART occurred, run callback
            if(currentStatus == I2C_SLAVE_RX && user_onReceive != NULL)
            {
                I2C_DEBUG_STR(" RSTART");
                rxBufferIndex = 0;
                user_onReceive(rxBufferLength);
            }
            // Is Addressed As Slave
            if(status.srw)
            {
                // Begin Slave Transmit
                I2C_DEBUG_STR(" AST"); // addressed slave transmit
                currentStatus = I2C_SLAVE_TX;
                txBufferLength = 0;
                if(user_onRequest != NULL)
                    user_onRequest(); // load Tx buffer with data
                I2C_DEBUG_STR(" BL:"); I2C_DEBUG_HEX(txBufferLength >> 8); I2C_DEBUG_HEX(txBufferLength); // buf len
                if(txBufferLength == 0)
                    txBuffer[0] = 0; // send 0's if buffer empty
//                I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE;
        		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1} ).raw;
                rxAddr = (I2C0.d>>1); // read to get target addr
//                I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_TX;
        		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .tx=1} ).raw;
                I2C0.d = txBuffer[0]; // send first data
                txBufferIndex = 1;
                I2C_DEBUG_STR(" D:"); I2C_DEBUG_HEX(txBuffer[0]); // Tx data
            }
            else
            {
                // Begin Slave Receive
                I2C_DEBUG_STR(" ASR"); // addressed slave receive
                irqCount = 0;
//                if(currentPins == I2C_PINS_18_19)
//                    attachInterrupt(18, sda_rising_isr, RISING);
//                else
//                    attachInterrupt(17, sda_rising_isr, RISING);
                currentStatus = I2C_SLAVE_RX;
                rxBufferLength = 0;
//                I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE;
        		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1} ).raw;
                rxAddr = (I2C0.d>>1); // read to get target addr
            }
            I2C_DEBUG_STR("\n");
            I2C0.s.iicif = 1; // clear intr
            return;
        }
        if(c1.tx)
        {
            // Continue Slave Transmit
            I2C_DEBUG_STR(" ST"); // slave transmit
            if((status.rxak) == 0)
            {
                // Master ACK'd previous byte
                I2C_DEBUG_STR(" A"); // ACK
                if(txBufferIndex < txBufferLength)
                    data = txBuffer[txBufferIndex++];
                else
                    data = 0; // send 0's if buffer empty
                I2C_DEBUG_STR(" D:"); I2C_DEBUG_HEX(data); // Tx data
                I2C0.d = data;
//                I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_TX;
        		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .tx=1} ).raw;
            }
            else
            {
                // Master did not ACK previous byte
                I2C_DEBUG_STR(" N"); // NAK
//                I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE; // switch to Rx mode
        		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1} ).raw;
                data = I2C0.d; // dummy read
                currentStatus = I2C_WAITING;
            }
        }
        else
        {
            // Continue Slave Receive
            I2C_DEBUG_STR(" SR"); // slave receive
            irqCount = 0;
//            if(currentPins == I2C_PINS_18_19)
//                attachInterrupt(18, sda_rising_isr, RISING);
//            else
//                attachInterrupt(17, sda_rising_isr, RISING);
            data = I2C0.d;
            I2C_DEBUG_STR(" D:"); I2C_DEBUG_HEX(data); // Rx data
            if(rxBufferLength < I2C_RX_BUFFER_LENGTH)
                rxBuffer[rxBufferLength++] = data;
        }
        I2C_DEBUG_STR("\n");
        I2C0.s.iicif = 1; // clear intr
    }
}

#if 0
// Detects the stop condition that terminates a slave receive transfer.
// If anyone from Freescale ever reads this code, please email me at
// paul@pjrc.com and explain how I can respond to the I2C stop without
// inefficient polling or a horrible pin change interrupt hack?!
void sda_rising_isr(void)
{
    if(!(I2C0.s.busy))
    {
        I2C_DEBUG_STR(" STOP\n"); // detected STOP
        currentStatus = I2C_WAITING;
        if(currentPins == I2C_PINS_18_19)
            detachInterrupt(18);
        else
            detachInterrupt(17);
        if(user_onReceive != NULL)
        {
            rxBufferIndex = 0;
            user_onReceive(rxBufferLength);
        }
    }
    else
    {
        if(++irqCount >= 2 || !(currentMode == I2C_SLAVE))
        {
            I2C_DEBUG_STR2(" -x-\n"); // disconnect SDA ISR
            if(currentPins == I2C_PINS_18_19)
                detachInterrupt(18);
            else
                detachInterrupt(17);
        }
        else
            I2C_DEBUG_STR2("\n");
    }
}
#endif

void t3_beginTransmission(uint8_t address) {
    txBuffer[0] = (address << 1); // store target addr
    txBufferLength = 1;
    currentStatus = I2C_WAITING; // reset status
}

//
// Master Transmit - blocking function, returns after Tx complete
// return: 0=success, 1=data too long, 2=recv addr NACK, 3=recv data NACK, 4=other error
//
uint8_t t3_endTransmission(enum i2c_stop sendStop) {
    t3_sendTransmission(sendStop);

    // wait for completion or timeout
    t3_finish();

    return 0;
}

//
// Send Master Transmit - non-blocking function, it initiates the Tx ISR then returns immediately
//                      - main loop can determine completion via polling done() or using finish()
//                        and success/fail using status()
//
void t3_sendTransmission(enum i2c_stop sendStop) {                       // non-blocking
    if(txBufferLength)
    {
        // clear the status flags
//        I2C0_S = I2C_S_IICIF | I2C_S_ARBL;
    	// clear the status flags
    	I2C0.s.iicif = 1;
    	I2C0.s.arbl = 1;

        // now take control of the bus...
        if(I2C0.c1.mst)
        {
            // we are already the bus master, so send a repeated start
            I2C_DEBUG_STR("RSTART"); // Repeated START
//            I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
    		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1, .rsta=1} ).raw;
        }
        else
        {
            // we are not currently the bus master, so wait for bus ready
    		while (I2C0.s.busy)
    			;
            // become the bus master in transmit mode (send start)
            I2C_DEBUG_STR("START"); // START
            currentMode = I2C_MASTER;
//            I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
    		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;
        }

        // send 1st data and enable interrupts
        currentStatus = I2C_SENDING;
        currentStop = sendStop;
        txBufferIndex = 0;
//        I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TX; // enable intr
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .mst=1, .tx=1} ).raw;
        I2C_DEBUG_STR(" T:"); I2C_DEBUG_HEX(txBuffer[txBufferIndex]); I2C_DEBUG_STR("\n"); // target addr
        I2C0.d = txBuffer[txBufferIndex];
    }
}

//
// Master Receive - blocking function, returns after Rx complete
// return: #bytes received, 0=fail (0 length request, NAK, timeout, or bus error)
//
size_t t3_requestFrom(uint8_t addr, size_t len, enum i2c_stop sendStop)
{
    // exit immediately if request for 0 bytes
    if(len == 0) return 0;

    t3_sendRequest(addr, len, sendStop);

    // wait for completion or timeout
    if(t3_finish())
        return rxBufferLength;
    else
        return 0; // NAK, timeout or bus error
}

//
// Start Master Receive - non-blocking function, it initiates the Rx ISR then returns immediately
//                      - main loop can determine completion via polling done() or using finish()
//                        and success/fail using status()
//
void t3_sendRequest(uint8_t addr, size_t len, enum i2c_stop sendStop) {
    // exit immediately if request for 0 bytes
    if(len == 0) return;

    reqCount=len; // store request length
    rxBufferIndex = 0; // reset buffer
    rxBufferLength = 0;

    // clear the status flags
//    I2C0_S = I2C_S_IICIF | I2C_S_ARBL;
	// clear the status flags
	I2C0.s.iicif = 1;
	I2C0.s.arbl = 1;

    // now take control of the bus...
    if(I2C0.c1.mst)
    {
        // we are already the bus master, so send a repeated start
        I2C_DEBUG_STR("RSTART"); // Repeated START
//        I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1, .rsta=1} ).raw;
    }
    else
    {
        // we are not currently the bus master, so wait for bus ready
		while (I2C0.s.busy)
			;
        // become the bus master in transmit mode (send start)
        I2C_DEBUG_STR("START"); // START
        currentMode = I2C_MASTER;
//        I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;
    }

    // send 1st data and enable interrupts
    currentStatus = I2C_SEND_ADDR;
    currentStop = sendStop;
//    I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TX; // enable intr
	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .mst=1, .tx=1} ).raw;
    uint8_t target = (addr << 1) | 1; // address + READ
    I2C_DEBUG_STR(" T:"); I2C_DEBUG_HEX(target); I2C_DEBUG_STR("\n"); // target addr
    I2C0.d = target;
}

//
// Return status
//
enum i2c_status t3_status(void) {
    return currentStatus;
}

//
// Master mode Tx/Rx done check
// return: 1=Tx/Rx complete (with or without errors), 0=still running
//
uint8_t t3_done(void) {
    return (currentStatus==I2C_WAITING ||
            currentStatus==I2C_ADDR_NAK ||
            currentStatus==I2C_DATA_NAK ||
            currentStatus==I2C_ARB_LOST);
}

//
// Master mode finish Tx/Rx - loops until timeout, I2C enters waiting state,
//                            or bus error occurs
// return: 1=success (Tx or Rx completed, no error), 0=fail (NAK, timeout or Arb Lost)
//
uint8_t t3_finish(void) {
	while(!t3_done())
		;
    return currentStatus == I2C_WAITING;
}

//
// Put single data into Tx buffer
// return: #data inserted = success, 0=fail
//
size_t t3_write(const uint8_t data)
{
    if(txBufferLength < I2C_TX_BUFFER_LENGTH)
    {
        txBuffer[txBufferLength++] = data;
        return 1;
    }
    return 0;
}

//
// Rx buffer remaining available bytes
// return: #bytes
//
int t3_available(void) {
    return rxBufferLength - rxBufferIndex;
}

//
// Read Rx buffer data
// return: data (int), or -1 if buffer empty
//
int t3_read(void) {
    if(rxBufferIndex >= rxBufferLength) return -1;
    return rxBuffer[rxBufferIndex++];
}

//
// Peek Rx buffer data
// return: data (int), or -1 if buffer empty
//
int t3_peek(void) {
    if(rxBufferIndex >= rxBufferLength) return -1;
    return rxBuffer[rxBufferIndex];
}

//
// Read Rx buffer data
// return: data (uint8_t), or 0 if buffer empty
//
uint8_t t3_readByte(void)
{
    if(rxBufferIndex >= rxBufferLength) return 0;
    return rxBuffer[rxBufferIndex++];
}

//
// Peek Rx buffer data
// return: data (uint8_t), or 0 if buffer empty
//
uint8_t t3_peekByte(void) { // returns 0 if empty
    if(rxBufferIndex >= rxBufferLength) return 0;
    return rxBuffer[rxBufferIndex];
}

//
// Flush not implemented
//
void t3_flush(void) {
}

//
// Return Rx Addr (used for Slave with address range)
// return: rxAddr
//
uint8_t t3_getRxAddr(void) {
    return rxAddr;
}

//
// set callback function for Slave Rx
//
void t3_onReceive(void (*function)(size_t len)) {
    user_onReceive = function;
}

//
// set callback function for Slave Tx
//
void t3_onRequest(void (*function)(void)) {
    user_onRequest = function;
}
