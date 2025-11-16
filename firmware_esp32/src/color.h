#pragma once

#include <Adafruit_NeoPixel.h>

typedef enum {
  MODE_COLOR_OFF,
  MODE_STATIC_WHITE,
  MODE_STATIC_RED,
  MODE_STATIC_BLUE,
  MODE_STATIC_YELLOW,
  MODE_FLASHY,
  MODE_WIPE,
  MODE_SIGN,
} LightMode;

void color_setup(Adafruit_NeoPixel* circleL_ptr);
void color_loop();
void color_next_mode();
void color_set_mode(LightMode new_mode);
int color_get_mode();
void color_set_sync_color(uint32_t color);