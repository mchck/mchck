/* Import the library from the swd/ folder first */
#include <swd.h>

const int BUFF_SIZE = 10;
const int LED = 13;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
}

void loop() {
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

size_t reply_space(void) {
  return BUFF_SIZE; // XXX can always write (I hope)
}

void reply_write(const uint8_t *buf, size_t len) {
  Serial.write(buf, len);
}

static void
new_data(uint8_t *data, size_t len)
{
  static uint8_t buf[BUFF_SIZE];
  static size_t buflen;
  digitalWrite(LED, 1);
  while (buflen + len > 0) {
    size_t copylen = sizeof(buf) - buflen;

    if (len < copylen)
      copylen = len;
    memcpy(buf + buflen, data, copylen);
    len -= copylen;
    buflen += copylen;
    data += copylen;

    const uint8_t *p = process_buf(buf, buflen);
    if (p == buf)
      break;
    buflen -= p - buf;
    memcpy(buf, p, buflen);
  }
  digitalWrite(LED, 0);
}

void serialEvent() {
  uint8_t buff[BUFF_SIZE];
  int n = 0;
  while (Serial.available()) {
    buff[n++] = (uint8_t)Serial.read();
    if (n >= BUFF_SIZE) {
      new_data(buff, n);
      n = 0;
    }
  }
  if (n > 0)
    new_data(buff, n);
}
