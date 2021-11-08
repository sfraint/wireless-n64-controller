#include "ESP32-BLE-Gamepad/BleGamepad.h"
#include "ble_gamepad_main.h"
#include "input_poll.h"

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

uint32_t led_mode = 0;
BleGamepad bleGamepad;
xQueueHandle gpio_evt_queue = NULL; // Button-press event queue


// Setup specified pin number as pulled-down, input pin
void setup_input_pin(uint32_t pin) {
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = ((1ULL << pin));
  io_conf.pull_down_en = (gpio_pulldown_t) 1;
  io_conf.pull_up_en = (gpio_pullup_t) 0;
  gpio_config(&io_conf);
}

// Setup GPIO
void setup_gpio()
{
  // Standard gamepad buttons
  for (uint32_t i = 0 ; i < NUM_OF_BUTTONS ; i++)
  {
    setup_input_pin(buttonPins[i]);
    previousButtonStates[i] = 0;
    currentButtonStates[i] =  0;
  }
  // D-pad
  for (uint32_t i = 0; i < 4; i++) {
    setup_input_pin(dpadPins[i]);
    previousDpadStates[i] = 0;
    currentDpadStates[i] =  0;
  }
  // Analog in  
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ANALOG_X, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ANALOG_Y, ADC_ATTEN_DB_11);

  // Other IO
  gpio_config_t io_conf;
  // Button feedback LED output
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = ((1ULL << LED_IO_PIN));
  io_conf.pull_down_en = (gpio_pulldown_t) 0;
  io_conf.pull_up_en = (gpio_pullup_t) 0;
  gpio_config(&io_conf);
  // Management button
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = ((1ULL << MGMT_IO_PIN));
  io_conf.pull_down_en = (gpio_pulldown_t) 0;
  io_conf.pull_up_en = (gpio_pullup_t) 0;
  gpio_config(&io_conf);
}

void setup_bt() {
  uint32_t count = NUM_OF_BUTTONS + 4;
  uint32_t hat_count = 0;
  if (ENABLE_HAT_DPAD) {
    count = NUM_OF_BUTTONS;
    hat_count = 2;
  }
  printf("Setting up BT controller w/ %d buttons (hat dpad %d, hat count %d)", count, ENABLE_HAT_DPAD, hat_count);
  bleGamepad.begin(count, hat_count, true, true, false, false, false, false, false, false, false, false, false, false, false);
  bleGamepad.setAutoReport(false);
}

// Control LED output
static void led_handler(void* arg)
{
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;  // 50ms
    uint32_t count = 0;
    for(;;) {
        if (led_mode == LED_OFF) {
            gpio_set_level((gpio_num_t) LED_IO_PIN, 0);
        } else if (led_mode == LED_ON) {
            gpio_set_level((gpio_num_t) LED_IO_PIN, 1);
        } else if (led_mode == LED_BLINK_SLOW) {
            gpio_set_level((gpio_num_t) LED_IO_PIN, (count % 16) == 1);
        } else if (led_mode == LED_BLINK_MED) {
            gpio_set_level((gpio_num_t) LED_IO_PIN, (count % 8) == 1);
        } else if (led_mode == LED_BLINK_FAST) {
            gpio_set_level((gpio_num_t) LED_IO_PIN, (count % 4) == 1);
        } else if (led_mode == LED_BLINK_VERY_FAST) {
            gpio_set_level((gpio_num_t) LED_IO_PIN, (count % 2) == 1);
        }
        vTaskDelay(xDelay);
        count += 1;
        if (count > 31) count = 0;
    }
}


// Handle button press events
static void gpio_button_handler(void* arg)
{
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;  // 500ms
    uint32_t button_press_event;
    uint32_t last_event = NONE;
    //gpio_set_level((gpio_num_t) GPIO_OUTPUT_IO_1, 1);
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &button_press_event, portMAX_DELAY)) {

            // Button UP events
            if (button_press_event == BUTTON_UNPRESS) {
                printf("    (unpress received after %d)\n", last_event);
                //gpio_set_level((gpio_num_t) GPIO_OUTPUT_IO_0, 0);
                
                // Shutdown
                // https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
                // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
                // https://github.com/espressif/esp-idf/blob/master/examples/system/deep_sleep/main/deep_sleep_example_main.c
                if (last_event == BUTTON_VERY_LONG_PRESS) {
                    // This is where you could do a shutdown procedure, e.g.
                    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1); //1 = High, 0 = Low
                    //esp_deep_sleep_start();
                }
                continue;
            }

            // Button DOWN events
            if (button_press_event == BUTTON_PRESS) {
                printf("    (press received)\n");
            } else if (button_press_event == BUTTON_LONG_PRESS) {
                printf("    (long press received)\n");            
            } else if (button_press_event == BUTTON_VERY_LONG_PRESS) {
                printf("    (very long press received)\n");
            }
            last_event = button_press_event;
        }
    }
}

void setup_tasks()
{
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  xTaskCreate(gpio_button_handler, "gpio_button_handler", 2048, NULL, 10, NULL);
  xTaskCreate(led_handler, "led_handler", 2048, NULL, 10, NULL);
  xTaskCreate(input_poll_loop, "in_poll", 2048, NULL, 10, NULL);
}


extern "C" {
   void app_main();
}


void app_main(void)
{
    setup_gpio();
    setup_tasks();
    setup_bt();
    led_mode = LED_ON;
    printf("Initial setup complete!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}

