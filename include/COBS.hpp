#include <Arduino.h>

namespace COBS {
    size_t encode(const void *data, size_t length, uint8_t *buffer);
    size_t decode(const uint8_t *buffer, size_t length, void *data);
}