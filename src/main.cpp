#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>

#define SPI0_MISO   0
#define SPI0_CSN    1
#define SPI0_SCK    2
#define SPI0_MOSI   3
#define RF24_INT    4
#define RF24_CE     5
#define UART1_RX    9
#define UART1_TX    8

#define FRAME_SIZE 10

#pragma pack(push, 1)
struct InputFrame {
  int16_t  lx, ly, rx, ry;
  uint16_t buttons;
};
#pragma pack(pop)
static_assert(sizeof(InputFrame) == FRAME_SIZE, "frame size mismatch");

RF24 radio(RF24_CE, SPI0_CSN);
uint8_t address[5] = { 0xCE, 0x15, 0x10, 0x55, 0xBB };

float payload = 0.0;
unsigned long tx_count = 0;

void setup() {
  Serial2.setRX(UART1_RX);
  Serial2.setTX(UART1_TX);
  Serial2.begin();

  SPI.setMISO(SPI0_MISO);
  SPI.setCS(SPI0_CSN);
  SPI.setSCK(SPI0_SCK);
  SPI.setMOSI(SPI0_MOSI);

  Serial2.print("Begin radio init...");
  while (!Serial2);
  if (!radio.begin()) {
    Serial2.println("radio hardware is not responding!!");
    while (1);
  }
  Serial2.println("OK");

  Serial2.printf("TX address: 0x%X 0x%X 0x%X 0x%X 0x%X\r\n",
    address[0], address[1], address[2], address[3], address[4]);
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes
  radio.stopListening(address);
  Serial2.println("Radio config OK");
  delay(1000);
}

void loop() {
  unsigned long start_timer = micros();                // start the timer
  bool report = radio.write(&payload, sizeof(float));  // transmit & save the report
  unsigned long end_timer = micros();                  // end the timer

  Serial2.printf("[%lu] ", millis());
  if (report) {
    Serial2.printf("[tx%lu] ", ++tx_count);
    Serial2.print("TX OK ");
    Serial2.print(end_timer - start_timer);  // print the timer result
    Serial2.println(" us");
    payload += 0.01;          // increment float payload
  } else {
      Serial2.printf("[tx%lu] ", tx_count);
    Serial2.println("BAD TX");  // payload was not delivered
  }

  delay(10);
}