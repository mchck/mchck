void wire_begin();
void wire_beginTransmission(uint8_t);
uint8_t wire_endTransmission(uint8_t);
uint8_t wire_requestFrom(uint8_t, uint8_t, uint8_t);
size_t wire_write(uint8_t);
int wire_read(void);
int wire_available(void);
