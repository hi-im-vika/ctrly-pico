#include <Arduino.h>
#include "UART.hpp"
#include "Radio.hpp"
#include "types.hpp"

#define UART1_RX    9
#define UART1_TX    8

InputFrame input = {};
TelemetryFrame tm = {};

void setup() {
  UART::init();

  Serial2.setRX(UART1_RX);
  Serial2.setTX(UART1_TX);
  Serial2.begin();

  Radio::init();
  Serial2.println("Radio config OK");
  delay(1000);
}

void loop() {
  UART::read_serial(input);
  if (UART::get_input_status()) {
    Radio::transmit(input);
  }
}

void loop1() {
  TelemetryFrame tf = { Radio::get_tx_cnt(), UART::get_rx_cnt(), Radio::get_fail_cnt() };
  UART::write_serial(tf);
  delay(1);
}