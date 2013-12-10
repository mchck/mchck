#define BUFFER_LENGTH 32

struct wire_s {
    uint8_t rxBuffer[BUFFER_LENGTH];
    uint8_t rxBufferIndex;
    uint8_t rxBufferLength;

    uint8_t txAddress;
    uint8_t txBuffer[BUFFER_LENGTH];
    uint8_t txBufferIndex;
    uint8_t txBufferLength;

    uint8_t transmitting;
};

void wire_begin();
void wire_beginTransmission(uint8_t);
uint8_t wire_endTransmission(uint8_t);
uint8_t wire_requestFrom(uint8_t, uint8_t, uint8_t);
size_t wire_write(uint8_t);
int wire_read(void);
int wire_available(void);

