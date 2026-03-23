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
InputFrame input = {};

RF24 radio(RF24_CE, SPI0_CSN);
uint8_t address[5] = { 0xCE, 0x15, 0x10, 0x55, 0xBB };

unsigned long last_report = millis();

/*---Count Variables---*/ 
unsigned long tx_count = 0;
unsigned long rx_count = 0;

void read_serial() {
  uint8_t pos = 0;
  uint8_t buf[FRAME_SIZE];

  while (Serial.available()) {
    buf[pos++] = Serial.read();
    if (pos == FRAME_SIZE) {
      rx_count++;
      pos = 0;
      memcpy(&input, buf, FRAME_SIZE);
    }
  }
}

void write_serial() {
  if (Serial) {
    Serial.printf("Hello from TXP, total_tx: %d, total_rx: %d\r\n", tx_count, rx_count);
  }
  delay(100);
}

void setup() {
  Serial.begin();

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
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(FRAME_SIZE);  // float datatype occupies 4 bytes
  radio.setAutoAck(false);
  radio.setRetries(0,0);
  radio.setDataRate(RF24_2MBPS);
  radio.stopListening(address);
  Serial2.println("Radio config OK");
  delay(1000);
}

void loop() {
  read_serial();
  bool report = radio.writeFast(&input, FRAME_SIZE);  // transmit & save the report
  if (!radio.txStandBy(1000)) {
    radio.flush_tx();
    Serial2.println("TX FAIL");
  } else {
    tx_count++;
  }
}

void loop1() {
  write_serial();
}