#include <Arduino.h>
#include <SPI.h>
#include "Radio.hpp"
#include "types.hpp"

#define UART1_RX    9
#define UART1_TX    8

InputFrame input = {};
TelemetryFrame tm = {};

bool new_input = false;
unsigned long last_report = millis();

/*---Count Variables---*/ 
unsigned long rx_count = 0;

void read_serial() {
  uint8_t pos = 0;
  uint8_t buf[FRAME_SIZE + 2];  // fs + COBS overhead + delimiter

  while (Serial.available() && !new_input) {
    uint8_t b = Serial.read();
    if (b == 0x00) {
      if (pos == FRAME_SIZE + 1) {
        uint8_t decoded[FRAME_SIZE];
        size_t decoded_len = COBS::decode(buf, pos, decoded);
        memcpy(&input, decoded, decoded_len);
        new_input = true;
        rx_count++;
      }
      pos = 0;
    } else {
      if (pos < sizeof(buf)) {
        buf[pos++] = b;
      } else {
        pos = 0;
        // fail_count++;
      }
    }
  }
}

void write_serial() {
  if (Serial) {
    uint8_t buf[TM_FRAME_SIZE + 2];
    TelemetryFrame tf = { Radio::get_tx_cnt(), rx_count, Radio::get_fail_cnt() };
    size_t encoded_len = COBS::encode(&tf,TM_FRAME_SIZE,buf);
    buf[TM_FRAME_SIZE + 1] = 0x00;
    Serial.write(buf,TM_FRAME_SIZE + 2);
    // Serial.printf("Hello from TXP, total_tx: %d, total_rx: %d, total_fail: %d\r\n", tx_count, rx_count, fail_count);
  }
  delay(1);
}

void setup() {
  Serial.begin();

  Serial2.setRX(UART1_RX);
  Serial2.setTX(UART1_TX);
  Serial2.begin();

  Radio::init();
  Serial2.println("Radio config OK");
  delay(1000);
}

void loop() {
  read_serial();
  if (new_input) {
    new_input = false;
    Radio::transmit(input);
  }
}

void loop1() {
  write_serial();
}