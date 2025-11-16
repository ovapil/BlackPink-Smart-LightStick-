#include <Arduino.h>
#include "input.h"

#define PIN_DEBOUNCE_TIME 50
#define MAX_DOUBLE_PRESS_TIME 300
#define LONG_PRESS_TIME 1000

static int input_button = 0;

typedef enum {
  STATE_OFF,
  STATE_PRESSED,
  STATE_DOUBLE_PRESSED_WAIT,
  STATE_LONG_PRESSED_WAIT
} ButtonState;

static ButtonState button_state = STATE_OFF;

static boolean gotPressed = false;
static boolean gotDoublePressed = false;
static boolean gotLongPressed = false;

static boolean isPressed = false;

static unsigned long last_press_start_time = 0;
static unsigned long last_press_end_time = 0;
static unsigned long last_debounce_time = 0;
static byte last_pin_state = HIGH;

void input_setup(int pin) {
  input_button = pin;
  pinMode(input_button, INPUT_PULLUP);
}

void input_scan() {
  byte pin_state = digitalRead(input_button);
  unsigned long current_time = millis();

  if (pin_state != last_pin_state) {
    last_debounce_time = current_time;
  }
  last_pin_state = pin_state;
 
  if ((current_time - last_debounce_time) < PIN_DEBOUNCE_TIME) {
    return;
  }
  
  switch (button_state) {    
    case STATE_OFF:
      isPressed = false;
      if (pin_state == LOW) { // Nút vừa được nhấn
        button_state = STATE_PRESSED;
        last_press_start_time = current_time;
        isPressed = true;
      }
      break;

    case STATE_PRESSED:
      if (pin_state == HIGH) { // Nút được thả ra (sau lần nhấn 1)
        button_state = STATE_DOUBLE_PRESSED_WAIT;
        last_press_end_time = current_time;
      } else if ((current_time - last_press_start_time) > LONG_PRESS_TIME) {
        button_state = STATE_LONG_PRESSED_WAIT;
        gotLongPressed = true;
      }
      break;

    case STATE_DOUBLE_PRESSED_WAIT:
    if (pin_state == LOW) { // Nút được nhấn lần 2 
        button_state = STATE_LONG_PRESSED_WAIT;
        isPressed = true;
        gotDoublePressed = true;
    } else if ((current_time - last_press_end_time) > MAX_DOUBLE_PRESS_TIME) {
        button_state = STATE_OFF;
        gotPressed = true;
    }
    break;

    case STATE_LONG_PRESSED_WAIT:
      if (pin_state == HIGH) { // Nút được thả ra (sau khi nhấn giữ)
        button_state = STATE_OFF;
        last_press_end_time = current_time;
      }
      break;
  }
}

boolean input_Pressed() {
  if (gotPressed) {
    gotPressed = false;
    return true;
  }
  return false;
}

boolean input_DoublePressed() {
  if (gotDoublePressed) {
    gotDoublePressed = false;
    return true;
  }
  return false;
}

boolean input_LongPressed() {
  if (gotLongPressed) {
    gotLongPressed = false;
    return true;
  }
  return false;
}