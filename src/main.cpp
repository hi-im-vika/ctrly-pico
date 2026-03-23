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

#define TM_FRAME_SIZE 12
#pragma pack(push, 1)
struct TelemetryFrame {
  unsigned long tx_count = 0;
  unsigned long rx_count = 0;
  unsigned long fail_count = 0;
};
#pragma pack(pop)
static_assert(sizeof(TelemetryFrame) == TM_FRAME_SIZE, "frame size mismatch");

InputFrame input = {};
TelemetryFrame tm = {};

RF24 radio(RF24_CE, SPI0_CSN);
uint8_t address[5] = { 0xCE, 0x15, 0x10, 0x55, 0xBB };
bool new_input = false;

unsigned long last_report = millis();

/*---Count Variables---*/ 
unsigned long tx_count = 0;
unsigned long rx_count = 0;
unsigned long fail_count = 0;

/** COBS encode data to buffer
	@param data Pointer to input data to encode
	@param length Number of bytes to encode
	@param buffer Pointer to encoded output buffer
	@return Encoded buffer length in bytes
	@note Does not output delimiter byte
*/
size_t cobs_encode(const void *data, size_t length, uint8_t *buffer) {
	assert(data && buffer);

	uint8_t *encode = buffer; // Encoded byte pointer
	uint8_t *codep = encode++; // Output code pointer
	uint8_t code = 1; // Code value

	for (const uint8_t *byte = (const uint8_t *)data; length--; ++byte) {
		if (*byte) // Byte not zero, write it
			*encode++ = *byte, ++code;

		if (!*byte || code == 0xff) { // Input is zero or block completed, restart
      *codep = code, code = 1, codep = encode;
			if (!*byte || length)
				++encode;
		}
	}
	*codep = code; // Write final code value

	return (size_t)(encode - buffer);
}

/** COBS decode data from buffer
	@param buffer Pointer to encoded input bytes
	@param length Number of bytes to decode
	@param data Pointer to decoded output data
	@return Number of bytes successfully decoded
	@note Stops decoding if delimiter byte is found
*/
size_t cobs_decode(const uint8_t *buffer, size_t length, void *data) {
	assert(buffer && data);

	const uint8_t *byte = buffer; // Encoded input byte pointer
	uint8_t *decode = (uint8_t *)data; // Decoded output byte pointer

	for (uint8_t code = 0xff, block = 0; byte < buffer + length; --block) {
		if (block) // Decode block byte
			*decode++ = *byte++;
		else {
			block = *byte++;             // Fetch the next block length
			if (block && (code != 0xff)) // Encoded zero, write it unless it's delimiter.
				*decode++ = 0;
			code = block;
			if (!code) // Delimiter code found
				break;
		}
	}

	return (size_t)(decode - (uint8_t *)data);
}

void read_serial() {
  uint8_t pos = 0;
  uint8_t buf[FRAME_SIZE + 2];  // fs + COBS overhead + delimiter

  while (Serial.available() && !new_input) {
    uint8_t b = Serial.read();
    if (b == 0x00) {
      if (pos == FRAME_SIZE + 1) {
        uint8_t decoded[FRAME_SIZE];
        size_t decoded_len = cobs_decode(buf, pos, decoded);
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
        fail_count++;
      }
    }
  }
}

void write_serial() {
  if (Serial) {
    Serial.printf("Hello from TXP, total_tx: %d, total_rx: %d, total_fail: %d\r\n", tx_count, rx_count, fail_count);
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
  if (new_input) {
    new_input = false;
    bool report = radio.writeFast(&input, FRAME_SIZE);  // transmit & save the report
    if (!radio.txStandBy(1000)) {
      radio.flush_tx();
      Serial2.println("TX FAIL");
      fail_count++;
    } else {
      tx_count++;
    }
  }
}

void loop1() {
  write_serial();
}