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


// Button state for gamepad buttons
#define NUM_OF_BUTTONS_RIGHT 6
#define NUM_OF_BUTTONS       13
#define ANALOG_X             ADC1_CHANNEL_3  
#define ANALOG_Y             ADC1_CHANNEL_6
// TODO add calibration function
// Amount to add to analog readings (pre scaling)
#define ANALOG_OFFSET_X        125
#define ANALOG_OFFSET_Y        165
// Amount of analog change to allow before calling the value changed (post scaling)
#define ANALOG_DRIFT         200

#define BTN_START      0
#define BTN_Z          1
#define BTN_L          2
#define BTN_B          3
#define BTN_A          4
#define BTN_IDK        5
#define BTN_R          6
#define BTN_C_DOWN     7
#define BTN_C_LEFT     8
#define BTN_C_UP       9
#define BTN_C_RIGHT    10

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
extern int16_t previousXState;
extern int16_t currentXState;
extern int16_t previousYState;
extern int16_t currentYState;


// Up, down, left, right
extern uint32_t previousDpadStates[4];
extern uint32_t currentDpadStates[4];
extern uint32_t dpadPins[4];
extern uint32_t physicalButtons[NUM_OF_BUTTONS];


void input_poll_loop(void* args);

#endif

