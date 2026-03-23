#pragma once
#include <Arduino.h>
#include <RF24.h>
#include "COBS.hpp"
#include "types.hpp"

namespace Radio {

    void init();
    void transmit(InputFrame input);

    unsigned long get_tx_cnt();
    unsigned long get_fail_cnt();
}