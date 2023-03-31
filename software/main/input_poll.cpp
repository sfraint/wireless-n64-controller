#include "ble_gamepad_main.h"
#include "input_poll.h"

#include <stdio.h>
#include <sys/time.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "driver/gpio.h"

//#define DEBUG 1
#include "debug.h"


// To change pins, update #define's in input_poll.h
uint32_t buttonPins[NUM_OF_BUTTONS] = {
    BTN_A_PIN,
    BTN_B_PIN,
    BTN_C_DOWN_PIN,
    BTN_C_LEFT_PIN,
    BTN_C_UP_PIN,
    BTN_C_RIGHT_PIN,
    BTN_START_PIN,
    BTN_Z_PIN,
    BTN_L_PIN,
    BTN_R_PIN,
    BTN_IDK2_PIN
};

// "Soft" buttons
// To change physical button reference, update #define's in input_poll.h
uint32_t softButtonRefs[NUM_OF_SOFT_BUTTONS] = {
    BTN_SOFT_SELECT,
    BTN_SOFT_ALT1,
    BTN_SOFT_ALT2
};

uint32_t previousButtonStates[NUM_OF_BUTTONS];
uint32_t currentButtonStates[NUM_OF_BUTTONS];
uint32_t previousSoftButtonStates[NUM_OF_SOFT_BUTTONS];
uint32_t currentSoftButtonStates[NUM_OF_SOFT_BUTTONS];

int16_t previousXState;
int16_t currentXState;
int16_t previousYState;
int16_t currentYState;
int16_t currentBattState = 100;

// Up, down, left, right
uint32_t previousDpadStates[4];
uint32_t currentDpadStates[4];
uint32_t dpadPins[4] = {25, 27, 26, 33};


// Analog input center and range
uint16_t center_x = ANALOG_CENTER;
uint16_t min_x = ANALOG_MIN;
uint16_t max_x = ANALOG_MAX;
uint16_t center_y = ANALOG_CENTER;
uint16_t min_y = ANALOG_MIN;
uint16_t max_y = ANALOG_MAX;

// Scale x from `in` range to `out` range
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    
}


// Return raw value from analog pin. Uses multi-sampling to reduce noise a bit
uint16_t get_analog_raw(adc1_channel_t pin) {
    uint32_t val = 0;
    // Read several times to reduce noise
    // Val will be between 0-32768 (0-4096*8)
    for(int i = 0; i < 8; i++) {
        val += adc1_get_raw(pin);
    }
    return (uint16_t) (val >> 3);
}


// Convert the provided raw analog value to a joystick value (-32767 to 32767) based on minimum, median, and maximum values.
int16_t analog_to_joystick_value(uint16_t raw, uint16_t min, uint16_t med, uint16_t max) {
    // Shortcut for min/max values
    if (raw >= max) return JOYSTICK_MAX;
    if (raw <= min) return JOYSTICK_MIN;

    // Negative
    if (raw < med) {
        int32_t joystick_val = map(raw, min, med, JOYSTICK_MIN, 0) ;
       // printf("negative joystick_val val before scaling : %d \n", joystick_val);
        if(SIXPIN_ENABLED)
        {
        joystick_val = joystick_val * SIXPIN_ANALOG_OVERSCALE;
        }
        else{
        joystick_val = joystick_val * ANALOG_OVERSCALE;
        }
        //printf("joystick_val val after scaling: %d \n", joystick_val);
        if (joystick_val < JOYSTICK_MIN) return JOYSTICK_MIN;
        
        return (int16_t) joystick_val;
    }

    // Positive
    int32_t joystick_val = map(raw, med, max, 0, JOYSTICK_MAX);
    //printf("positive joystick_val valbefore scaling : %d \n", joystick_val);
    if(SIXPIN_ENABLED)
    {
        joystick_val = joystick_val * SIXPIN_ANALOG_OVERSCALE;
    }
    else{
        joystick_val = joystick_val * ANALOG_OVERSCALE;
    }
    //printf("joystick_val val after scaling: %d \n", joystick_val);
    if (joystick_val > JOYSTICK_MAX) return JOYSTICK_MAX;

    return (int16_t) joystick_val;
}


// Convert up, down, left, right signals into the rotational hat encoding
signed char encode_hat(uint32_t up, uint32_t down, uint32_t left, uint32_t right) {
    if (up && right) return 2;
    if (down && right) return 4;
    if (down && left) return 6;
    if (up && left) return 8;
    if (up) return 1;
    if (right) return 3;
    if (down) return 5;
    if (left) return 7;
    return 0;
}

uint8_t get_battery_level() {
    uint16_t rawBatt = adc1_get_raw(ANALOG_BAT);
    if (rawBatt > BATTERY_LEVEL_FULL) {
        currentBattState = 100;
    } else if (rawBatt < BATTERY_LEVEL_EMPTY) {
        currentBattState = 0;
    } else {
        currentBattState = map(rawBatt, BATTERY_LEVEL_EMPTY, BATTERY_LEVEL_FULL, 0, 100);
    }
    return (uint8_t) currentBattState;
}


// Poll for "regular" button presses and returns a boolean indicating if button states changed
bool poll_buttons() {
    bool changed = false;
    // Regular buttons
    for (uint32_t currentIndex = 0 ; currentIndex < NUM_OF_BUTTONS ; currentIndex++) {
      currentButtonStates[currentIndex]  = gpio_get_level((gpio_num_t) buttonPins[currentIndex]);

      if (currentButtonStates[currentIndex] != previousButtonStates[currentIndex]) {
        changed = true;
        if(currentButtonStates[currentIndex] == 1) {
          // Button numbers are 1-indexed
          bleGamepad.press(currentIndex + 1);
        } else {
          // Button numbers are 1-indexed
          bleGamepad.release(currentIndex + 1);
        }
      }
    }
    return changed;
}


// Poll for dpad button presses and returns a boolean indicating if button states changed
bool poll_dpad() {
    bool changed = false;
    // D-pad
    // up, down, left, right
    for (uint32_t i = 0 ; i < 4; i++) {
      currentDpadStates[i]  = gpio_get_level((gpio_num_t) dpadPins[i]);
      if (currentDpadStates[i] != previousDpadStates[i]) {
        changed = true;
        if(!ENABLE_HAT_DPAD) {
          if(currentDpadStates[i] == 1) {
            debug("press DPAD %d (%d)", i, NUM_OF_BUTTONS + i);
            bleGamepad.press(NUM_OF_BUTTONS + i);
          } else {
            bleGamepad.release(NUM_OF_BUTTONS + i);
          }
        }
      }
    }
    // Do I need to setup both hats?
    bleGamepad.setHat(encode_hat(currentDpadStates[0], currentDpadStates[1], currentDpadStates[2], currentDpadStates[3]));
    bleGamepad.setHat2(encode_hat(currentDpadStates[0], currentDpadStates[1], currentDpadStates[2], currentDpadStates[3]));
    return changed;
}


// Poll for joystick changes and returns a boolean indicating if the states changed
bool poll_joystick() {
    bool changed = false;

    // X
    if(SIXPIN_ENABLED){
    currentXState = analog_to_joystick_value(get_sixpin_count(0), min_x, center_x, max_x);
    }
    else{
    currentXState = analog_to_joystick_value(get_analog_raw(ANALOG_X), min_x, center_x, max_x);
    }
    if ((currentXState > 0 && currentXState < JOYSTICK_DEADZONE) || (currentXState < 0 && currentXState > (-1 * JOYSTICK_DEADZONE))) {
        currentXState = 0;
    }
    if (currentXState != previousXState) {
        debug(" ADC1_X: %d\n", currentXState);
        previousXState = currentXState;
        bleGamepad.setX(currentXState);
        changed = true;
    }

    // Y
    if(SIXPIN_ENABLED){
    currentYState = analog_to_joystick_value(get_sixpin_count(1), min_y, center_y, max_y);;
    }
    else{
    currentYState = analog_to_joystick_value(get_analog_raw(ANALOG_Y), min_y, center_y, max_y);
    }
    if ((currentYState > 0 && currentYState < JOYSTICK_DEADZONE) || (currentYState < 0 && currentYState > (-1 * JOYSTICK_DEADZONE))) {
        currentYState = 0;
    }
    if (currentYState != previousYState) {
        debug(" ADC1_Y: %d\n", currentYState);
        previousYState = currentYState;
        bleGamepad.setY(currentYState);
        changed = true;
    }

    return changed;
}


// Poll joystick and button inputs
void input_poll_loop(void* args)
{
    const TickType_t xDelay = 2 / portTICK_PERIOD_MS;  // 2 ms
    static bool pressed = false;
    static uint32_t held = 0;
    static uint32_t counter = 0;
    timezone tz;
    tz.tz_minuteswest = 0;
    tz.tz_dsttime = 0;
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    settimeofday(&tv, &tz);
    while (true) {
        gettimeofday(&tv, &tz);
        int start = tv.tv_usec;
        counter += 1;
        uint32_t mgmt_level = gpio_get_level((gpio_num_t) MGMT_IO_PIN);
        bool changed = false;
        
        // Battery
        if (ENABLE_BATTERY_CHECK) {
            bleGamepad.setBatteryLevel(get_battery_level());
        }

        // Joystick
        bool joystick_changed = poll_joystick();
        changed |= joystick_changed;
        if (joystick_changed) debug("X: %d    Y: %d\n", currentXState, currentYState);
        gettimeofday(&tv, &tz);
        int timeTakenAnalog = tv.tv_usec - start;

        // D-pad
        changed |= poll_dpad();

        // Regular buttons
        changed |= poll_buttons();
        
        // "Soft" buttons
        // TODO improve `changed` logic to detect soft-button resetting other buttons
        // Assume if all Dpad buttons are pressed, that we're in soft-button mode
        if (currentDpadStates[0] == 1 && currentDpadStates[1] == 1 && currentDpadStates[2] == 1 && currentDpadStates[3] == 1) {
        
            // Reset dpad
            // Do I need to set both hats?
            bleGamepad.setHat(encode_hat(0,0,0,0));
            bleGamepad.setHat2(encode_hat(0,0,0,0));
            currentDpadStates[0] = 0;
            currentDpadStates[1] = 0;
            currentDpadStates[2] = 0;
            currentDpadStates[3] = 0;

            for (uint32_t i = 0; i < NUM_OF_SOFT_BUTTONS; i++) {
                uint32_t ref = softButtonRefs[i];
                if (currentButtonStates[ref] == 1) {
                    debug(" soft button %d pressed\n", i);
                    currentSoftButtonStates[i] = 1;
                    bleGamepad.press(NUM_OF_BUTTONS + i + 1);
                    
                    // Don't allow both the phsycal and soft-button to be pressed
                    currentButtonStates[ref] = 0;
                    bleGamepad.release(ref + 1);
                } else {
                    currentSoftButtonStates[i] = 0;
                    bleGamepad.release(NUM_OF_BUTTONS + i + 1);
                }
                if (currentSoftButtonStates[i] != previousSoftButtonStates[i]) changed = true;
            }
        } else {
            for (uint32_t i = 0; i < NUM_OF_SOFT_BUTTONS; i++) {
                currentSoftButtonStates[i] = 0;
                bleGamepad.release(NUM_OF_BUTTONS + i + 1);
                if (currentSoftButtonStates[i] != previousSoftButtonStates[i]) changed = true;
            }
        }


        gettimeofday(&tv, &tz);
        int timeTakenPrePrint = tv.tv_usec - start;
        if (changed) {
            //printf("Button states changed:\n");
            debug("        123456        789AB\n");
            debug(" Group1 ");
            for (uint32_t i = 0 ; i < NUM_OF_BUTTONS_RIGHT ; i++) {
                previousButtonStates[i] = currentButtonStates[i];
                debug("%d", currentButtonStates[i]);
            }
            debug(" Group2 ");
            for (uint32_t i = NUM_OF_BUTTONS_RIGHT ; i < NUM_OF_BUTTONS ; i++) {
                previousButtonStates[i] = currentButtonStates[i];
                debug("%d", currentButtonStates[i]);
            }
            debug(" Dpad: ");
            for (uint32_t i = 0 ; i < 4 ; i++) {
                previousDpadStates[i] = currentDpadStates[i];
                debug("%d", currentDpadStates[i]);
            }
            debug(" (hat encoded: %d)", encode_hat(currentDpadStates[0], currentDpadStates[1], currentDpadStates[2], currentDpadStates[3]));
            debug(" soft buttons: ");
            for (uint32_t i = 0 ; i < NUM_OF_SOFT_BUTTONS ; i++) {
                previousSoftButtonStates[i] = currentSoftButtonStates[i];
                debug("%d", currentSoftButtonStates[i]);
            }
            debug("\n");
            
            if (bleGamepad.isConnected()) {
                debug("Sending report ");
                bleGamepad.sendReport();
                debug("Sent report\n");
            } else {
                debug("No report sent, not connected\n");
            }
            gettimeofday(&tv, &tz);
            int timeTakenTotal = tv.tv_usec - start;
            debug("ANALOG TIME:   %d\n", timeTakenAnalog >> 10);
            debug("PREPRINT TIME: %d\n", timeTakenPrePrint >> 10);
            debug("TOTAL TIME:    %d\n", timeTakenTotal >> 10);
        }


        // MGMT button handling
        // Button UP
        if (mgmt_level == 0) {
            // Button released
            if (pressed && held >= BUTTON_PRESS_THRESH) {
                debug("Button released!\n");
                uint32_t status = BUTTON_UNPRESS;
                xQueueSend(gpio_evt_queue, &status, NULL);
                led_mode = LED_OFF;
            }
            held = 0;
            pressed = false;
            vTaskDelay(xDelay);
            gettimeofday(&tv, &tz);
            int timeTakenEnd = tv.tv_usec - start;
            if (changed) debug("END TIME:     %d\n", timeTakenEnd >> 10);
            continue;
        }
        // Button DOWN
        held += 1;
        uint32_t button_press_event;
        pressed = true;
        if (held == BUTTON_VERY_LONG_PRESS_THRESH) {
            button_press_event = BUTTON_VERY_LONG_PRESS;
            xQueueSend(gpio_evt_queue, &button_press_event, NULL);
            led_mode = LED_BLINK_SLOW;
            debug("Button very long pressed!\n");
        } else if (held == BUTTON_LONG_PRESS_THRESH) {
            button_press_event = BUTTON_LONG_PRESS;
            xQueueSend(gpio_evt_queue, &button_press_event, NULL);
            led_mode = LED_BLINK_MED;
            debug("Button long pressed!\n");
        } else if (held == BUTTON_PRESS_THRESH) {
            button_press_event = BUTTON_PRESS;
            xQueueSend(gpio_evt_queue, &button_press_event, NULL);
            led_mode = LED_BLINK_FAST;
            debug("Button pressed!\n");
        }
        vTaskDelay(xDelay);
    }
}


