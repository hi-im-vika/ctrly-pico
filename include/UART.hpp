#include <Arduino.h>
#include "COBS.hpp"
#include "types.hpp"

namespace UART {
    
    void init();

    void read_serial(InputFrame &input);
    void write_serial(TelemetryFrame tf);

    bool get_input_status();
    unsigned long get_rx_cnt();
}