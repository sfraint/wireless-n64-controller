#ifndef BLE_GAMEPAD_MAIN_H
#define BLE_GAMEPAD_MAIN_H

#include "ESP32-BLE-Gamepad/BleGamepad.h"
#include "freertos/queue.h"

// CPU core number for compute loads
#define COMPUTE_CORE         1

// LED state
extern uint32_t led_mode;
#define LED_IO_PIN             5
// LED command events
#define LED_OFF                0
#define LED_ON                 1
#define LED_BLINK_SLOW         2
#define LED_BLINK_MED          3
#define LED_BLINK_FAST         4
#define LED_BLINK_VERY_FAST    5

// Non-volatile Storage keys
#define STORAGE_KEY_X_MIN      "min_x"
#define STORAGE_KEY_X_CENTER   "center_x"
#define STORAGE_KEY_X_MAX      "max_x"
#define STORAGE_KEY_Y_MIN      "min_y"
#define STORAGE_KEY_Y_CENTER   "center_y"
#define STORAGE_KEY_Y_MAX      "max_y"

extern BleGamepad bleGamepad;

extern xQueueHandle gpio_evt_queue; // Button-press event queue
#endif

