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
// Assuming full battery is 3.95V and empty is 3.3V
// Also, assuming battery voltage is cut in half before passing into ESP32
#define BATTERY_LEVEL_EMPTY  2150
#define BATTERY_LEVEL_FULL   2600

// Button state for gamepad buttons
#define NUM_OF_BUTTONS_RIGHT 5
#define NUM_OF_BUTTONS       11
// "Soft" buttons are not physical buttons, e.g. could be triggered by button combos, simulating things like SELECT
#define NUM_OF_SOFT_BUTTONS  3

// 4 PIN Analog Stick config
// GPIO 39
#define ANALOG_X             ADC1_CHANNEL_3
// GPIO 34
#define ANALOG_Y             ADC1_CHANNEL_6

// For 6-pin joysticks: OEM analog stick setup - Not actually analog, uses optical sensors monitoring leading edge with inturrupts
//Enable SIXPIN Stick - If set to 0 then four pin analog enabled
#define SIXPIN_ENABLED  0
#define ESP_INTR_FLAG_DEFAULT 0
// GPIO 39
#define SIXPIN_ANALOG_X     39
// GPIO 35
#define SIXPIN_ANALOG_XQ    35
// GPIO 34
#define SIXPIN_ANALOG_Y     34
// GPIO 34
#define SIXPIN_ANALOG_YQ    13
#define GPIO_INPUT_PIN_SEL  ((1ULL<<SIXPIN_ANALOG_X) | (1ULL<<SIXPIN_ANALOG_Y)| (1ULL<<SIXPIN_ANALOG_XQ)| (1ULL<<SIXPIN_ANALOG_YQ))
// Approx. max ADC readings for SIXPIN joystick potentiometer
#define SIXPIN_ANALOG_MAX   130 
#define SIXPIN_ANALOG_CENTER     (SIXPIN_ANALOG_MAX/2)
// Multiplier to apply to scaled analog readings
// Should be higher than 1 so the max value can be hit consistently
#define SIXPIN_ANALOG_OVERSCALE  1.1

// TODO add calibration function
// Amount to add to analog readings (pre scaling)
#define ANALOG_OFFSET_X     125
#define ANALOG_OFFSET_Y     165
// Amount of analog change to allow before calling the value changed (post scaling)
#define ANALOG_DRIFT        200
// GPIO 35
#define ANALOG_BAT           ADC1_CHANNEL_7
// Define which pins are used for each button
// Right side pins
#define BTN_START_PIN      36
#define BTN_Z_PIN          16
#define BTN_L_PIN          4
#define BTN_B_PIN          14
#define BTN_A_PIN          12
// Left side pins
#define BTN_R_PIN          23
#define BTN_C_DOWN_PIN     22
#define BTN_C_LEFT_PIN     17
#define BTN_C_UP_PIN       19
#define BTN_C_RIGHT_PIN    18
#define BTN_IDK2_PIN       21
// "Soft" buttons logical button number reference
// Number reference points to position in the `buttonPins` array in input_poll.cpp
#define BTN_SOFT_SELECT    6
#define BTN_SOFT_ALT1      0
#define BTN_SOFT_ALT2      1

// Button state for management button
// TODO change this back to 2?
#define MGMT_IO_PIN            4
#define ANALOG_PWR_PIN         21
// Events
#define NONE                   0
#define BUTTON_PRESS           1
#define BUTTON_LONG_PRESS      2
#define BUTTON_VERY_LONG_PRESS 3
#define BUTTON_UNPRESS         10
// Timings TODO base on constant delay time
#define BUTTON_PRESS_THRESH            8
#define BUTTON_LONG_PRESS_THRESH       484
#define BUTTON_VERY_LONG_PRESS_THRESH  1204

// Seconds of idle time until the controller powers down
#define IDLE_TIMEOUT_S        180

// Approx. min, centered, and max ADC readings for joystick potentiometers
#define ANALOG_MIN        0
#define ANALOG_MAX        3900 
#define ANALOG_CENTER     (ANALOG_MAX/2)
// Multiplier to apply to scaled analog readings
// Should be higher than 1 so the max value can be hit consistently
#define ANALOG_OVERSCALE  3/2
// Values used by BleGamepad joystick
#define JOYSTICK_MIN      -32767
#define JOYSTICK_MAX      32767
// 1600 is ~5% of 32767, before analog overscaling
#define JOYSTICK_DEADZONE 1200

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

// Analog input center, range and count for 6-pin joysticks:
extern int16_t center_x;
extern int16_t min_x;
extern int16_t max_x;
extern int countx;
extern int16_t center_y;
extern int16_t min_y;
extern int16_t max_y;
extern int county;

uint16_t get_analog_raw(adc1_channel_t pin);
bool poll_buttons();
bool poll_dpad();
void input_poll_loop(void* args);

#endif

