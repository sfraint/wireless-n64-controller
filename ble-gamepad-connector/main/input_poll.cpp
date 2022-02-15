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

uint32_t buttonPins[NUM_OF_BUTTONS] = {
    36, 35, 32, 14, 12, 13,  // Right side pins
    23, 22, 21, 19, 18, 17, 16  // Left side pins
};
uint32_t previousButtonStates[NUM_OF_BUTTONS];
uint32_t currentButtonStates[NUM_OF_BUTTONS];
int16_t previousXState;
int16_t currentXState;
int16_t previousYState;
int16_t currentYState;

// Up, down, left, right
uint32_t previousDpadStates[4];
uint32_t currentDpadStates[4];
uint32_t dpadPins[4] = {25, 27, 26, 33};
uint32_t physicalButtons[NUM_OF_BUTTONS] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };


int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// Reads value from analog pin and returns scaled, multi-sampled value ~between -32767 and 32768
int16_t get_analog(adc1_channel_t pin, uint32_t offset) {
  uint32_t val = 0;
  for(int i = 0; i < 8; i++) {
    val += adc1_get_raw(pin) + offset;
  }
  // Overscale the value here since the potentiometers we use don't cover the full range
  // TODO fix hackery to avoid overflow during map
  // TODO make asymetric scaling cleaner
  if (val <= 16384) {
    int32_t val_scaled = (map(val, 0, 16384, -7000, 0) << 3);
    if (val_scaled < -32767) return -32767;
    return val_scaled;
  }
  int32_t val_scaled = (map(val, 16385, 32768, 0, 5000) << 3);
  if (val_scaled > 32767) return 32767;
  return (int16_t) val_scaled;
  // Each iteration adds up to 4095, for a total of 32760
  //return (int16_t) (((int32_t)val << 1) - 32767);
}


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


void input_poll_loop(void* args)
{
  const TickType_t xDelay = 10 / portTICK_PERIOD_MS;  // 10ms
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

    // Joystick
    currentXState = get_analog(ANALOG_X, ANALOG_OFFSET_X);
    if ((currentXState - ANALOG_DRIFT > previousXState) || (currentXState + ANALOG_DRIFT < previousXState)) {
      printf(" ADC1_X: %d\n", currentXState);
      previousXState = currentXState;
      bleGamepad.setX(currentXState);
      changed = true;
    }
    currentYState = get_analog(ANALOG_Y, ANALOG_OFFSET_Y);
    if ((currentYState - ANALOG_DRIFT > previousYState) || (currentYState + ANALOG_DRIFT < previousYState)) {
      printf(" ADC1_Y: %d\n", currentYState);
      previousYState = currentYState;
      bleGamepad.setY(currentYState);
      changed = true;
    }
    gettimeofday(&tv, &tz);
    int timeTakenAnalog = tv.tv_usec - start;

    // D-pad
    // up, down, left, right
    for (uint32_t i = 0; i < 4; i++) {
      currentDpadStates[i]  = gpio_get_level((gpio_num_t) dpadPins[i]);
      if (currentDpadStates[i] != previousDpadStates[i]) {
        changed = true;
      }
    }
    // TODO fix this
    // NOTE: unsigned version sortof worked w/ 0x80 - 0x7F?
    //signed char hatX = 0x0;
    //if (currentDpadStates[2]) hatX += 0x7F;
    //if (currentDpadStates[3]) hatX -= 0x7F;
    //signed char hatY = 0x0;
    //if (currentDpadStates[0]) hatY += 0x7F;
    //if (currentDpadStates[1]) hatY -= 0x7F;
    //bleGamepad.setHat1(hatX);
    //bleGamepad.setHat2(hatY);
    for (uint32_t i = 0 ; i < 4; i++) {
      currentDpadStates[i]  = gpio_get_level((gpio_num_t) dpadPins[i]);
      if (currentDpadStates[i] != previousDpadStates[i]) {
        changed = true;
        if(!ENABLE_HAT_DPAD) {
          if(currentDpadStates[i] == 1) {
            printf("press DPAD %d (%d)", i, NUM_OF_BUTTONS + i);
            bleGamepad.press(NUM_OF_BUTTONS + i);
          } else {
            bleGamepad.release(NUM_OF_BUTTONS + i);
          }
        }
      } 
    }
    // This apparently doesn't work
    bleGamepad.setHat(encode_hat(currentDpadStates[0], currentDpadStates[1], currentDpadStates[2], currentDpadStates[3]));
    
    // TODO try setting second one, different encoding, etc.
    bleGamepad.setHat2(encode_hat(currentDpadStates[0], currentDpadStates[1], currentDpadStates[2], currentDpadStates[3]));

    // Regular buttons
    for (uint32_t currentIndex = 0 ; currentIndex < NUM_OF_BUTTONS ; currentIndex++) {
      currentButtonStates[currentIndex]  = gpio_get_level((gpio_num_t) buttonPins[currentIndex]);

      if (currentButtonStates[currentIndex] != previousButtonStates[currentIndex]) {
        changed = true;
        if(currentButtonStates[currentIndex] == 1) {
          bleGamepad.press(physicalButtons[currentIndex]);
        } else {
          bleGamepad.release(physicalButtons[currentIndex]);
        }
      } 
    }
    gettimeofday(&tv, &tz);
    int timeTakenPrePrint = tv.tv_usec - start;
    
    if (changed) {
      printf("Button states changed:\n");
      printf("        SZLBA?      RtDLUR\n");
      printf(" RIGHT: ");
      for (uint32_t i = 0 ; i < NUM_OF_BUTTONS_RIGHT ; i++) {
        previousButtonStates[i] = currentButtonStates[i];
        printf("%d", currentButtonStates[i]);
      }
      printf(" LEFT: ");
      for (uint32_t i = NUM_OF_BUTTONS_RIGHT ; i < NUM_OF_BUTTONS ; i++) {
        previousButtonStates[i] = currentButtonStates[i];
        printf("%d", currentButtonStates[i]);
      }
      printf(" Dpad: ");
      for (uint32_t i = 0 ; i < 4 ; i++) {
        previousDpadStates[i] = currentDpadStates[i];
        printf("%d", currentDpadStates[i]);
      }
      printf(" (hat encoded: %d)", encode_hat(currentDpadStates[0], currentDpadStates[1], currentDpadStates[2], currentDpadStates[3]));
      printf("\n");
      
      if (bleGamepad.isConnected()) {
        printf("Sending report ");
        bleGamepad.sendReport();
        printf("Sent report\n");
      } else {
        printf("No report sent, not connected\n");
      }
      gettimeofday(&tv, &tz);
      int timeTakenTotal = tv.tv_usec - start;
      printf("ANALOG TIME:   %d", timeTakenAnalog >> 10);
      printf("PREPRINT TIME: %d", timeTakenPrePrint >> 10);
      printf("TOTAL TIME:    %d", timeTakenTotal >> 10);
    }

    // MGMT button handling
    // Button UP
    if (mgmt_level == 0) {
        // Button released
        if (pressed && held >= BUTTON_PRESS_THRESH) {
            printf("Button released!\n");
            uint32_t status = BUTTON_UNPRESS;
            xQueueSend(gpio_evt_queue, &status, NULL);
            led_mode = LED_OFF;
        }
        held = 0;
        pressed = false;
        vTaskDelay(xDelay);
        gettimeofday(&tv, &tz);
        int timeTakenEnd = tv.tv_usec - start;
        if (changed) printf("END TIME:     %d\n", timeTakenEnd >> 10);
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
        printf("Button very long pressed!\n");
    } else if (held == BUTTON_LONG_PRESS_THRESH) {
        button_press_event = BUTTON_LONG_PRESS;
        xQueueSend(gpio_evt_queue, &button_press_event, NULL);
        led_mode = LED_BLINK_MED;
        printf("Button long pressed!\n");
    } else if (held == BUTTON_PRESS_THRESH) {
        button_press_event = BUTTON_PRESS;
        xQueueSend(gpio_evt_queue, &button_press_event, NULL);
        led_mode = LED_BLINK_FAST;
        printf("Button pressed!\n");
    }
    vTaskDelay(xDelay);
  }
}


