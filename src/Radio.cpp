#include "Radio.hpp"

namespace {

    constexpr uint8_t SPI0_MISO     = 0;
    constexpr uint8_t SPI0_CSN      = 1;
    constexpr uint8_t SPI0_SCK      = 2;
    constexpr uint8_t SPI0_MOSI     = 3;
    constexpr uint8_t RF24_INT      = 4;
    constexpr uint8_t RF24_CE       = 5;

    constexpr uint8_t address[5] = { 0xCE, 0x15, 0x10, 0x55, 0xBB };
    unsigned long tx_count          = 0;
    unsigned long fail_count        = 0;
    RF24 radio(RF24_CE, SPI0_CSN);
}

namespace Radio {
    void init() {
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
    }

    void transmit(InputFrame input) {
        bool report = radio.writeFast(&input, FRAME_SIZE);  // transmit & save the report
        if (!radio.txStandBy(1000)) {
            radio.flush_tx();
            Serial2.println("TX FAIL");
            fail_count++;
        } else {
            tx_count++;
        }
    }

    unsigned long get_tx_cnt() {
        return tx_count;
    }

    unsigned long get_fail_cnt() {
        return fail_count;
    }
}