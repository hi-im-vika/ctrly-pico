#pragma once
#include <Arduino.h>

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