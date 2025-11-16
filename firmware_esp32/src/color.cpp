#include "color.h"

static Adafruit_NeoPixel* circleL;
static LightMode current_mode;
static unsigned long last_anim_time = 0; 
static int anim_index = 0;
static int faded_color_index = 0;
static int g_current_color_mode = MODE_COLOR_OFF;

static const uint32_t FLASHY_COLORS[] = {
  Adafruit_NeoPixel::Color(255, 0, 0),     // Đỏ
  Adafruit_NeoPixel::Color(255, 127, 0),   // Cam
  Adafruit_NeoPixel::Color(255, 255, 0),   // Vàng
  Adafruit_NeoPixel::Color(0, 255, 0),     // Lục
  Adafruit_NeoPixel::Color(0, 0, 255),     // Lam
  Adafruit_NeoPixel::Color(75, 0, 130),    // Chàm
  Adafruit_NeoPixel::Color(148, 0, 211)    // Tím
};
static const int NUM_FLASHY_COLORS = 7;

static const uint32_t FADED_COLORS[] = {
  Adafruit_NeoPixel::Color(255, 20, 147),  // Hồng neon
  Adafruit_NeoPixel::Color(0, 255, 255),   // Cyan
  Adafruit_NeoPixel::Color(255, 255, 0),   // Vàng tươi
  Adafruit_NeoPixel::Color(0, 255, 127),   // Xanh lục nhạt
  Adafruit_NeoPixel::Color(255, 0, 20),    // Đỏ
  Adafruit_NeoPixel::Color(255, 0, 255),   // Magenta
  Adafruit_NeoPixel::Color(255, 69, 0)     // Đỏ cam
};
static const int NUM_FADED_COLORS = 7;

static void set_all_pixels(uint32_t color) {
  for (int i = 0; i < circleL->numPixels(); i++) {
    circleL->setPixelColor(i, color);
  }
}

static void run_flashy_animation() {
  unsigned long current_time = millis();
  if (current_time - last_anim_time > 1000) {
    last_anim_time = current_time;
    uint32_t color = FLASHY_COLORS[anim_index % NUM_FLASHY_COLORS];
    set_all_pixels(color);
    anim_index++;
    circleL->show();
  }
}

static void run_faded_animation() {
  unsigned long current_time = millis();
  if (current_time - last_anim_time > 110) { 
    last_anim_time = current_time;

    if (anim_index < 7) {
      uint32_t color = FADED_COLORS[faded_color_index];
      circleL->setPixelColor(anim_index, color);
      circleL->show();
    } 
    else if (anim_index < 14) {
      int led_to_turn_off = anim_index - 7;
      circleL->setPixelColor(led_to_turn_off, circleL->Color(0, 0, 0));
      circleL->show();
    }

    anim_index++;

    if (anim_index >= 14) {
      anim_index = 0;
      faded_color_index = (faded_color_index + 1) % NUM_FADED_COLORS;
    }
  }
}

static void run_setup() {
  unsigned long current_time = millis();
  if (current_time - last_anim_time > 400) {
    last_anim_time = current_time;
    if (anim_index == 0) {
      set_all_pixels(circleL->Color(235, 100, 6)); // Cam
      anim_index = 1;
    } else {
      set_all_pixels(circleL->Color(0, 0, 0));
      anim_index = 0;
    }
    circleL->show();
  }
}

void color_setup(Adafruit_NeoPixel* circleL_ptr) {
  circleL = circleL_ptr;
  current_mode = MODE_COLOR_OFF;
}

void color_loop() {
  if (current_mode == MODE_FLASHY) {
    run_flashy_animation();
  } 
  else if (current_mode == MODE_SIGN) {
    run_setup();
  }
  else if (current_mode == MODE_WIPE) {
    run_faded_animation();
  }
}

void color_set_mode(LightMode new_mode) {
  g_current_color_mode = new_mode;
  current_mode = new_mode;
  last_anim_time = 0; 
  anim_index = 0;

  switch (current_mode) {
    case MODE_COLOR_OFF:
      set_all_pixels(circleL->Color(0, 0, 0));
      break;
    case MODE_STATIC_WHITE:
      set_all_pixels(circleL->Color(255, 255, 255));
      break;
    case MODE_STATIC_RED:
      set_all_pixels(circleL->Color(255, 0, 0));
      break;
    case MODE_STATIC_BLUE:
      set_all_pixels(circleL->Color(0, 0, 255));
      break;
    case MODE_STATIC_YELLOW:
      set_all_pixels(circleL->Color(255, 255, 0));
      break;
    default:
      break;
  }

  if (current_mode != MODE_FLASHY && current_mode != MODE_SIGN && current_mode != MODE_WIPE)
      circleL->show();
}

void color_next_mode() {
  LightMode next_mode;

  switch (current_mode) {
    case MODE_STATIC_WHITE:   
      next_mode = MODE_STATIC_RED;    
      break;
    case MODE_STATIC_RED:     
      next_mode = MODE_STATIC_BLUE;   
      break;
    case MODE_STATIC_BLUE:    
      next_mode = MODE_STATIC_YELLOW; 
      break;
    case MODE_STATIC_YELLOW:  
      next_mode = MODE_FLASHY;   
      break;
    case MODE_FLASHY:         
      next_mode = MODE_WIPE; 
      break;
    case MODE_WIPE:         
      next_mode = MODE_STATIC_WHITE; 
      break;
    default:
      next_mode = MODE_STATIC_WHITE;
      break;
  }
  
  color_set_mode(next_mode);
}

int color_get_mode() {
  return g_current_color_mode;
}

void color_set_sync_color(uint32_t color) {
  set_all_pixels(color);
  circleL->show();
}