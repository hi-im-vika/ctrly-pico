#pragma once
#include <Arduino.h>
#include <RF24.h>
#include "COBS.hpp"
#include "types.hpp"

namespace Radio {

    void init();
    void transmit(InputFrame input, unsigned long tx_count, unsigned long fail_count);
}