#include "UART.hpp"

namespace {

    unsigned long rx_count      = 0;
    bool new_input              = false;
}

namespace UART {

    void init() {
        Serial.begin();
    }

    void read_serial(InputFrame &input) {
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

    void write_serial(TelemetryFrame tf) {
        if (Serial) {
            uint8_t buf[TM_FRAME_SIZE + 2];
            
            size_t encoded_len = COBS::encode(&tf,TM_FRAME_SIZE,buf);
            buf[TM_FRAME_SIZE + 1] = 0x00;
            Serial.write(buf,TM_FRAME_SIZE + 2);
            // Serial.printf("Hello from TXP, total_tx: %d, total_rx: %d, total_fail: %d\r\n", tx_count, rx_count, fail_count);
        }
    }

    bool get_input_status() {
        if (new_input) {
            new_input = false;
            return true;
        } else {
            return false;
        }
    }

    unsigned long get_rx_cnt() {
        return rx_count;
    }
}