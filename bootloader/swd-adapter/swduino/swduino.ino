#include "swd.h"

const int BUFF_SIZE = 10;
const int LED = 13;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
}

void loop() {
}

void signal_led(void) {
  digitalWrite(LED, !digitalRead(LED));
}

void pin_configure(enum swd_pin pin, enum swd_pin_mode mode) {
  pinMode(pin, mode);
}

void pin_write(enum swd_pin pin, int val) {
  digitalWrite(pin, val);
}

int pin_read(enum swd_pin pin) {
  return digitalRead(pin);
}

int outpipe_space(size_t len) {
  return 1; // XXX can always write (I hope)
}

void reply_write(const uint8_t *buf, size_t len) {
  Serial.write(buf, len);
}

void serialEvent() {
  while (Serial.available()) {
    uint8_t data = Serial.read();
    process_data(&data, 1);
  }
}
