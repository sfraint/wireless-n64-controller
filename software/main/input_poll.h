#ifndef INPUT_POLL_H
#define INPUT_POLL_H

#include <stdio.h>
#include <sys/time.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/adc.h"
#include "driver/gpio.h"

// Whether or not the Dpad is treated as a HAT or just four buttons
#define ENABLE_HAT_DPAD      1

// Whether or not to measure and update battery level
#define ENABLE_BATTERY_CHECK 0
// Empirically measured with test ESP32 setup, assuming full battery is 4.2V and empty is 3.6V
// Also, assuming battery voltage is cut in half before passing into ESP32
#define BATTERY_LEVEL_EMPTY  2020
#define BATTERY_LEVEL_FULL   2355

// Button state for gamepad buttons
#define NUM_OF_BUTTONS_RIGHT 6
#define NUM_OF_BUTTONS       13
// "Soft" buttons are not physical buttons, e.g. could be triggered by button combos, simulating things like SELECT
#define NUM_OF_SOFT_BUTTONS  3
// GPIO 39
#define ANALOG_X             ADC1_CHANNEL_3
// GPIO 34
#define ANALOG_Y             ADC1_CHANNEL_6
// GPIO 35
#define ANALOG_BAT           ADC1_CHANNEL_7
// TODO add calibration function
// Amount to add to analog readings (pre scaling)
#define ANALOG_OFFSET_X        125
#define ANALOG_OFFSET_Y        165
// Amount of analog change to allow before calling the value changed (post scaling)
#define ANALOG_DRIFT         200

// Define which pins are used for each button
// Right side pins
#define BTN_START_PIN      36
#define BTN_Z_PIN          16
#define BTN_L_PIN          32
#define BTN_B_PIN          14
#define BTN_A_PIN          12
#define BTN_IDK1_PIN       13
// Left side pins
#define BTN_R_PIN          23
#define BTN_C_DOWN_PIN     22
#define BTN_C_LEFT_PIN     17
#define BTN_C_UP_PIN       19
#define BTN_C_RIGHT_PIN    18
#define BTN_IDK2_PIN       21
#define BTN_IDK3_PIN       35
// "Soft" buttons logical button number reference
// Number reference points to position in the `buttonPins` array in input_poll.cpp
#define BTN_SOFT_SELECT    6
#define BTN_SOFT_ALT1      0
#define BTN_SOFT_ALT2      1

// Button state for management button
#define MGMT_IO_PIN            2
// Events
#define NONE                   0
#define BUTTON_PRESS           1
#define BUTTON_LONG_PRESS      2
#define BUTTON_VERY_LONG_PRESS 3
#define BUTTON_UNPRESS         10
// Timings TODO base on constant delay time
#define BUTTON_PRESS_THRESH            2
#define BUTTON_LONG_PRESS_THRESH       121
#define BUTTON_VERY_LONG_PRESS_THRESH  301

// Looks like 5 is always pulled high?
// (right) 39, 34 are used for analog in
// (left) 1, 3 are used for UART0; 5 is stuck high     15, 2, 4 are used for other IO in this app
extern uint32_t buttonPins[NUM_OF_BUTTONS];
extern uint32_t previousButtonStates[NUM_OF_BUTTONS];
extern uint32_t currentButtonStates[NUM_OF_BUTTONS];
extern uint32_t previousSoftButtonStates[NUM_OF_SOFT_BUTTONS];
extern uint32_t currentSoftButtonStates[NUM_OF_SOFT_BUTTONS];
extern int16_t previousXState;
extern int16_t currentXState;
extern int16_t previousYState;
extern int16_t currentYState;
extern int16_t currentBattState;


// Up, down, left, right
extern uint32_t previousDpadStates[4];
extern uint32_t currentDpadStates[4];
extern uint32_t dpadPins[4];
extern uint32_t physicalButtons[NUM_OF_BUTTONS];

uint16_t get_analog_raw(adc1_channel_t pin);
bool poll_buttons();
bool poll_dpad();
void input_poll_loop(void* args);

#endif

