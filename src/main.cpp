#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>

#define SPI0_MISO   0
#define SPI0_CSN    1
#define SPI0_SCK    2
#define SPI0_MOSI   3
#define RF24_INT    4
#define RF24_CE     5

RF24 radio(RF24_CE, SPI0_CSN);
uint8_t address[][6] = { "1Node", "2Node" };

bool radioNumber = 0;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

float payload = 0.0;

void setup() {
  Serial2.setRX(9);
  Serial2.setTX(8);
  Serial2.begin();

  SPI.setMISO(SPI0_MISO);
  SPI.setCS(SPI0_CSN);
  SPI.setSCK(SPI0_SCK);
  SPI.setMOSI(SPI0_MOSI);

  while (!Serial2) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial2.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  radioNumber = 0;
  Serial2.print(F("radioNumber = "));
  Serial2.println((int)radioNumber);
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes

  // set the TX address of the RX node for use on the TX pipe (pipe 0)
  radio.stopListening(address[radioNumber]);  // put radio in TX mode

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1
}

void loop() {
  unsigned long start_timer = micros();                // start the timer
  bool report = radio.write(&payload, sizeof(float));  // transmit & save the report
  unsigned long end_timer = micros();                  // end the timer

  if (report) {
    Serial2.print(F("TX OK "));
    Serial2.print(end_timer - start_timer);  // print the timer result
    Serial2.println(F(" us"));
    payload += 0.01;          // increment float payload
  } else {
    Serial2.println(F("BAD TX"));  // payload was not delivered
  }

  delay(10);
}